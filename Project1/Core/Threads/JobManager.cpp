#include "JobManager.h"

#include "JobManagerImpl.h"

namespace lunar {

    JobManager::JobManager(const JobManagerConfig& cfg) 
	    : mImpl(std::make_unique<JobManagerImpl>(cfg))
	{
        // create per-thread entries and start mThreads
        mImpl->mWorkerEntries.reserve(cfg.mNumWorkerThreads);
        mImpl->mIOEntries.reserve(cfg.mNumIOThreads);

        for (int i = 0; i < cfg.mNumWorkerThreads; ++i) 
        {
            auto entry = std::make_unique<JobManagerImpl::ThreadEntry>();
            entry->mType = ThreadType::Worker; 
        	entry->mIndex = i;
            entry->mArena = std::make_unique<PerThreadArena>(cfg.mThreadArenaSizeBytes);
            entry->mLocalQueue = std::make_unique<LocalQueue>();
            mImpl->mWorkerEntries.push_back(std::move(entry));
        }
        
    	for (int i = 0; i < cfg.mNumIOThreads; ++i) 
        {
            auto entry = std::make_unique<JobManagerImpl::ThreadEntry>();
            entry->mType = ThreadType::IO;
            entry->mIndex = i;
            entry->mArena = std::make_unique<PerThreadArena>(cfg.mThreadArenaSizeBytes);
            entry->mLocalQueue = std::make_unique<LocalQueue>();
            mImpl->mIOEntries.push_back(std::move(entry));
        }

        // Start threads
        for (auto& entry : mImpl->mWorkerEntries) {
            JobManagerImpl::ThreadEntry* raw = entry.get();
            mImpl->mWorkerThreads.emplace_back([this, raw] { mImpl->WorkerLoop(raw); });
        }

        for (auto& entry : mImpl->mIOEntries) {
            JobManagerImpl::ThreadEntry* raw = entry.get();
            mImpl->mIOThreads.emplace_back([this, raw] { mImpl->WorkerLoop(raw); });
        }
    }

    JobManager::~JobManager() {
        Shutdown(true);
    }

    void JobManager::Shutdown(bool graceful) 
	{
        if (!mImpl) return;

        mImpl->mShuttingDown.store(true);

        // Signal all threads to stop working
        for (auto& entry : mImpl->mWorkerEntries)
        {
            entry->mIsActive.store(false);
        	std::scoped_lock<std::mutex> lk(entry->mCVMtx);
            entry->mCV.notify_all(); // Wake up if they were sleeping due to CV
        }

        for (auto& entry : mImpl->mIOEntries)
        {
            entry->mIsActive.store(false);
        	std::scoped_lock<std::mutex> lk(entry->mCVMtx);
            entry->mCV.notify_all(); // Wake up if they were sleeping due to CV
        }

        for (auto& t : mImpl->mWorkerThreads) if (t.joinable()) t.join();
        for (auto& t : mImpl->mIOThreads) if (t.joinable()) t.join();
    }

    void JobManager::BeginFrameSync() 
	{
        if (!mImpl) return;

        // Prepare indexer: collect any ready nodes; implementation dependent
        std::scoped_lock<std::mutex> lk(mImpl->mIndexerMtx);

        // Indexer will be run explicitly when user calls run_indexer (or pause/resume sequences)
    }

    void JobManager::PauseForSync() 
	{
        if (!mImpl) return;

        mImpl->mPauseRequested.store(true);

        // notify all mThreads to wake and see pause
        for (auto& e : mImpl->mWorkerEntries) 
        { 
        	std::scoped_lock<std::mutex> lk(e->mCVMtx);
        	e->mCV.notify_all(); 
        }

        for (auto& e : mImpl->mIOEntries) 
        { 
        	std::scoped_lock<std::mutex> lk(e->mCVMtx);
        	e->mCV.notify_all();
        }

        // wait until all report idle
        bool anyWorking = true;
        while (anyWorking)
        {
            anyWorking = false;
            for (auto& e : mImpl->mWorkerEntries)
            {
	            if (e->mIsWorking.load())
	            {
		            anyWorking = true; 
	            	break;
	            }
            }

            for (auto& e : mImpl->mIOEntries)
            {
	            if (e->mIsWorking.load())
	            {
		            anyWorking = true; 
	            	break;
	            }
            }

            if (anyWorking) std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }

    void JobManager::Resume() 
	{
        if (!mImpl) return;

        // If deterministic mode: run indexer to dispatch ready set now
        if (mImpl->mCfg.mDeterministic) mImpl->RunIndexerAndDispatch();

        // Wake the threads back up
        mImpl->mPauseRequested.store(false);

        for (auto& e : mImpl->mWorkerEntries)
        {
            std::scoped_lock<std::mutex> lk(e->mCVMtx);
            e->mCV.notify_all();
        }

        for (auto& e : mImpl->mIOEntries)
        {
            std::scoped_lock<std::mutex> lk(e->mCVMtx);
            e->mCV.notify_all();
        }
    }

    void JobManager::WaitUntilIdle() 
	{
        if (!mImpl) return;

        while (true) 
        {
            bool anyQueued = false;
            {
                std::scoped_lock<std::mutex> g(mImpl->mGraphMtx);
                for (auto& pair : mImpl->mNodes) 
                {
                    std::atomic<JobStatus>& status = pair.second->mStatus;

                    if (status.load(std::memory_order_acquire) == JobStatus::Ready || status.load(std::memory_order_acquire) == JobStatus::Pending)
                    {
	                    anyQueued = true; 
                    	break;
                    }
                }
            }

            if (!anyQueued) 
            {
                bool anyWorking = false;
                for (auto& e : mImpl->mWorkerEntries) 
                {
                    if (e->mIsWorking.load())
                    {
	                    anyWorking = true;
                    	break;
                    }
                }

                for (auto& e : mImpl->mIOEntries)
                {
	                if (e->mIsWorking.load())
	                {
		                anyWorking = true;
	                	break;
	                }
                }

                if (!anyWorking) break;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    }

    GlobalDiagnostics JobManager::SnapshotDiagnostics() const 
	{
        GlobalDiagnostics g;
        if (!mImpl) return g;

        g.mTotalJobsSubmitted = mImpl->mTotalJobsSubmitted.load();
        g.mFailedJobs = mImpl->mFailedJobs.load();

        // Read thread data
        g.mThreads.reserve(mImpl->mWorkerEntries.size() + mImpl->mIOEntries.size());
        for (size_t i = 0; i < mImpl->mWorkerEntries.size(); ++i) 
        {
            ThreadDiagnostics t;
            t.mThreadIndex = static_cast<uint32_t>(i); 
        	t.mType = ThreadType::Worker; 
        	t.mIsActive = mImpl->mWorkerEntries[i]->mIsActive.load(); 
        	t.mIsWorking = mImpl->mWorkerEntries[i]->mIsWorking.load(); 

            // TODO: These need to be synchronized?
        	t.mCurrentJobID = mImpl->mWorkerEntries[i]->mCurrentJobID; 
        	t.mLocalQueueLength = mImpl->mWorkerEntries[i]->mLocalQueue->Size();

            g.mThreads.push_back(t);
        }

        for (size_t i = 0; i < mImpl->mIOEntries.size(); ++i)
        {
            ThreadDiagnostics t;
            t.mThreadIndex = static_cast<uint32_t>(i);
        	t.mType = ThreadType::IO;
        	t.mIsActive = mImpl->mIOEntries[i]->mIsActive.load();
        	t.mIsWorking = mImpl->mIOEntries[i]->mIsWorking.load();

            // TODO: These need to be synchronized?
        	t.mCurrentJobID = mImpl->mIOEntries[i]->mCurrentJobID;
        	t.mLocalQueueLength = mImpl->mIOEntries[i]->mLocalQueue->Size();

            g.mThreads.push_back(t);
        }

        return g;
    }

    void JobManager::SetThreadAffinityMapping(const std::vector<int>& mCoreMap) 
	{
        if (!mImpl) return;
        mImpl->mCfg.mPinThreads = true;
        mImpl->mCfg.mCoreMap = mCoreMap;
    }

    void JobManager::DumpJobGraph(std::string& out_json) const 
	{
        out_json.clear();

        if (!mImpl) return;

        std::scoped_lock<std::mutex> g(mImpl->mGraphMtx);

        // TODO: Use a JSON lib for this
        out_json = "{ \"nodes\": [";

        bool first = true;
        for (auto& p : mImpl->mNodes) 
        {
            if (!first) out_json += ", ";
            first = false;
            out_json += "{\"id\":" + std::to_string(p.first) + ", \"status\": \"" + std::to_string(static_cast<int>(p.second->mStatus.load())) + "\" }";
        }

        out_json += "] }";
    }

    bool JobManager::Cancel(const JobHandleBase& handle) 
	{
        if (!mImpl) return false;

        std::scoped_lock<std::mutex> g(mImpl->mGraphMtx);

        auto it = mImpl->mNodes.find(handle.id());
        if (it == mImpl->mNodes.end()) return false;

        JobNodeBase* base = it->second.get();
        JobStatus s = base->mStatus.load();

        if (s == JobStatus::Pending || s == JobStatus::Ready) 
        {
            base->mStatus.store(JobStatus::Cancelled);
            // TODO: Actually do something to cancel the job work instead of just setting the flag
            return true;
        }

        return false;
    }

    // Templated submit helpers (user must supply matching definitions in implementation)

    template<typename R, typename Fn>
    JobHandle<R> JobManager::Submit(Fn&& task, const JobMeta& meta, const std::vector<JobHandleBase>& deps) {
        if (!mImpl) throw std::runtime_error("JobManager not initialized");

        // Create typed JobNode<R> and wire into graph
        uint64_t id = gJobIDCounter.fetch_add(1, std::memory_order_relaxed);

        std::unique_ptr<JobNode<R>> node = std::make_unique<JobNode<R>>();
        node->id = id; 
    	node->mPriority = meta.mPriority; 
    	node->mTarget = meta.mTarget; 
        node->mHeuristic = meta.mEstimatedCost;

        // TODO: Preferred thread index so we can guarantee certain jobs run on a thread

    	node->mStatus.store(JobStatus::Pending);

        // capture task into node (SSO/arena logic TODO)
        // TODO: Need to be able to pass on data ike function args to the job
        node->mTask = std::function<R()>(std::forward<Fn>(task));

        // register in graph
        {
            std::scoped_lock<std::mutex> g(mImpl->mGraphMtx);
            mImpl->mNodes[id] = std::move(node);

            // append dependency edges
            for (auto& d : deps) 
            {
                auto it = mImpl->mNodes.find(d.id());
                if (it != mImpl->mNodes.end())
                {
                    mImpl->mNodes[id]->mPendingDependencies.fetch_add(1);
                    it->second->mDependents.push_back(mImpl->mNodes[id].get());
                }
            }

            if (mImpl->mNodes[id]->mPendingDependencies.load() == 0) // No dependencies, we're ready to go
            {
                mImpl->mNodes[id]->mStatus.store(JobStatus::Ready);

                if (mImpl->mCfg.mDeterministic) 
                {
                    std::scoped_lock<std::mutex> lk(mImpl->mIndexerMtx);
                    mImpl->mIndexerReadySet.push_back(mImpl->mNodes[id].get());
                }
                else 
                {
                    mImpl->EnqueueReadyNode(mImpl->mNodes[id].get());
                }
            }
        }

        mImpl->mTotalJobsSubmitted.fetch_add(1, std::memory_order_acq_rel);

        // return handle
        JobHandle<R> h(id);
        return h;
    }

    template<typename R, typename Fn, typename... Deps>
    JobHandle<R> JobManager::Submit(Fn&& task, const JobMeta& meta, Deps... deps) 
	{
        std::vector<JobHandleBase> vec;
        (vec.push_back(deps), ...);
        return submit<R, Fn>(std::forward<Fn>(task), meta, vec);
    }

    // JobHandle template method stubs — real implementation hooks into Impl

    template<typename R>
    R JobHandle<R>::Get() const 
	{
        // naive: poll the JobManager nodes map until done — production should block on condition variable
        // Here, we require an external global access to JobManager's Impl; in real code JobHandle would carry a weak_ptr
        throw std::logic_error("JobHandle::get not implemented in skeleton — link into JobManager Impl");
    }

    template<typename R>
    bool JobHandle<R>::TryGet(R& out) const noexcept 
	{
        (void)out;
        return false;
    }

    template<typename R>
    void JobHandle<R>::Wait() const 
	{
        throw std::logic_error("JobHandle::wait not implemented in skeleton — link into JobManager Impl");
    }

    template<typename R>
    bool JobHandle<R>::IsDone() const noexcept
    {
	    return false;
    }

    template<typename R>
    template<typename ContFn>
    void JobHandle<R>::Then(ContFn&&, const JobMeta&)
    {
	    throw std::logic_error("JobHandle::then not implemented in skeleton");
    }
}

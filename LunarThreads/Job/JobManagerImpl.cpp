#include  "JobManagerImpl.h"

#include <algorithm>
#include <new>
#include <cassert>

namespace lunar {

    JobManagerImpl::JobManagerImpl(const JobManagerConfig& c) : mCfg(c) {}

    JobManagerImpl::~JobManagerImpl() {

        // ensure mThreads stopped

    }



    void JobManagerImpl::WorkerLoop(ThreadEntry* entry) {

        // Optionally pin thread

        if (mCfg.mPinThreads) 

        {

            // Determine mapping: find mIndex in combined map

            // TODO: map entry->mIndex to mCoreMap

            // platform::set_current_thread_affinity(coreIndex);

        }



        using clock = std::chrono::steady_clock;

        while (entry->mIsActive.load(std::memory_order_acquire) && !mShuttingDown.load()) {



            // Pause handling

            if (mPauseRequested.load(std::memory_order_acquire)) 

            {

                // Signal idle, release guarantees that any writes that happened before this store() can be read by other threads as long as they acquire mIsWorking

                entry->mIsWorking.store(false, std::memory_order_release);



                // Wait until unpaused or shutdown

                std::unique_lock<std::mutex> lk(entry->mCVMtx);

                entry->mCV.wait_for(lk, std::chrono::milliseconds(10), [&] {

                    return !mPauseRequested.load(std::memory_order_acquire) || mShuttingDown.load();

                });



                continue;

            }



            JobNodeBase* node = nullptr;



            // Get next local task

            node = entry->mLocalQueue->PopFront();



            // No local task, look at the global immediate queue

            if (!node) 

            {

                std::scoped_lock<std::mutex> lk(mImmediateMtx);

                if (!mGlobalImmediate.empty())

                {

	                node = mGlobalImmediate.front(); 

                	mGlobalImmediate.pop_front();

                }

            }



            // No global or local task, try to Work-steal (sketch) - disabled in deterministic frame

            if (!node && mCfg.mCanWorkSteal && !mCfg.mDeterministic) 

            {

                // TODO: attempt to steal from other entries deterministically

            }



            if (node) 

            {

                // Signal we are working, release to guarantee that all previous reads/and writes can be seen by other threads (ie: the new queue state)

                entry->mIsWorking.store(true, std::memory_order_release);

                entry->mCurrentJobID = node->mID;



                // Perform work

                ExecuteNodeOnThread(node, entry);



                // Signal done working, release to guarantee that the other threads can see the job results. For example:

                // If we wanted to access some data the job produced, we need to make sure we entry->mIsWorking.load(std::memory_order_acquire) before accessing the data

                entry->mIsWorking.store(false, std::memory_order_release);

                entry->mCurrentJobID = 0;



                continue;

            }



            // Nothing found: spin for short time then sleep

            auto start = clock::now();

            while (clock::now() - start < mCfg.mSpinWaitTime && !mShuttingDown.load() && !mPauseRequested.load()) 

            {

                // Retry local queue (Fast)

                node = entry->mLocalQueue->PopFront();

                if (node) break;



                // No work, signal OS that it's okay to give up our time-slice to allow other threads to execute

                // NOTE: This thread can still remain active despite signaling, it just gives the scheduler a chance to run other threads

                std::this_thread::yield();

            }



            // Nothing found when spinning, sleep on condition variable or timed wait

            if (!node)

            {

                std::unique_lock<std::mutex> lk(entry->mCVMtx);

                entry->mCV.wait_for(lk, std::chrono::milliseconds(10));

            }

        }

    }



    // Execute node (mType-erased entry point). It must downcast to typed JobNode and run it.

    void JobManagerImpl::ExecuteNodeOnThread(JobNodeBase* base, ThreadEntry* entry) 

	{

        // Increase mRefcount as it's being executed

        base->mRefcount.fetch_add(1, std::memory_order_acq_rel);

        base->mStatus.store(JobStatus::Running, std::memory_order_release);



        // Note: this skeleton only demonstrates void and non-void handling for common cases.

        // Real implementation requires mType-dispatch (mType mIndex) or templated execute logic.



        // Type switch: dynamic_cast to known typed variants (costly) - implement real mType-indexing in production.

        // For skeleton, attempt dynamic_casts for example types (user will extend for other types).



        bool success = true;

        try 

        {

            // TODO: proper typed invocation; here we just capture mException if any

            // Example (pseudocode): auto typed = static_cast<JobNode<T>*>(base); typed->mResult = typed->mTask();



        }

        catch (...) 

        {

            base->mStatus.store(JobStatus::Failed, std::memory_order_release);

            base->mRefcount.fetch_sub(1, std::memory_order_acq_rel);

            PropagateFailure(base);

            return;

        }



        base->mStatus.store(JobStatus::Completed, std::memory_order_release);



        // Notify mContinuations

        {

            std::scoped_lock<std::mutex> lk(base->mContMtx);

            for (auto& c : base->mContinuations) 

            {

                // schedule continuation as job or execute depending on policy

                // For skeleton we execute continuation inline - production should schedule it as a job with JobMeta.

                try { c(); }

                catch (...) { /* swallow or log */ }

            }



            base->mContinuations.clear();

        }



        // Propagate readiness to mDependents

        PropagateCompletion(base);



        // Decrement mRefcount and potentially free

        if (base->mRefcount.fetch_sub(1, std::memory_order_acq_rel) == 1) 

        {

            // free node from mNodes map - caller must ensure mGraphMtx held or other safe mechanism

            std::scoped_lock<std::mutex> g(mGraphMtx);

            mNodes.erase(base->mID);

        }

    }



    void JobManagerImpl::PropagateCompletion(JobNodeBase* base) 

	{

        std::vector<JobNodeBase*> readiedJobs;

        {

            // copy mDependents under graph lock to avoid races during iteration

            std::scoped_lock<std::mutex> g(mGraphMtx);

            for (auto dep : base->mDependents) {

                int prev = dep->mPendingDependencies.fetch_sub(1, std::memory_order_acq_rel);

                if (prev == 1) 

                {

                    // became ready

                    dep->mStatus.store(JobStatus::Ready, std::memory_order_release);

                    readiedJobs.push_back(dep);

                }

            }

        }



        // Enqueue ready mNodes according to mode (deterministic vs mImmediate)

        for (auto r : readiedJobs) 

        {

            if (mCfg.mDeterministic) 

            {

                std::scoped_lock<std::mutex> lk(mIndexerMtx);

                mIndexerReadySet.push_back(r);

            }

            else 

            {  // non-deterministic: push to mTarget local queue or mImmediate

                EnqueueReadyNode(r);

            }

        }

    }



    void JobManagerImpl::PropagateFailure(JobNodeBase* base) 

	{

        // BFS propagate failure to mDependents

        std::vector<JobNodeBase*> q;

        {

            std::scoped_lock<std::mutex> g(mGraphMtx);

            for (auto d : base->mDependents) q.push_back(d);

        }



        while (!q.empty()) 

        {

            JobNodeBase* n = q.back(); q.pop_back();



            JobStatus prev = n->mStatus.load(std::memory_order_acquire);



            if (prev == JobStatus::Pending || prev == JobStatus::Ready) 

            {

                n->mStatus.store(JobStatus::Failed, std::memory_order_release);

                mFailedJobs.fetch_add(1, std::memory_order_acq_rel);



                // gather its mDependents

                std::scoped_lock<std::mutex> g(mGraphMtx);

                for (auto d : n->mDependents) q.push_back(d);

            }

        }

    }



    void JobManagerImpl::EnqueueReadyNode(JobNodeBase* node) 

	{

        // Respect mTarget

        if (node->mTarget == ThreadType::IO)

        {

            // simple round-robin enqueue to IO local queues

            if (!mIOEntries.empty())   // TODO: Does it matter if its not empty?

            {

                mIOEntries[0]->mLocalQueue->Push(node); // simplistic



                // wake thread

                std::scoped_lock<std::mutex> lk(mIOEntries[0]->mCVMtx);

                mIOEntries[0]->mCV.notify_one();



                // TODO: Don't just choose first entry, select based on lowest workload

            }



            return;

        }



        // else enqueue to worker local queue (round-robin for skeleton)

        if (!mWorkerEntries.empty())  // TODO: Does it matter if its not empty?

        {

            mWorkerEntries[0]->mLocalQueue->Push(node);

            std::scoped_lock<std::mutex> lk(mWorkerEntries[0]->mCVMtx);

            mWorkerEntries[0]->mCV.notify_one();



            // TODO: Don't just choose first entry, select based on lowest workload



            return;

        }



        // fallback to mImmediate

        {

            std::scoped_lock<std::mutex> lk(mImmediateMtx);

            mGlobalImmediate.push_back(node);

        }

    }



    // Indexer: build deterministic schedule and load per-thread queues

    void JobManagerImpl::RunIndexerAndDispatch() 

	{

        std::vector<JobNodeBase*> readyCopy;

        {

            std::scoped_lock<std::mutex> lk(mIndexerMtx);



            // Swap to local copy so we avoid locking for longer

            // We also swap the empty vector to mIndexerReadySet, so it is cleared for us

            readyCopy.swap(mIndexerReadySet); 

        }



        if (readyCopy.empty()) return;



        // stable sort by mPriority, then by mID (as proxy for submit order)

        std::stable_sort(readyCopy.begin(), readyCopy.end(), [](JobNodeBase* a, JobNodeBase* b) {

            if (a->mPriority != b->mPriority)

            {

                return static_cast<int>(a->mPriority) > static_cast<int>(b->mPriority);

            }

                

            return a->mID < b->mID;

        });



        // Deterministic assignment: round-robin as example

        // TODO: Assignment based on heuristic from JobMeta

        size_t W = mWorkerEntries.size();

        size_t idx = 0;

        std::vector<std::vector<JobNodeBase*>> threadBatches(W);

        for (JobNodeBase* n : readyCopy)

        {

            threadBatches[idx++ % W].push_back(n);

        }



        // Enqueue to local queues deterministically

        for (size_t i = 0; i < W; ++i) 

        {

            mWorkerEntries[i]->mLocalQueue->EnqueueBatch(threadBatches[i]);



            // wake thread

            std::scoped_lock<std::mutex> lk(mWorkerEntries[i]->mCVMtx);

            mWorkerEntries[i]->mCV.notify_one();

        }

    }



    // Simplified submit (mType erasure path) - real impl is templated and constructs JobNode<R>

    JobNodeBase* JobManagerImpl::CreateNodeErased(JobPriority pri, ThreadType tt) 

	{

        uint64_t mID = gJobIDCounter.fetch_add(1, std::memory_order_relaxed);



        // for skeleton allocate base node - typed mNodes created by templated submit

        auto raw = new JobNodeBase();

        raw->mID = mID; 

    	raw->mPriority = pri;

    	raw->mTarget = tt; 

    	raw->mStatus.store(JobStatus::Pending);



        std::scoped_lock<std::mutex> g(mGraphMtx);

        mNodes[mID].reset(raw); // Reset will swap out the old data with the new and also handle deleting the old

        mTotalJobsSubmitted.fetch_add(1, std::memory_order_acq_rel);


            return raw;
        }
	
    // --- PerThreadArena ---

    PerThreadArena::PerThreadArena(size_t bytes)
        : mCapacity(bytes),
        mBuffer(bytes, 0),
        mOffset(0)
    {
    }

    PerThreadArena::~PerThreadArena() = default;

    void* PerThreadArena::AllocateFromBuffer(size_t sz, size_t align)
    {
        if (sz == 0 || mCapacity == 0) return nullptr;

        unsigned char* base = mBuffer.data();
        size_t offset = mOffset;
        void* ptr = base + offset;
        size_t space = mCapacity - offset;

        void* aligned = std::align(align, sz, ptr, space);
        if (!aligned) return nullptr;

        unsigned char* alignedBytes = static_cast<unsigned char*>(aligned);
        size_t consumed = static_cast<size_t>(alignedBytes - base) + sz;
        if (consumed > mCapacity) return nullptr;

        mOffset = consumed;
        return alignedBytes;
    }

    void* PerThreadArena::Allocate(size_t sz, size_t align)
    {
        if (sz == 0) return nullptr;

        // Fast path: reuse from free list
        {
            std::scoped_lock<std::mutex> lk(mMtx);
            auto it = mFreeLists.find(sz);
            if (it != mFreeLists.end() && !it->second.empty())
            {
                void* ptr = it->second.back();
                it->second.pop_back();
                return ptr;
            }
        }

        // Bump allocate
        if (void* p = AllocateFromBuffer(sz, align))
        {
            return p;
        }

        // Fallback to heap if arena exhausted
        return ::operator new(sz, std::align_val_t(align));
    }

    void PerThreadArena::Deallocate(void* ptr, size_t sz)
    {
        if (!ptr || sz == 0) return;

        // Return to size-class free list
        std::scoped_lock<std::mutex> lk(mMtx);
        mFreeLists[sz].push_back(ptr);
    }

    void PerThreadArena::Reset()
    {
        std::scoped_lock<std::mutex> lk(mMtx);
        mFreeLists.clear();
        mOffset = 0;
    }

}

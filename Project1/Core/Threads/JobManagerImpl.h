#pragma once

#include "JobTypes.h"

#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <deque>
#include <thread>
#include <unordered_map>

// Platform specifics for Windows affinity
#if defined(_WIN32)
#include <windows.h>
#endif

namespace lunar
{
	// Forward declarations for internal types
    struct JobNodeBase;

    // Small variant for mResult storage: SSO buffer (Small Object Optimization)
    struct SSOBuffer 
	{
        static constexpr size_t DefaultSize = 64; // default; overridden by config if needed
        alignas(std::max_align_t) unsigned char data[DefaultSize];
    };

    // Per-thread mArena interface (simple skeleton)
    class PerThreadArena 
	{
    public:
        explicit PerThreadArena(size_t bytes);
        ~PerThreadArena();

        void* Allocate(size_t sz, size_t align = alignof(std::max_align_t));
        void Deallocate(void* ptr, size_t sz);

        // Deterministic reuse policy optional
        void Reset();

    private:
        // TODO: implement bump allocator + free lists
        std::vector<unsigned char> mBuffer;
        size_t mOffset = 0;
        std::mutex mMtx; // for remote frees
    };

    // LocalQueue skeleton for deterministic dispatch lists + non-deterministic path
    class LocalQueue 
	{
    public:
        LocalQueue();
        ~LocalQueue();

        // Deterministic mode: indexer will call this to overwrite the dispatch list for the frame
        void EnqueueBatch(const std::vector<JobNodeBase*>& batch);

        // Non-deterministic push (external submissions)
        void Push(JobNodeBase* node);

        // Pop next job for owner thread (fast path)
        JobNodeBase* PopFront();

        size_t Size() const;

    private:
        // TODO: implement ring buffer with SPSC fast path and MPSC push fallback
        std::deque<JobNodeBase*> mDeque;
        mutable std::mutex mMtx; // protects mDeque for now
    };

    // Job node base for mType-erased management
    struct JobNodeBase 
	{
        // virtual destructor to be overridden by typed JobNode
        virtual ~JobNodeBase() = default;

        uint64_t mID = 0;
        JobPriority mPriority = JobPriority::Normal;
        ThreadType mTarget = ThreadType::Worker;
        uint8_t mHeuristic = 1;
        std::atomic<int> mPendingDependencies{ 0 };
        std::vector<JobNodeBase*> mDependents;
        std::atomic<JobStatus> mStatus{ JobStatus::Pending };
        std::atomic<int> mRefcount{ 1 };
        std::mutex mContMtx;
        std::vector<std::function<void()>> mContinuations; // mType-erased continuation wrapper
    };

    // Typed JobNode storing the callable and typed mResult
    template<typename R>
    struct JobNode : JobNodeBase 
	{
        JobNode() = default;
        ~JobNode() override = default;

        using TaskFn = std::function<R()>;

        // SSO inline storage for mTask/mResult when small
        std::optional<TaskFn> mTask; // stored here; large closures may be allocated in mArena
        // Result storage: for void use special handling
        std::optional<R> mResult; // placement storage or pointer-managed depending on SSO/pool strategy

        // Exception capture
        std::exception_ptr mException;
    };

    // Simple atomic mID generator
    static std::atomic<uint64_t> gJobIDCounter{ 1 };

    // Platform affinity shim for Windows (other platforms no-op for now)
    namespace platform {
        inline bool set_current_thread_affinity(int coreIndex) {
#if defined(_WIN32)
            DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << coreIndex);
            HANDLE th = GetCurrentThread();
            DWORD_PTR prev = SetThreadAffinityMask(th, mask);
            return prev != 0;
#else
            (void)coreIndex;
            return false;
#endif
        }
    } // namespace platform

    struct JobManagerImpl 
	{
        // Per-thread state
        struct ThreadEntry {
            ThreadType mType;
            int mIndex;
            std::unique_ptr<PerThreadArena> mArena;
            std::unique_ptr<LocalQueue> mLocalQueue;
            std::atomic<bool> mIsActive{ true };
            std::atomic<bool> mIsWorking{ false };
            uint64_t mCurrentJobID = 0;

            // synchronization
            std::condition_variable mCV; // used to wake thread from pause/sleep
            std::mutex mCVMtx;
            // TODO: Alignment to prevent false sharing
        };

        JobManagerImpl(const JobManagerConfig& c);

        ~JobManagerImpl();

        JobManagerImpl(const JobManagerImpl&) = delete;
        JobManagerImpl(JobManagerImpl&&) = delete;
        JobManagerImpl operator=(const JobManagerImpl&) = delete;
        JobManagerImpl operator=(JobManagerImpl&&) = delete;

        // Thread entrypoint for worker mThreads
        void WorkerLoop(ThreadEntry* entry);

        // Execute node (mType-erased entry point). It must downcast to typed JobNode and run it.
        void ExecuteNodeOnThread(JobNodeBase* base, ThreadEntry* entry);

        void PropagateCompletion(JobNodeBase* base);
        void PropagateFailure(JobNodeBase* base);

        void EnqueueReadyNode(JobNodeBase* node);

        // Indexer: build deterministic schedule and load per-thread queues
        void RunIndexerAndDispatch();

        // Simplified submit (mType erasure path) — real impl is templated and constructs JobNode<R>
        JobNodeBase* CreateNodeErased(JobPriority pri, ThreadType tt);

        // --- Variables ---

        JobManagerConfig mCfg;

        // Threads
        std::vector<std::thread> mWorkerThreads;
        std::vector<std::thread> mIOThreads;

        std::vector<std::unique_ptr<ThreadEntry>> mWorkerEntries;
        std::vector<std::unique_ptr<ThreadEntry>> mIOEntries;

        // GlobalImmediateQueue (simple mutex-protected queue for skeleton)
        std::deque<JobNodeBase*> mGlobalImmediate;
        std::mutex mImmediateMtx;

        // Dependency graph storage: store mNodes for lifetime
        std::mutex mGraphMtx;
        std::unordered_map<uint64_t, std::unique_ptr<JobNodeBase>> mNodes; // owned mNodes

        // Control flags
        std::atomic<bool> mShuttingDown{ false };
        std::atomic<bool> mPauseRequested{ false };

        // Indexer state
        std::mutex mIndexerMtx;
        std::vector<JobNodeBase*> mIndexerReadySet;

        // Diagnostics counters
        std::atomic<size_t> mTotalJobsSubmitted{ 0 };
        std::atomic<size_t> mFailedJobs{ 0 };
    };
}
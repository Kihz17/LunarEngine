#pragma once

#include "JobTypes.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>

// Forward notes:
// - Public API is type-safe and templated.
// - Internals (JobNode storage, indexer, arenas) are opaque here; only interfaces are exposed.
// - Error handling: Job failures propagate to dependents and are reported via JobResult/exception.

namespace lunar {

    struct JobManagerImpl;

    // Error type used for job execution failure
    struct JobExecutionError : public std::runtime_error { explicit JobExecutionError(const std::string& msg) : std::runtime_error(msg) {} };

    // Opaque base for job handles used to describe dependencies in generic form
    class JobHandleBase 
	{
    public:
        virtual ~JobHandleBase() = default;
        uint64_t id() const noexcept { return mID; }
    protected:
        explicit JobHandleBase(uint64_t id = 0) : mID(id) {}
    private:
        uint64_t mID = 0;
    };

    // Typed job handle (templated)
    template<typename R>
    class JobHandle final : public JobHandleBase 
	{
    public:
        JobHandle() = delete;
        JobHandle(const JobHandle&) = default;
        JobHandle(JobHandle&&) noexcept = default;
        JobHandle& operator=(const JobHandle&) = default;
        JobHandle& operator=(JobHandle&&) noexcept = default;

        // blocks until job's status is Completed or Failed. On Failed, throws JobExecutionError.
        R Get() const;

        // non-blocking; returns true if Completed (writes out) or false otherwise. If Failed, returns false.
        bool TryGet(R& out) const noexcept;

        // blocks until job Completed or Failed; does not return result.
        void Wait() const;

        // Query finished state
        bool IsDone() const noexcept;

        // Registering a continuation via Then will schedule a new job when the original completes.
		// Continuations are scheduled according to contMeta and respect deterministic/pause rules.
        template<typename ContFn>
        void Then(ContFn&& cont, const struct JobMeta& contMeta);

    private:
        // Internal construction by JobManager
        explicit JobHandle(uint64_t id) : JobHandleBase(id) {}
        friend class JobManager;
    };

    // JobMeta describes scheduling hints for a job
    struct JobMeta 
	{
        JobPriority mPriority = JobPriority::Normal;
        ThreadType mTarget = ThreadType::Worker;
        bool mImmediate = false; // attempt ASAP; will be subject to pause/determinism rules
        std::optional<int> mPreferredThreadIndex; // affinity hint
        size_t mEstimatedCost = 1; // heuristic for indexer balancing
    };

    class JobManager 
	{
    public:
        explicit JobManager(const JobManagerConfig& cfg = {});
        ~JobManager();

        JobManager(const JobManager&) = delete;
        JobManager& operator=(const JobManager&) = delete;

        // Start / shutdown
		// start() implicitly called by constructor; if you stop, create a new manager instance.
		// shutdown blocks until threads join. If graceful==false, running jobs may be cancelled.
        void Shutdown(bool graceful = true);

        // Control: deterministic/frame sync
        void BeginFrameSync(); // prepare for deterministic scheduling (collect ready set). It does NOT block workers.
        void PauseForSync(); // blocks until all threads finish current task and are paused. While paused, no jobs run.
        void Resume(); // unpauses workers.

        // Wait until all queues empty and threads idle (useful for quiet points)
        void WaitUntilIdle();

        // Set affinity mapping for threads. Must be called before worker threads are created (i.e., before ctor returns)
		// or will take effect on next restart. Values are Windows logical processor indices.
        void SetThreadAffinityMapping(const std::vector<int>& mCoreMap);


        // Submit() semantics:
		//   * On submit, manager creates a typed JobNode in its allocation system.
		//   * If deps.empty() && system not currently paused/deterministic-window, behave according to meta.immediate and target.
		//   * If mDeterministic is true and a deterministic frame is active, new ready jobs are recorded for next indexer run.

        // Submitting jobs (templated). 'deps' accepts a list of JobHandleBase pointers or typed handles.
		// Returns a typed JobHandle<R> that can be waited on. The JobManager takes ownership of the job
		// payload and handles result storage according to the SSO + arena strategy.
        template<typename R, typename Fn>
        JobHandle<R> Submit(Fn&& task, const JobMeta& meta = {}, const std::vector<JobHandleBase>& deps = {});

        template<typename R, typename Fn, typename... Deps>
        JobHandle<R> Submit(Fn&& task, const JobMeta& meta, Deps... deps);

        // TODO: submit for void-return jobs (template deduction helps), alternative to specialized function.

        // Non-blocking attempt to cancel a job prior to execution. Returns true if cancelled.
        bool Cancel(const JobHandleBase& handle);

        // Diagnostics/Debug
        GlobalDiagnostics SnapshotDiagnostics() const;
        void DumpJobGraph(std::string& out_json) const;

    private:
        std::unique_ptr<JobManagerImpl> mImpl;
    };

} // namespace lunar

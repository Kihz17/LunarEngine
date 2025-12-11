#pragma once

#include <chrono>
#include <vector>

namespace lunar {

    enum class ThreadType : uint8_t
    {
        Worker = 0,
        IO = 1,
        Any = 2
    };

    enum class JobPriority : uint8_t
    {
        Low = 0,
        Normal = 1,
        High = 2
    };

    enum class JobStatus : uint8_t
    {
        Pending,
        Ready,
        Running,
        Completed,
        Failed,
        Cancelled
    };

    struct ThreadDiagnostics
    {
        uint32_t mThreadIndex = 0;
        ThreadType mType = ThreadType::Worker;
        bool mIsActive = false;
        bool mIsWorking = false;
        uint64_t mCurrentJobID = 0;
        size_t mLocalQueueLength = 0;
        // pad/alignment intentionally omitted in header
    };

    struct GlobalDiagnostics
    {
        size_t mTotalJobsSubmitted = 0;
        size_t mActiveJobs = 0;
        size_t mQueuedJobs = 0;
        size_t mFailedJobs = 0;
        std::vector<ThreadDiagnostics> mThreads;
    };

    struct JobManagerConfig
    {
        int mNumWorkerThreads = 4;
        int mNumIOThreads = 1;
        bool mPinThreads = false;
        // If supplied, length must equal (mNumWorkerThreads + mNumIOThreads)
        // Each entry is a Windows logical processor index. Ignored if mPinThreads == false.
        std::vector<int> mCoreMap;

        // Scheduler & spin
        std::chrono::microseconds mSpinWaitTime{ 1000 };
        bool mCanWorkSteal = false;
        bool mDeterministic = true;

        // Hybrid allocation
        size_t mThreadArenaSizeBytes = 8 * 1024 * 1024;
        size_t mSmallObjectThresholdBytes = 64;

        // Debug/Safety
        bool mValidateGraphOnSubmit = false;
    };

}

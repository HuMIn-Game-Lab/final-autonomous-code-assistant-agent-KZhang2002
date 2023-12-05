#ifndef LAB_1_MULTITHREADING_KZHANG2002_JOBSYSTEM_H
#define LAB_1_MULTITHREADING_KZHANG2002_JOBSYSTEM_H

#pragma once
#include <deque>
#include <utility>
#include <vector>
#include <mutex>
#include <cstring>
#include "JobFactory.h"
#include "DefaultJobFactory.h"

constexpr int JOB_TYPE_ANY = -1;

class JobWorkerThread;

enum JobStatus{
    JOB_STATUS_NEVER_SEEN,
    JOB_STATUS_QUEUED,
    JOB_STATUS_RUNNING,
    JOB_STATUS_COMPLETED,
    JOB_STATUS_RETIRED,
    NUM_JOB_STAUSES
};

extern const char* JobStatusNames[];

struct JobHistoryEntry{
    JobHistoryEntry( int jobID, std::string jobType, JobStatus jobStatus, std::string name)
            : m_jobID(jobID)
            , m_jobType(move(jobType))
            , m_jobStatus(jobStatus)
            , m_jobName(move(name)){};

	JobHistoryEntry( int jobID, std::string jobType, JobStatus jobStatus, std::string name, std::string condition)
			: m_jobID(jobID)
			, m_jobType(move(jobType))
			, m_jobStatus(jobStatus)
			, m_jobName(move(name))
			, condition(move(condition)){};



    int m_jobID;
    std::string m_jobType = "N/A";
    int m_jobStatus = JOB_STATUS_NEVER_SEEN;
    std::string m_jobName = "N/A";
	std::string condition = "N/A";
};

class Job;

class JobSystem{
    friend class JobWorkerThread;


public:
    JobSystem();
    ~JobSystem();

    static JobSystem* CreateOrGet();
    void Start();
    void Stop();
    static void Destroy();
    int totalJobs = 0;

    void CreateWorkerThread( const char* uniqueName, unsigned long workerJobChannels=0xFFFFFFFF);
    void DestroyWorkerThread( const char* uniqueName);
    void QueueJob(Job* job);

    //Job Factory
    void RegisterJobFactory(const std::string& jobType, JobFactory* factory);
    void CreateAndQueueJob(const json& params);
    static json ReadFile(std::ifstream& ifs) { return json::parse(ifs); }

    //Status Queries
    JobStatus GetJobStatus(int jobID) const;
    void PrintJobStatus(int jobID) const;
    void FinishCompletedJobs();
    bool IsJobComplete(int jobID) const;
    void FinishJob(int jobID);
    void DestroyJob(int jobID);
    void PrintAllJobsStatuses() const;
	std::pair<bool, std::string> GetJobStatusByName(const std::string& name) const;

private:
    Job* ClaimAJob(unsigned long workerJobFlags);
    void OnJobCompleted(Job* jobJustExecuted);
    static JobSystem* s_jobSystem;
    std::vector< JobWorkerThread* > m_workerThreads;

    bool isStopped = false;

    std::deque< Job* >              m_jobsQueued;
    std::deque< Job* >              m_jobsRunning;
    std::deque< Job* >              m_jobsCompleted;

    mutable std::mutex              m_jobsQueuedMutex;
    mutable std::mutex              m_jobsRunningMutex;
    mutable std::mutex              m_jobsCompletedMutex;
    mutable std::mutex              m_workerThreadsMutex;

    std::vector<JobHistoryEntry>    m_jobHistory;
    mutable int                     m_jobHistoryLowestActiveIndex = 0;
    mutable std::mutex              m_jobHistoryMutex;

    DefaultJobFactory*               	m_defaultFactory = new DefaultJobFactory;
	std::map<std::string, JobFactory*>	m_jobFactories =
        {
                {"compile", m_defaultFactory},
                {"render", m_defaultFactory},
				{"chat", m_defaultFactory}
        };
    mutable std::mutex              	m_jobFactoriesMutex;
};

#endif //LAB_1_MULTITHREADING_KZHANG2002_JOBSYSTEM_H

#include "JobSystem.h"
#include "JobWorkerThread.h"

JobSystem *JobSystem::s_jobSystem = nullptr;

const char* JobStatusNames[] = { "NEVER SEEN", "QUEUED", "RUNNING" , "COMPLETED", "RETIRED"};

using JobCallback = void (*)(Job *completedJob);

JobSystem::JobSystem() { m_jobHistory.reserve(256 * 1024); }

JobSystem::~JobSystem() {
    m_workerThreadsMutex.lock();
    int numWorkerThreads = (int) m_workerThreads.size();

    for (int i = 0; i < numWorkerThreads; ++i) {
        m_workerThreads[i]->ShutDown();
    }

    while (!m_workerThreads.empty()) {
        delete m_workerThreads.back();
        m_workerThreads.pop_back();
    }

    m_workerThreadsMutex.unlock();
}

JobSystem *JobSystem::CreateOrGet() {
    if (s_jobSystem == nullptr) {
        s_jobSystem = new JobSystem();
    }

    return s_jobSystem;
}

void JobSystem::Destroy() {
    if (s_jobSystem != nullptr) {
        delete s_jobSystem;
        s_jobSystem = nullptr;
    }
}

void JobSystem::CreateWorkerThread(const char *uniqueName,
                                   unsigned long workerJobChannels) {

    JobWorkerThread *newWorker =
            new JobWorkerThread(uniqueName, workerJobChannels, this);

    m_workerThreadsMutex.lock();
    m_workerThreads.push_back(newWorker);
    m_workerThreads.back()->StartUp();
    m_workerThreadsMutex.unlock();
}

void JobSystem::DestroyWorkerThread(const char *uniqueName) {
    m_workerThreadsMutex.lock();
    JobWorkerThread *doomedWorker = nullptr;
    std::vector<JobWorkerThread*>::iterator it = m_workerThreads.begin();

    for (; it != m_workerThreads.end(); ++it) {
        if (strcmp((*it)->m_uniqueName, uniqueName) == 0) {
            doomedWorker = *it;
            m_workerThreads.erase(it);
            break;
        }
    }

    if (doomedWorker) {
        doomedWorker->ShutDown();
        delete doomedWorker;
    }

    m_workerThreadsMutex.unlock();
}

void JobSystem::QueueJob(Job *job) {
    m_jobHistoryMutex.lock();
    m_jobHistory.emplace_back(JobHistoryEntry(job->m_jobID,
                                              job->m_jobType,
                                              JOB_STATUS_QUEUED,
                                              job->m_jobName,
											  job->condition));
    m_jobHistoryMutex.unlock();

    m_jobsQueuedMutex.lock();
    m_jobsQueued.push_back(job);
    m_jobsQueuedMutex.unlock();
}

JobStatus JobSystem::GetJobStatus(int jobID) const {
    JobStatus jobStatus = JOB_STATUS_NEVER_SEEN;

    //std::cout << "Job ID received: " << jobID << std::endl;
    //std::cout << "Job History Size: " << m_jobHistory.size() << std::endl;

    m_jobHistoryMutex.lock();
    if (jobID < (int) m_jobHistory.size()) {
        jobStatus = (JobStatus) m_jobHistory[jobID].m_jobStatus;
    }
    m_jobHistoryMutex.unlock();

    //std::cout << "Job Status assigned: " << jobStatus << std::endl;

    return jobStatus;
}

bool JobSystem::IsJobComplete(int jobID) const {
    return (GetJobStatus(jobID)) == JOB_STATUS_COMPLETED;
}

void JobSystem::FinishCompletedJobs() {
    std::deque<Job *> jobsCompleted;

    m_jobsCompletedMutex.lock();
    jobsCompleted.swap(m_jobsCompleted);
    m_jobsCompletedMutex.unlock();

    m_jobHistoryMutex.lock();
    for (Job *job: jobsCompleted) {
        job->JobCompleteCallback();
        m_jobHistory[job->m_jobID].m_jobStatus = JOB_STATUS_RETIRED;
        delete job;
    }
    m_jobHistoryMutex.unlock();
}

void JobSystem::FinishJob(int jobID) {
    JobStatus jobStatus = GetJobStatus(jobID);
    if (jobStatus == JOB_STATUS_NEVER_SEEN || jobStatus == JOB_STATUS_RETIRED) {
        std::cout << "ERROR waiting for job " << jobID << " no such job" << std::endl;
        return;
    }

    m_jobsCompletedMutex.lock();
    Job *thisCompletedJob = nullptr;
    for (auto jcIter = m_jobsCompleted.begin(); jcIter != m_jobsCompleted.end(); ++jcIter) {
        Job *someCompletedJob = *jcIter;
        if (someCompletedJob->m_jobID == jobID) {
            thisCompletedJob = someCompletedJob;
            m_jobsCompleted.erase(jcIter);
            break;
        }
    }
    m_jobsCompletedMutex.unlock();

    if (thisCompletedJob == nullptr) {
        std::cout << "ERROR: Job #" << jobID << "was status complete but not found in the history";
        return;
    }

    thisCompletedJob->JobCompleteCallback();

    m_jobHistoryMutex.lock();
    m_jobHistory[thisCompletedJob->m_jobID].m_jobStatus = JOB_STATUS_RETIRED;
    m_jobHistoryMutex.unlock();

    delete thisCompletedJob;
}

void JobSystem::DestroyJob(int jobID) {
    JobStatus jobStatus = GetJobStatus(jobID);
    if (jobStatus == JOB_STATUS_NEVER_SEEN || jobStatus == JOB_STATUS_RETIRED) {
        std::cout << "ERROR waiting for job " << jobID << " no such job" << std::endl;
        return;
    }

    m_jobsCompletedMutex.lock();
    Job *thisCompletedJob = nullptr;
    for (auto jcIter = m_jobsCompleted.begin(); jcIter != m_jobsCompleted.end(); ++jcIter) {
        Job *someCompletedJob = *jcIter;
        if (someCompletedJob->m_jobID == jobID) {
            thisCompletedJob = someCompletedJob;
            m_jobsCompleted.erase(jcIter);
            break;
        }
    }
    m_jobsCompletedMutex.unlock();

    if (thisCompletedJob == nullptr) {
        std::cout << "ERROR: Job #" << jobID << "was status complete but not found in the history";
        return;
    }

    thisCompletedJob->JobCompleteCallback();

    m_jobHistoryMutex.lock();
    m_jobHistory[thisCompletedJob->m_jobID].m_jobStatus = JOB_STATUS_RETIRED;
    m_jobHistoryMutex.unlock();

    delete thisCompletedJob;
}

void JobSystem::OnJobCompleted(Job *jobJustExecuted) {
    totalJobs++;
    m_jobsCompletedMutex.lock();
    m_jobsRunningMutex.lock();

    std::deque<Job *>::iterator runningJobItr = m_jobsRunning.begin();

    for (; runningJobItr != m_jobsRunning.end(); ++runningJobItr) {
        if (jobJustExecuted == *runningJobItr) {
            m_jobHistoryMutex.lock();
            m_jobsCompleted.push_back(jobJustExecuted);
            m_jobHistory[jobJustExecuted->m_jobID].m_jobStatus = JOB_STATUS_COMPLETED;
            m_jobHistoryMutex.unlock();
            break;
        }
    }
    m_jobsRunningMutex.unlock();
    m_jobsCompletedMutex.unlock();
}

Job *JobSystem::ClaimAJob(unsigned long workerJobChannels) {
    m_jobsQueuedMutex.lock();
    m_jobsRunningMutex.lock();

    Job *ClaimedJob = nullptr;
    std::deque<Job *>::iterator queuedJobIter = m_jobsQueued.begin();

    for (; queuedJobIter != m_jobsQueued.end(); ++queuedJobIter) {
        Job *queuedJob = *queuedJobIter;

        if ((queuedJob -> m_jobChannels & workerJobChannels) != 0) {
            ClaimedJob = queuedJob;

            m_jobHistoryMutex.lock();
            m_jobsQueued.erase(queuedJobIter);
            m_jobsRunning.push_back(ClaimedJob);
            //m_jobsRunning.push_back(ClaimedJob);
            m_jobHistory[ClaimedJob -> m_jobID].m_jobStatus = JOB_STATUS_RUNNING;
            m_jobHistoryMutex.unlock();
            m_jobsRunningMutex.unlock();
            m_jobsQueuedMutex.unlock();
            break;
        }
    }

    m_jobsRunningMutex.unlock();
    m_jobsQueuedMutex.unlock();

    return ClaimedJob;
}

void JobSystem::PrintAllJobsStatuses() const {
    std::cout << "\n-----" << std::endl;
    std::cout << "Job Statuses:" << std::endl;

    m_jobHistoryMutex.lock();
    for (const JobHistoryEntry& job : m_jobHistory) {
        int jobID = job.m_jobID;
        std::cout << "  "
        << "ID: " << jobID << " | "
        << "Name: " << job.m_jobName << " | "
        << "Type: " << job.m_jobType << " | "
        << "Status: " << JobStatusNames[job.m_jobStatus] << " | "
		<< "Condition: " << job.condition << std::endl;
    }
    m_jobHistoryMutex.unlock();

    std::cout << "-----\n" << std::endl;
}

void JobSystem::PrintJobStatus(int jobID) const {
    m_jobHistoryMutex.lock();

    JobHistoryEntry job = m_jobHistory[jobID];
    std::cout
         << "ID: " << jobID << " | "
         << "Name: " << job.m_jobName << " | "
         << "Type: " << job.m_jobType << " | "
         << "Status: " << JobStatusNames[job.m_jobStatus] << " | "
		 << "Condition: " << job.condition << std::endl;

    m_jobHistoryMutex.unlock();
}

void JobSystem::Start() {
    if (!isStopped) {
        std::cout << "Error: Job system is already running." << std::endl;
        return;
    }

    m_workerThreadsMutex.lock();
    m_workerThreads.back()->StartUp();
    m_workerThreadsMutex.unlock();

    isStopped = false;

    std::cout << "Job system has been started." << std::endl;
}

void JobSystem::Stop() {
    if (isStopped) {
        std::cout << "Error: Job system is already stopped." << std::endl;
        return;
    }

    m_workerThreadsMutex.lock();
    int numWorkerThreads = (int) m_workerThreads.size();
    for (int i = 0; i < numWorkerThreads; ++i) {
        m_workerThreads[i]->ShutDown();
    }
    m_workerThreadsMutex.unlock();

    isStopped = true;

    std::cout << "Job system has been stopped." << std::endl;
}

void JobSystem::RegisterJobFactory(const std::string &jobType, JobFactory* factory) {
    auto result = m_jobFactories.emplace(jobType, factory);

    if (!result.second) {
        std::cout << "ERROR: Job type of that name already exists." << std::endl;
    } else {
        std::cout << "Job type with name \"" << jobType << "\" has been added." << std::endl;
    }
}

void JobSystem::CreateAndQueueJob(const json& params) {
    std::string jobType = params["jobType"];

    m_jobFactoriesMutex.lock();

    if (m_jobFactories.find(jobType) == m_jobFactories.end()) {
        std::cout << "ERROR: Given job type not found in registry. If you have a custom job, please register it first." << std::endl;
        m_jobFactoriesMutex.unlock();
        return;
    }

    auto factory = m_jobFactories[jobType];
    auto job = factory -> CreateJob(params);
    m_jobFactoriesMutex.unlock();

    QueueJob(job);
}

std::pair<bool, std::string> JobSystem::GetJobStatusByName(const std::string& name) const {
	//return false if job not found
	std::pair<bool, std::string> res{false, ""};
	m_jobHistoryMutex.lock();
	for (const JobHistoryEntry& job : m_jobHistory) {
		int jobID = job.m_jobID;
		if (job.m_jobName == name && (job.m_jobStatus == JOB_STATUS_COMPLETED || job.m_jobStatus == JOB_STATUS_RETIRED)) {
			res.first = true;
			res.second = job.condition;
			break;
		}
	}
	m_jobHistoryMutex.unlock();

	return res;
}

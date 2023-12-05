#include "JobWorkerThread.h"
#include "JobSystem.h"

JobWorkerThread::JobWorkerThread(
        const char* uniqueName,
        unsigned long workerJobChannels,
        JobSystem* JobSystem)
        : m_uniqueName(uniqueName), m_workerJobChannels(workerJobChannels),
          m_jobSystem(JobSystem) {}

JobWorkerThread::~JobWorkerThread() {
    ShutDown();

    m_thread -> join();

    delete m_thread;
    m_thread = nullptr;
}

void JobWorkerThread::StartUp() {
    m_workerStatusMutex.lock();
    m_isStopping = false;
    m_workerStatusMutex.unlock();

    m_thread = new std::thread(WorkerThreadMain, this);
}

void JobWorkerThread::Work(){
    //cout << "Work called" << endl;
    while(!IsStopping()){
        m_workerStatusMutex.lock();
        unsigned long workerJobChannels = m_workerJobChannels;
        m_workerStatusMutex.unlock();

        Job* job = m_jobSystem->ClaimAJob(workerJobChannels);
        if(job){
            job->Execute();
            m_jobSystem->OnJobCompleted(job);
        }

        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

void JobWorkerThread::ShutDown(){
    m_workerStatusMutex.lock();
    m_isStopping = true;
    m_workerStatusMutex.unlock();
}

bool JobWorkerThread::IsStopping() const {
    bool isStopping;
    m_workerStatusMutex.lock();
    isStopping = m_isStopping;
    m_workerStatusMutex.unlock();

    return isStopping;
}

void JobWorkerThread::SetWorkerJobChannels(unsigned long workerJobChannels) {
    m_workerStatusMutex.lock();
    m_workerJobChannels = workerJobChannels;
    m_workerStatusMutex.unlock();
}

void JobWorkerThread::WorkerThreadMain(JobWorkerThread *workerThreadObject) {
    JobWorkerThread* thisWorker = (JobWorkerThread*) workerThreadObject;
    thisWorker->Work();
}



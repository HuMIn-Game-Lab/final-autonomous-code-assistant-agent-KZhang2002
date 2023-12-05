#ifndef LAB_1_MULTITHREADING_KZHANG2002_JOBWORKERTHREAD_H
#define LAB_1_MULTITHREADING_KZHANG2002_JOBWORKERTHREAD_H

# include "Job.h"
#include <pthread.h>

class JobSystem;

class JobWorkerThread {
    friend class JobSystem;

private:
    JobWorkerThread(
            const char* uniqueName,
            unsigned long workerJobChannels,
            JobSystem* JobSystem
    );

    ~JobWorkerThread();

    void StartUp(); //Kick off actual thread. which will call Work()
    void Work(); //Called in private thread, blocks forever until StopWorking() is true

    void ShutDown(); //Signal that work should stop at next opportunity


    bool IsStopping() const; //missing
    void SetWorkerJobChannels(unsigned long workerJobChannels);
    static void WorkerThreadMain(JobWorkerThread *workThreadObject);

private:
    const char*             m_uniqueName;
    unsigned long           m_workerJobChannels = 0xffffffff;
    bool                    m_isStopping = false;
    JobSystem*              m_jobSystem = nullptr;
    std::thread*            m_thread = nullptr;

    mutable std::mutex      m_workerStatusMutex;
};

#endif //LAB_1_MULTITHREADING_KZHANG2002_JOBWORKERTHREAD_H

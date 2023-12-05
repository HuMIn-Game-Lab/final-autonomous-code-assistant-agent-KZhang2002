#ifndef LAB_1_MULTITHREADING_KZHANG2002_JOB_H
#define LAB_1_MULTITHREADING_KZHANG2002_JOB_H

#pragma once
#include <iostream>
#include <utility>
#include <vector>
#include <array>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include "json.hpp"

using json = nlohmann::json; //may be bad practice

class Job {
    friend class JobSystem;
    friend class JobWorkerThread;

public:
    //To be deprecated. Don't use this. Left in for testing purposes.
//    explicit Job(unsigned long jobChannels = 0xFFFFFFFF, std::string jobName = "N/A") :
//    m_jobChannels(jobChannels), m_jobName(move(jobName)) {
//        static int s_nextJobID = 0;
//        m_jobID = s_nextJobID++;
//    }

    explicit Job(json &input) : m_inputJSON(input) {
        static int s_nextJobID = 0;
        m_jobID = s_nextJobID++;

        if (input.find("jobType") == input.end()) {
            std::cout << "No job type given. Assigning job default channel \"N/A\"." << std::endl;
        } else {
            //std::cout << input["jobType"] << std::endl;
            m_jobType = input["jobType"];
        }

        if (input.find("jobChannels") == input.end()) {
            std::cout << "No job channels given. Assigning job default channel \"0xFFFFFFFF\"." << std::endl;
        } else {
            //std::cout << input["jobChannels"] << std::endl;
            m_jobChannels = stoul((std::string)input["jobChannels"], nullptr, 16);
        }

        if (input.find("jobName") == input.end()) {
            std::cout << "No name given. Assigning job name \"N/A\"." << std::endl;
        } else {
            std::string name = input["jobName"];
            if (name.empty()) std::cout << "Empty name given. Assigning job name \"N/A\"." << std::endl;
            m_jobName = name;
        }

		if (input.find("callback") == input.end()) {
			std::cout << "No callback given. Assigning callback \"N/A\"." << std::endl;
		} else {
			std::string callback = input["callback"];
			if (callback.empty()) std::cout << "Empty callback given. Assigning callback \"N/A\"." << std::endl;
			condition = callback;
		}
    };


    virtual ~Job() {}

    virtual void Execute() = 0; //Pure virtual function
    virtual void JobCompleteCallback(){};
    int GetUniqueID() const { return m_jobID; }
    std::string GetJobType() const { return m_jobType; }

private:

    int m_jobID                 = -1;
    std::string m_jobType            = "N/A";

    unsigned long m_jobChannels = 0xFFFFFFFF;
    std::string m_jobName            = "N/A";
	std::string condition            = "N/A";

    json m_inputJSON;
};

#endif //LAB_1_MULTITHREADING_KZHANG2002_JOB_H

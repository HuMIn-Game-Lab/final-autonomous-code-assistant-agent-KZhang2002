//
// Created by Kenny on 10/23/2023.
//

#include "CustomJobFactory.h"

Job *CustomJobFactory::CreateJob(const json& params) {
    if (params.find("jobType") == params.end()) {
        std::cout << "ERROR:No job type given. Aborting job creation." << std::endl;
        return nullptr;
    }
    std::string jobType = params["jobType"];
    std::string jobName = params["jobName"];
    std::cout << "\nCreating job of type \"" << jobType << "\" with name \"" << jobName << "\"." << std::endl;

    if (jobType.compare("custom") == 0) {
        return new CustomJob(params);
    } else {
        std::cout << "ERROR: invalid jobType given. Returning nullptr." << std::endl;
        return nullptr;
    }
}


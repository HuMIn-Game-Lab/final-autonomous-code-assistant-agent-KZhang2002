//
// Created by Kenny on 10/23/2023.
//

#include "DefaultJobFactory.h"

Job *DefaultJobFactory::CreateJob(const json& params) {
    if (params.find("jobType") == params.end()) {
        std::cout << "ERROR: No job type given. Aborting job creation." << std::endl;
        return nullptr;
    }

    std::string jobType = params["jobType"];
    std::string jobName = params["jobName"];
    std::cout << "\nCreating job of type \"" << jobType << "\" with name \"" << jobName << "\"." << std::endl;

	if (params.find("callback") != params.end()) {

	}

    if (jobType.compare("compile") == 0) {
        return new CompileJob(params);
    } else if (jobType.compare("render") == 0) {
        return new RenderJob(params);
    } else if (jobType.compare("chat") == 0) {
		return new ChatJob(params);
	} else {
        std::cout << "ERROR: invalid jobType given. Returning nullptr." << std::endl;
        return nullptr;
    }
}

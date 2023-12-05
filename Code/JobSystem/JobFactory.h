//
// Created by Kenny on 10/21/2023.
//

#ifndef LAB_2_FLOWSCRIPT_DSL_KZHANG2002_JOBFACTORY_H
#define LAB_2_FLOWSCRIPT_DSL_KZHANG2002_JOBFACTORY_H

#pragma once
#include "Job.h"
#include "CompileJob.h"
#include "json.hpp"
#include <fstream>

class JobFactory {
public:
    virtual Job* CreateJob(const json& params) = 0;
    virtual ~JobFactory() {}
};


#endif //LAB_2_FLOWSCRIPT_DSL_KZHANG2002_JOBFACTORY_H

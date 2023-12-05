//
// Created by Kenny on 10/22/2023.
//

#ifndef LAB_2_FLOWSCRIPT_DSL_KZHANG2002_CUSTOMJOBFACTORY_H
#define LAB_2_FLOWSCRIPT_DSL_KZHANG2002_CUSTOMJOBFACTORY_H

#pragma once
#include "JobFactory.h"
#include "CustomJob.h"

class CustomJobFactory : public JobFactory {
public:
    Job* CreateJob(const json& params) override;
};


#endif //LAB_2_FLOWSCRIPT_DSL_KZHANG2002_CUSTOMJOBFACTORY_H

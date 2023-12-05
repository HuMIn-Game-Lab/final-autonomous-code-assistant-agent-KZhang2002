//
// Created by Kenny on 10/23/2023.
//

#ifndef LAB_2_FLOWSCRIPT_DSL_KZHANG2002_DEFAULTJOBFACTORY_H
#define LAB_2_FLOWSCRIPT_DSL_KZHANG2002_DEFAULTJOBFACTORY_H

#pragma once
#include "JobFactory.h"
#include "CompileJob.h"
#include "RenderJob.h"
#include "ChatJob.h"


class DefaultJobFactory : public JobFactory{
public:
    Job* CreateJob(const json& params) override;
};


#endif //LAB_2_FLOWSCRIPT_DSL_KZHANG2002_DEFAULTJOBFACTORY_H

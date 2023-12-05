//
// Created by Kenny on 10/22/2023.
//

#ifndef LAB_2_FLOWSCRIPT_DSL_KZHANG2002_CUSTOMJOB_H
#define LAB_2_FLOWSCRIPT_DSL_KZHANG2002_CUSTOMJOB_H

#pragma once
#include "Job.h"

class CustomJob : public Job {
public:
//    CustomJob(unsigned long jobChannels = 0xFFFFFFFF, string jobName = "N/A")
//            : Job(jobChannels, jobName) {}

    explicit CustomJob(json params);
    ~CustomJob() override = default;

    std::vector<int> data;

    void Execute() override;
    void JobCompleteCallback() override;
};

#endif //LAB_2_FLOWSCRIPT_DSL_KZHANG2002_CUSTOMJOB_H

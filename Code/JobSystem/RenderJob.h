#pragma once
#include "Job.h"
#include <iostream>
#include <ostream>
#include <vector>

class RenderJob : public Job {
  public:
//    RenderJob(unsigned long jobChannels = 0xFFFFFFFF, string jobName = "N/A")
//        : Job(jobChannels, jobName) {};

    explicit RenderJob(json params);
    ~RenderJob() override= default;

    std::vector<int> data;

    void Execute() override;
    void JobCompleteCallback() override;
};

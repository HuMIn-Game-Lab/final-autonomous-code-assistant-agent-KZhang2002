#include "CustomJob.h"
#include <vector>

CustomJob::CustomJob(json params) : Job(params) {
    for (int i = 0; i < 1000; i++) {
        data.push_back(i);
    }
}

void CustomJob::Execute() {
    int total = 0;
    std::vector<int>::iterator it = data.begin();

    for (; it != data.end(); ++it) {
        total += *it;
    }

    it = data.begin();

    for (; it != data.end(); ++it) {
        total += *it;
    }

    data[0] = total; // [0] is the sum of the vector
    std::cout << "Job: " << this->GetUniqueID() << " has been executed"
              << std::endl;
}

void CustomJob::JobCompleteCallback() {
    std::cout << "\nJob: " << this->GetUniqueID() << " has been completed | "
              << "Calculated Sum: " << this->data[0] << std::endl;
}



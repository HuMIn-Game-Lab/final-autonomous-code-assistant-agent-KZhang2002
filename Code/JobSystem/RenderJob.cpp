#include "RenderJob.h"
#include <vector>

RenderJob::RenderJob(json params) : Job(params) {
	if (params.find("callback") == params.end()) {
		std::cout << "No inputs given. Aborting job." << std::endl;
		return;
	}

    for (int i = 0; i < 1000; i++) {
        data.push_back(i);
    }
}

void RenderJob::Execute() {
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

void RenderJob::JobCompleteCallback() {
    std::cout << "\nJob: " << this->GetUniqueID() << " has been completed | "
              << "Calculated Sum: " << this->data[0] << std::endl;
}

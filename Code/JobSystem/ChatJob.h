#ifndef LAB_1_MULTITHREADING_KZHANG2002_CHATJOB_H
#define LAB_1_MULTITHREADING_KZHANG2002_CHATJOB_H

#include "Job.h"
#include <string>
#include <utility>
#include "json.hpp"
#include "openai.hpp"
#include <cstdio>
#include <curl/curl.h>

class ChatJob : public Job {
public:

    explicit ChatJob(json input);

    ~ChatJob(){};

	std::string endpoint = "";
	std::string model = "";
	std::string prompt = "";
    int returnCode = 0;

    void Execute();
	std::string Chat();
    void JobCompleteCallback();

private:
    void OutputAsJSON();

    std::string m_jobType = "chat";
	std::string response;
	bool isAborted = false;

    nlohmann::json output;

	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response);
};


#endif //LAB_1_MULTITHREADING_KZHANG2002_CHATJOB_H

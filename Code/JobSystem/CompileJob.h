#ifndef LAB_1_MULTITHREADING_KZHANG2002_COMPILEJOB_H
#define LAB_1_MULTITHREADING_KZHANG2002_COMPILEJOB_H

#include "Job.h"
#include <string>
#include <utility>
#include "json.hpp"

class CompileJob : public Job {
public:
    //To be deprecated. Don't use this. Left in for testing purposes.
//    CompileJob(unsigned long jobChannels, std::string makefileContents, std::string outputFileName)
//            : Job(jobChannels, "compile"){
//        this->makefileContents = move(makefileContents);
//        this->outputFileName = move(outputFileName);
//    };

    explicit CompileJob(json input);

    ~CompileJob(){};

    std::string makefileContents;
    std::string outputFileName;
    std::string output;
	std::unordered_map<std::string, std::vector<std::string>> codeFile;
    int returnCode = 0;

    void Execute();
    void JobCompleteCallback();

private:
    void Compile();
    void ParseError();
    void OutputToJSON();

    std::string m_jobType = "compile";

    nlohmann::json compilationErrors;
};


#endif //LAB_1_MULTITHREADING_KZHANG2002_COMPILEJOB_H

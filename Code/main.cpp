#include "JobSystem.h"
#include "CustomJob.h"
#include "CustomJobFactory.h"
#include "FlowScriptInterpreter.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <curl/curl.h>
#include "openai.hpp" //remove later
#include <filesystem>

using namespace std;

string writePrompt(string codeTxt, string errorText) {
	stringstream output;
	output <<
	R"(
Task: Use the error json to fix the errors in the provided code and output them in the provided format.
Output Format:
Error [NUM]:
Error Type: [ERROR TYPE]
Original Line(s):
[ERROR CODE]
Fixed Line(s):
Explanation:
[EXPLANATION/ADVICE ON HOW TO FIX CODE]

Example:
"main.cpp":
[MAIN.CPP]
Error JSON:
[ERROR JSON FOR MAIN.CPP]
Error 1:
Error Type: Syntax Error
Original Line(s):
int missing_semicolon
Fixed Line(s):
int missing_semicolon;
Explanation:
Missing semicolon
Error 2:
Error Type: Linker Error
Original Line(s):
//#include "test1.h"
Fixed Line(s):
#include "test1.h"
Explanation:
Uncommented needed file
Error 3:
Error Type: Uninitialized variable
Original Line(s):
int uninitialized_variable;
int result = uninitialized_variable;
Fixed Line(s):
[NO FIX]
Explanation:
uninitialized_variable needs to be given a value before being used.
[EXAMPLE END])";

	//output << "Code:\n" << codeTxt;
	output << "Errors:\n" << errorText;

	//cout << "Prompt output: \n" << output.str() << endl;

	return output.str();
}

int main (){
//	std::filesystem::path currentPath = std::filesystem::current_path();
//	std::cout << "Current working directory: " << currentPath.string() << std::endl;

	JobSystem* js = JobSystem::CreateOrGet();

	std::cout << "Creating Worker Threads" << std::endl;

	js -> CreateWorkerThread("Thread1", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread2", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread3", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread4", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread5", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread6", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread7", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread8", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread9", 0xFFFFFFFF);

	//To change the error file analyzed, change the location of the json files below. Look in the jobJSONs folder to find them.

	ifstream coj("../Data/jobJSONs/compileJobErrorTest.json");
	if (coj.good()) js->CreateAndQueueJob(JobSystem::ReadFile(coj));
	else cout << "File read error" << endl;
	coj.close();

	ifstream etj("../Data/output/errorTest.json");
	string errorText((istreambuf_iterator<char>(etj)), istreambuf_iterator<char>());
	etj.close();

	ifstream chj("../Data/jobJSONs/chatJob1.json");
	string rawText((istreambuf_iterator<char>(chj)), istreambuf_iterator<char>());
	chj.close();

	ifstream code("../Data/output/errorTestCodeFile.json");
	string codeText((istreambuf_iterator<char>(code)), istreambuf_iterator<char>());
	code.close();

//	cout << "Chat Job: " << rawText << endl;

	json chatJobJSON = json::parse(rawText);
//	cout << "error: " << errorText << endl;
//	cout << "chat job: " << rawText << endl;
	string prompt = writePrompt(codeText, errorText);
	chatJobJSON["input"]["prompt"] = prompt;
	ofstream out("../Data/jobJSONs/chatJob1.json");
	out << chatJobJSON.dump(4) << endl;
	out.close();

	js->CreateAndQueueJob(chatJobJSON);

	int running = 1;
	int curJobID = 0;

	while (running) {
		string command;
		cout << "Enter stop, start, destroy, finish, status, job status, destroyjob, or finishjob\n";
		cin >> command;

		if (command == "stop") {
			js -> Stop();
		}
		else if (command == "start") {
			js -> Start();
		}
		else if (command == "destroy") {
			js -> FinishCompletedJobs();
			js -> Destroy();
			running = 0;
			break;
		}
		else if (command == "destroyjob") {
			js -> DestroyJob(0);
		}
		else if (command == "jobstatus") {
			js -> PrintJobStatus(0);
		}
		else if (command == "finish"){
			js->FinishCompletedJobs();
			cout << "Total jobs completed " << js->totalJobs << endl;
		}
		else if (command == "finishjob"){
			cout << "Finishing Job " << curJobID << endl;
			js->FinishJob(curJobID);
			curJobID++;
		}
		else if(command == "status") {
			js->PrintAllJobsStatuses();
		}
		else {
			cout << "Invalid Command" << endl;
		}
	}

	js -> FinishCompletedJobs();

	js -> Destroy();

	return 0;
}



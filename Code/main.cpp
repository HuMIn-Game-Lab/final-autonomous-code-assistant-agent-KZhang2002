#include "JobSystem.h"
#include "CustomJob.h"
#include "CustomJobFactory.h"
#include "FlowScriptInterpreter.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <filesystem>

using namespace std;

string flowPrompt(string jobName) {
	stringstream output;
	output <<
	R"(
Using flowscript, a language based on DOT, create a graph.

Flowscript Example:
digraph conditionalTestSuccess{
   A;
   B-> A;
   C-> A;

   D-> B;
   D-> C;
   E-> F;
}

Available Jobs:
compileJobErrorTest.json
compileJobSuccessTest.json
customJob.json
renderJob.json
renderJobA.json
renderJobBfail.json
renderJobBsuccess.json
renderJobC.json
renderJobD.json
renderJobE.json
renderJobF.json

Make a graph that executes the job ")";
	output << jobName;
	output << R"(.json". To do this, make a digraph with only one node: ")";
	output << jobName;
	output << R"(. Your response should have the code and nothing else.)";

	return output.str();
}

string writePrompt(string code, json errors) {
	stringstream output;
	output << code;
	output << "\nIn the previous code there are the following errors:\n";

	for (auto file : errors) {
		for (auto error : file) {
			output << "Error on line " << error["line_number"];
			output << ", column " << error["column_number"] << ":\n";
			output << "\t " << error["description"] << "\n";
		}
	}

	output << "\n Fix the code. Your response should include nothing but raw code. Do not put the code into a special format.";

	return output.str();
}

void pause() {
	int running = 1;
	while(running == 1) {
		string command;
		cout << "Enter any input to continue.\n";
		cin >> command;

		if (!command.empty()) {
			running = 0;
		}
	}
}

void pause(JobSystem* js){
	int running = 1;
	int curJobID = 0;

	while (running == 1) {
		string command;
		cout << "Enter stop, start, destroy, finish, status, job status, destroyjob, or finishjob\n";
		cin >> command;

		if (command == "stop") {
			js -> Stop();
		}
		else if (command == "start") {
			js -> Start();
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
			running = 0;
			break;
		}
	}
}

int main (){
	JobSystem* js = JobSystem::CreateOrGet();
	string relativePathToTargetCPP = "../Code/TestCodeError/test2.cpp";

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


	ifstream chj("../Data/jobJSONs/chatJobFlow.json");
	json chatJobJSON = json::parse(chj);
	chj.close();
	pause(js);
	string prompt = flowPrompt("compileJobErrorTest");
	chatJobJSON["input"]["prompt"] = prompt;
	ofstream out("../Data/jobJSONs/chatJobFlow.json");
	out << chatJobJSON.dump(4) << endl;
	out.close();

	js->CreateAndQueueJob(chatJobJSON);
	pause(js);

	ifstream llmFlowCode("../Data/output/llmOutput.json");
	json flowCode = json::parse(llmFlowCode);
	llmFlowCode.close();
	string response = flowCode["response"];

	string pathToDot = "../Data/graphs/llmAttempt.dot";
	ofstream llmFlowOut(pathToDot);
	llmFlowOut << response;
	llmFlowOut.close();

	auto* flow = new FlowScriptInterpreter(pathToDot);
	if (flow->hasFailed()) {
		cout << "Error processing flowscript graph." << endl;
		return 2;
	}

	while (true) {
		ifstream coj("../Data/jobJSONs/compileJobErrorTest.json");
		cout << "parsing compileJob..." << endl;
		json compileJob = json::parse(coj);
		js->CreateAndQueueJob(compileJob);
		pause(js);
		coj.close();

		ifstream etj("../Data/output/errorTest.json");
		string errorText((istreambuf_iterator<char>(etj)), istreambuf_iterator<char>());
		if (errorText.length() > 1000) {
			cout << "Buffer overflow" << endl;
			break;
		} else if (errorText == "Files compiled successfully. No errors found.") {
			break;
		}
		cout << "parsing errorJSON..." << endl;
		json errorJSON = json::parse(errorText);
		etj.close();

		ifstream errorCode("../Data/output/errorTestCodeFile.txt");
		string faultyCode((istreambuf_iterator<char>(errorCode)), istreambuf_iterator<char>());
		errorCode.close();

		string prompt = writePrompt(faultyCode, errorJSON);

		ifstream chj("../Data/jobJSONs/chatJobCode.json");
		json chatJobCode = chatJobJSON;
		chatJobCode["input"]["prompt"] = prompt;
		chatJobCode["jobName"] = "chatJobCode";
		ofstream out("../Data/jobJSONs/chatJobCode.json");
		out << chatJobCode.dump(4) << endl;
		out.close();

		js->CreateAndQueueJob(chatJobCode);
		pause(js);

		cout << "parsing llmOutput..." << endl;
		ifstream llmFlowCode("../Data/output/llmOutput.json");
		json flowCodeJSON = json::parse(llmFlowCode);
		string flowCode = flowCodeJSON["response"];
		llmFlowCode.close();

		//Put file you want to compile here
		ofstream llmFlowOut(relativePathToTargetCPP);
		llmFlowOut << flowCode;
		llmFlowOut.close();

		flow->Interpret(pathToDot);
	}

	return 0;
}



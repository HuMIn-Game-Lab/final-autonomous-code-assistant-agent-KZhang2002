#include "ChatJob.h"
#include <array>
#include <iostream>
#include <ostream>

//needed for parse job
#include <regex>
#include "json.hpp"
#include <fstream>

ChatJob::ChatJob(json input) : Job(input) {
    if (input.find("input") == input.end()) {
        std::cout << "No inputs given. Aborting job." << std::endl;
        return;
    }

	if (input["input"].find("endpoint") == input["input"].end() ||
			input["input"].find("model") == input["input"].end() ||
			input["input"].find("prompt") == input["input"].end()) {
		std::cout << "Missing inputs. JSON must include endpoint, model, and prompt. Process aborted." << std::endl;
		isAborted = true;
		return;
	}

	endpoint = input["input"].at("endpoint");
	model = input["input"].at("model");
	prompt = input["input"].at("prompt");

	if (endpoint == "" || model == "" || prompt == "") {
		std::cout << "Some inputs empty. Endpoint, model, and prompt must all contain values." << std::endl;
		isAborted = true;
		return;
	}
}

void ChatJob::Execute() {
	if (isAborted) {
		return;
	}

	std::string response = Chat();

	OutputAsJSON();

    std::cout << "Executing chat job" << std::endl;
    std::cout << "Job: " << this->GetUniqueID() << " has been executed"
              << std::endl;
}

size_t ChatJob::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
	size_t totalSize = size * nmemb;
	response->append((char*)contents, totalSize);
	return totalSize;
}

std::string ChatJob::Chat() {
	std::string baseUrl = endpoint;
	CURL* curl = curl_easy_init();

	if (curl) {
		json requestData;
		requestData["model"] = model;
		requestData["messages"][0]["role"] = "user";
		requestData["messages"][0]["content"] = prompt;
		requestData["temperature"] = 0.7;
		requestData["max_tokens"] = 1000;
		requestData["echo"] = true;

		std::string requestDataStr = requestData.dump();

		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, ("Authorization: Bearer "));
		curl_easy_setopt(curl, CURLOPT_URL, baseUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestDataStr.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestDataStr.length());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		CURLcode res = curl_easy_perform(curl);

		//std::cout << answer << std::endl;

		if (res != CURLE_OK) {
			std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
		}

		curl_easy_cleanup(curl);
	}


	json jresponse = json::parse(response);

	std::string answer = jresponse["choices"][0]["message"]["content"].get<std::string>();
	answer = answer.substr(prompt.length() + 2, answer.length());
	response = answer;

	return answer;
}

void ChatJob::JobCompleteCallback() {
    std::cout << "Compile Job " << this->GetUniqueID() << " | Return code " << this->returnCode << " | Status: ";
    if (isAborted) {
		std::cout << "Failed" << std::endl;
    } else {
		std::cout << "True" << std::endl;
    }
}

void ChatJob::OutputAsJSON() {
    // Serialize JSON data to a std::string
    json res;
	res["prompt"] = prompt;
	res["response"] = response;

    // Write JSON data to a file
    std::string outputDirectory = "../Data/output/";
    std::string fileName = "llmOutput.json";
    std::string outputFile = outputDirectory + fileName; // Specify the relative path to the output file
    std::ofstream json_file(outputFile);
    json_file << res.dump(4);
    json_file.close();

    std::cout << "llm prompt and response sent to \"" << outputDirectory << "\"." << std::endl;
}

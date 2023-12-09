#include "CompileJob.h"
#include <array>
#include <iostream>
#include <ostream>
#include <filesystem>

//needed for parse job
#include <regex>
#include "json.hpp"
#include <fstream>
#include <cmath>

CompileJob::CompileJob(json input) : Job(input) {
    if (input.find("input") == input.end()) {
        std::cout << "No inputs given. Aborting job." << std::endl;
        return;
    }

    if (input["input"].find("makefileContents") != input["input"].end()) {
        makefileContents = input["input"].at("makefileContents");
        std::cout << "Makefile contents set as: " << makefileContents << std::endl;
    } else {
        std::cout << "No makefile commands given. Aborting job." << std::endl;
        return;
    }

    if (input["input"].find("outputFileName") != input["input"].end()) {
        outputFileName = input["input"].at("outputFileName");
        std::cout << "Output file name set as: " << outputFileName << std::endl;
    } else {
        outputFileName = "output";
        std::cout << "No output file name given. Output will have the default name \"output\"." << std::endl;
    }
}

void CompileJob::Execute() {
	//std::filesystem::path currentPath = std::filesystem::current_path();
	//std::cout << "Current working directory: " << currentPath.string() << std::endl;

    std::cout << "Executing compile job" << std::endl;
    //std::cout << "Running compile" << std::endl;
    Compile();
    //std::cout << "Parsing errors from compile" << std::endl;
    ParseError();
    //std::cout << "Outputting error to JSON" << std::endl;
    OutputToJSON();

    std::cout << "Job: " << this->GetUniqueID() << " has been executed"
              << std::endl;
}

void CompileJob::JobCompleteCallback() {
    std::cout << "Compile Job " << this->GetUniqueID() << " | Return code " << this->returnCode << " | Output: ";
    if (this->output.empty()) {
        std::cout << "No output. Code compilation successful." << std::endl;
    } else {
        std::cout << "\n\n" << this->output << std::endl;
    }
}

void CompileJob::Compile() {
    std::array<char, 128> buffer;
    std::string command = makefileContents;
    //command += this->makefileContents;

    // Redirect err to std::cout
    command.append(" 2>&1");

    //std::cout << "Running command: " << command << std::endl;

    // open pipe and run command
    FILE *pipe = popen(command.c_str(), "r");
    if(!pipe) {
        std::cout << "popen Failed: Error: failed to open pipe" << std::endl;
        return;
    }

    // read till end of process:
    while(fgets(buffer.data(), 128, pipe) != nullptr) {
        this->output.append(buffer.data());
    }

    // close pipe and get return code
    this->returnCode = pclose(pipe);

    std::cout << "Compiling for job " << this->GetUniqueID() << " has been completed" << std::endl;
}

void CompileJob::ParseError() {
    std::regex error_pattern(R"(([^:\n]+):(\d+):(\d+): (error|note|warning): (.+))");

    std::vector<std::map<std::string, std::string>> errors;
    std::map<std::string, std::vector<std::map<std::string, std::string>>> errors_by_file;

    // Use std::sregex_iterator to iterate over matches in the error messages
	std::sregex_iterator iter(output.begin(), output.end(), error_pattern);
	std::sregex_iterator end;

    for (; iter != end; ++iter) {
        std::smatch match = *iter;

        // Extract matches
        std::string file_path = match[1];
        std::string line_number = match[2];
        std::string column_number = match[3];
        std::string error = match[4];
        std::string description = match[5];

        // Create and fill error object
        std::map<std::string, std::string> error_data;
        error_data["description"] = description;
        error_data["file_path"] = file_path;
        error_data["line_number"] = line_number;
        error_data["column_number"] = column_number;
        error_data["error_type"] = error;

        // Store error data in a std::vector
        errors.push_back(error_data);

        // Group errors by file path
        errors_by_file[file_path].push_back(error_data);
    }

    nlohmann::json jsonOutput;
	for (const auto &file_errors : errors_by_file) {
		if (file_errors.first.find("JetBrains") != std::string::npos) {
			break;
		}
		std::ifstream sourceFileStream(file_errors.first);
		std::string sourceLine;
		int currentLineNumber = 1;
		while (getline(sourceFileStream, sourceLine)) {
			codeFile[file_errors.first].push_back(sourceLine);
			currentLineNumber++;
		}
	}

//	codeFile.push_back(sourceLine);

    // Convert error data to JSON using nlohmann/json
    for (const auto &file_errors : errors_by_file) {
        nlohmann::json json_errors;
        for (const auto &error : file_errors.second) {
            nlohmann::json json_error;
            if (error.find("description") != error.end()) {
                json_error["description"] = error.at("description");
            }
            if (error.find("line_number") != error.end()) {
                json_error["line_number"] = error.at("line_number");
            }
            if (error.find("column_number") != error.end()) {
                json_error["column_number"] = error.at("column_number");
            }
            if (error.find("error_type") != error.end()) {
                json_error["error_type"] = error.at("error_type");
            }
            if (error.find("code_snippet") != error.end()) {
                json_error["code_snippet"] = error.at("code_snippet");
            }

			nlohmann::json contextLines = nlohmann::json::array();
			std::vector<std::string> fileCode = codeFile[file_errors.first];
            int targetLineNumber = stoi(error.at("line_number")) - 1;
			int currentLineNumber = fmax(targetLineNumber - 2, 0);
			int endLineNumber = fmin(targetLineNumber + 3, fileCode.size());

			for (int i = currentLineNumber; i < endLineNumber; i++) {
				contextLines.push_back(fileCode[i]);
			}

            json_error["code_snippet"] = contextLines;

            json_errors.push_back(json_error);
        }
        jsonOutput[file_errors.first] = json_errors;
    }

    compilationErrors = jsonOutput;
}

void CompileJob::OutputToJSON() {
    // Serialize JSON data to a std::string
    std::string json_str = compilationErrors.dump(4); // Pretty-print with 4 spaces

    if (json_str == "null") {
        json_str = "Files compiled successfully. No errors found.";
    }
    // Write JSON data to a file
    std::string outputDirectory = "../Data/output/";
    std::string fileName = outputFileName + ".json";
    std::string outputFile = outputDirectory + fileName; // Specify the relative path to the output file
    std::ofstream json_file(outputFile);
    json_file << json_str;
    json_file.close();

    
	fileName = outputFileName + "CodeFile.txt";
	outputFile = outputDirectory + fileName; // Specify the relative path to the output file
	std::ofstream text_file(outputFile);
	std::string result;

	for (const auto& file : codeFile) {
		result += "Relative Filepath: " + file.first + "\n";
		for (const auto& line : file.second) {
			result += line + "\n";
		}
	}

	//std::cout << "codefile text: " << result << std::endl;
	text_file << result;
	text_file.close();

    std::cout << "g++ error and warning messages converted to JSON and saved to \"" << outputDirectory << "\"." << std::endl;
	std::cout << "Analyzed code put into txt file and saved to \"" << outputFile << "\"." << std::endl;
}

//
// Created by Kenny on 11/7/2023.
//

#include "FlowScriptInterpreter.h"

std::ostream &operator<<(std::ostream &os, const token &token) {
    os << "<" << tokenTypeNames[token.type];
    if (token.type == TWORD) {
        os << " : " << token.value;
    }
    os  << ">";
    return os;
}

bool token::operator==(tokenType other) const {
    return (type == other);
}


FlowScriptInterpreter::FlowScriptInterpreter(const std::string& relativePath) {
    std::ifstream input(relativePath);
    if (input.good()) Interpret(input);
    else std::cout << "ERROR: Given path could not be found." << std::endl;
    input.close();
}

void FlowScriptInterpreter::Interpret(std::ifstream &input) {
    l_Lexer(input);
    if(e_Flag) return;
	std::cout << std::endl;
    printLexResult();
	std::cout << std::endl;
	s_ParseSyntax();
	if(e_Flag) return;

	printMap();

	Execute();
	if(e_Flag) return;
}

void FlowScriptInterpreter::Interpret(std::string &relativePath) {
	Reset();

	std::ifstream input(relativePath);
	if (input.good()) Interpret(input);
	else std::cout << "ERROR: Given path could not be found." << std::endl;
	input.close();
}

void FlowScriptInterpreter::Reset() {
	graphName.clear();
	rawCodeText.clear();
	dependencyMap.clear();
	existingJobs.clear();
	lexResult.clear();

	e_Flag = false;
	e_Value.clear();
	i_ResetIterator();
}

bool FlowScriptInterpreter::Execute() {
	if(e_Flag) return false;

	js -> CreateWorkerThread("Thread1", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread2", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread3", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread4", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread5", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread6", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread7", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread8", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread9", 0xFFFFFFFF);
	js -> CreateWorkerThread("Thread10", 0xFFFFFFFF);

	RunJobs();

	return true;
}

void FlowScriptInterpreter::RunJobs() {
	std::unordered_map<std::string, bool> triedJobs; // <node name, job succeeded?>

	// run all jobs with no dependencies
	for (auto it = existingJobs.begin(); it != existingJobs.end(); ++it) {
		if (dependencyMap[*it].empty()) {
			std::ifstream jobInput("../Data/jobJSONs/" + *it + ".json");
			if (jobInput.good()) {
				js -> CreateAndQueueJob(JobSystem::ReadFile(jobInput));
				triedJobs.insert(std::pair<std::string, bool> (*it, false));
			}
			else std::cout << "File read error" << std::endl;
			jobInput.close();
		}
	}

	// check untried jobs for dependency fulfillment
	while (triedJobs.size() != existingJobs.size()) {
		for (auto it = existingJobs.begin(); it != existingJobs.end(); ++it) {
			// check if job has been tried before
			if (triedJobs.find(*it) == triedJobs.end()) {
				bool dependenciesFulfilled = false;
				bool conditionFailed = false;
				auto depMapIt = dependencyMap[*it].begin();
				for (const auto& dep : dependencyMap[*it]) {
					if (triedJobs.find(dep.first) == triedJobs.end()) break; //if dependency hasn't been tried yet, break
					auto jobStatus = js->GetJobStatusByName(dep.first);

					// checks if dependencies are fulfilled
					// if conditional exists and conditional does not match callback, break
					if (!jobStatus.first || (!dep.second.empty() && (dep.second != jobStatus.second))) {
						conditionFailed = true;
						triedJobs.insert(std::pair<std::string, bool> (*it, false));
						std::cout << "Conditions failed for " << *it << std::endl;
						break;
					}
					if (++depMapIt == dependencyMap[*it].end()) {
						dependenciesFulfilled = true;
					}
				}

				if (dependenciesFulfilled && !conditionFailed) {
					std::ifstream jobInput("../Data/jobJSONs/" + *it + ".json");
					if (jobInput.good()) {
						js -> CreateAndQueueJob(JobSystem::ReadFile(jobInput));
						triedJobs.insert(std::pair<std::string, bool> (*it, true));
					}
					else std::cout << "File read error" << std::endl;
					jobInput.close();
				}
			}
		}
	}
}

void FlowScriptInterpreter::l_Lexer(std::ifstream &input) {
    int currentRow = 0;
    while(!input.eof() && !e_Flag) {
		std::string buffer;
        getline(input, buffer);
        std::cout << "Lexing line: " << buffer << std::endl;
		rawCodeText.push_back(buffer);
        lexResult.push_back(l_LexLine(buffer, currentRow));
        //std::cout << std::endl << std::endl;

        //std::cout << "Line read" << std::endl;

		currentRow++;
    }
}

std::vector<token> FlowScriptInterpreter::l_LexLine(const std::string& str, int currentRow) {
    std::vector<token> res;
    std::regex delimiter(R"(\s*(->|\w+\b|\W|\n)\s*)");

	int currentToken = 0;

    if (str.empty() || str.find_first_not_of(' ') == std::string::npos) {
        token temp(EMPTY);
        res.push_back(temp);
        return res;
    }

    for (std::sregex_iterator it(str.begin(), str.end(), delimiter), end; it != end; ++it) {
        std::string tokenStr = (*it)[1];
        //std::cout << "Matching token \"" << tokenStr << "\" -> ";
        tokenType type = l_GetTokenType(tokenStr);
        //std::cout << "<" << tokenTypeNames[type] << "> " << std::endl;

        if (type == INVALID) {
            std::cout << std::endl << std::endl;
            std::cout << "INVALID TOKEN \"" + tokenStr + "\"." << std::endl;
            std::cout << "LEXER ABORTED." << std::endl;
			token error = token(INVALID, (std::string &) tokenStr, currentRow, currentToken);
			e_HandleError(error, 1); // Lexical error
            return res;
        }

        //std::cout << "TOKEN \"" + tokenStr + "\" OF TYPE \"" << tokenTypeNames[type] << "\" READ." << std::endl;
        res.emplace_back(type, tokenStr, currentRow, currentToken);

		currentToken++;
    }

    return (res);
}

tokenType FlowScriptInterpreter::l_GetTokenType(std::string& str) {
    //std::cout << "Tested: ";

    for(const auto& temp : regexMap) {
        //std::cout << tokenTypeNames[temp.first] <<  " ";
        std::regex reg(temp.second);
        if (regex_match(str, reg)) return temp.first;
    }
    //std::cout << std::endl;

    return INVALID;
}

void FlowScriptInterpreter::e_HandleError(const token& token, int errorCode) {
	e_Flag = true;
	std::string path = "../Data/logs/" + graphName + "log.txt";

	std::cout << "ERROR CODE " << errorCode << std::endl;
	std::cout << "Error desc: " << errorMap[errorCode] << std::endl;
	std::cout << "Error occurred on line: " << token.lineIndex + 1 << ", at token: " << token.tokenIndex + 1 << std::endl;
	std::cout << "Token is of type: " << tokenTypeNames[token.type] << ", with value: " << token.value << std::endl;
	std::cout << "Printing line:" << std::endl;
	std::cout << rawCodeText[token.lineIndex] << std::endl;
	std::cout << "This error text can be found in " << path << std::endl;
	std::cout << "Check the documentation for further information about syntax and fixing errors." << std::endl;

	std::ofstream output(path);
	if (!output.is_open()) {
		std::cerr << "Failed to open the file for writing." << std::endl;
		return;  // or handle the error appropriately
	}
	output << "ERROR CODE " << errorCode << std::endl;
	output << "Error desc: " << errorMap[errorCode] << std::endl;
	output << "Error occurred on line: " << token.lineIndex + 1 << ", at token: " << token.tokenIndex + 1 << std::endl;
	output << "Token is of type: " << tokenTypeNames[token.type] << ", with value: " << token.value << std::endl;
	output << "Printing line where error occurs:" << std::endl << std::endl;
	output << rawCodeText[token.lineIndex] << std::endl << std::endl;
	output << "Check the documentation for further information about syntax and fixing errors." << std::endl;
	output.close();
}

void FlowScriptInterpreter::e_HandleError(int lineNum, int errorCode) {
	e_Flag = true;
	std::string path = "../Data/logs/" + graphName + "log.txt";

	std::cout << "ERROR CODE " << errorCode << std::endl;
	std::cout << "Error desc: " << errorMap[errorCode] << std::endl;
	std::cout << "Error occurred on line: " << lineNum + 1 << std::endl;
	std::cout << rawCodeText[lineNum] << std::endl;
	std::cout << "This error text can be found in ../Data/logs/log.txt" << std::endl;

	std::ofstream output(path);
	if (!output.is_open()) {
		std::cerr << "Failed to open the file for writing." << std::endl;
		return;  // or handle the error appropriately
	}
	output << "ERROR CODE " << errorCode << std::endl;
	output << "Error desc: " << errorMap[errorCode] << std::endl;
	output << "Error occurred on line: " << lineNum + 1 << std::endl;
	output << rawCodeText[lineNum] << std::endl;
	output << "Check the documentation for further information about the error code." << std::endl;
	output.close();
}

void FlowScriptInterpreter::printLexResult() {
    int lineNum = 1;
    for (auto line : lexResult) {
        std::cout << lineNum << ": ";
        for (auto token : line) {
            std::cout << token << " ";
        }
        std::cout << std::endl;
        lineNum++;
    }
}

void FlowScriptInterpreter::printMap() {
	for (const auto& entry : dependencyMap) {
		std::cout << entry.first << " -> ";

		const auto& vec = entry.second;
		for (auto it = vec.begin(); it != vec.end(); ++it) {
			std::cout << "(" << it->first << ", " << it->second << ")";
			if (std::next(it) != vec.end()) {
				std::cout << ", ";
			}
		}

		std::cout << std::endl;
	}
}

void FlowScriptInterpreter::i_ResetIterator() {
	i_braceFlag     = false;
	i_endFlag       = false;
	i_curLine       = 0;
	i_curToken      = 0;
}

token FlowScriptInterpreter::i_GetCurrent() {
	return lexResult[i_curLine][i_curToken];
}

bool FlowScriptInterpreter::i_Increment() {
	if (i_endFlag) return false;

	// Is current token last in row?
	if (i_curToken == lexResult[i_curLine].size() - 1) {
		// Is current row last in file?
		if (i_curLine == lexResult.size() - 1) {
			i_endFlag = true;
			return false;
		}
		i_curToken = 0;
		i_curLine++;
	} else {
		i_curToken++;
	}

	return true;
}

token FlowScriptInterpreter::i_GetNext() {
	i_Increment();
	return i_GetCurrent();
}

void FlowScriptInterpreter::s_ParseSyntax() {
    // reset flags and iterator ints
    i_ResetIterator();
	token curToken = i_GetCurrent();

	while(!i_endFlag) {
		std::vector<token> command;

		while (!binary_search(punctuation.begin(), punctuation.end(), curToken.type) && !i_endFlag) {
			//std::cout << curToken << std::endl;
			command.push_back(curToken);
			curToken = i_GetNext();
		}

		for(auto token : command) {
			std::cout << token << " ";
		}
		std::cout << std::endl;

		//std::cout << curToken << std::endl;
		token endSymbol = curToken;

		switch(endSymbol.type) {
			case LBRACE:
				if (i_braceFlag) {
					e_HandleError(curToken, 2); return;
				}
				i_braceFlag = true;
				s_ParseGraphDefinition(command, i_curLine);
				break;
			case RBRACE:
				if (!i_braceFlag) {
					e_HandleError(curToken, 3); return;
				}
				i_braceFlag = false;
				break;
			case SEMICOLON:
				if (!i_braceFlag) {
					e_HandleError(curToken, 4); return;
				}
				if (command.size() > 1) s_ParseNodeStatement(command);
				break;
			case EMPTY:
				break;
			default:
				//std::cout << "Impossible: " << curToken << " Line num: " <<  std::endl;
				e_HandleError(curToken, 22); return;
		}

		if(e_Flag) {
			//e_HandleError(i_curToken, i_curLine, 1);
			return;
		}

		curToken = i_GetNext();

		std::cout << std::endl;
	}

	if (i_braceFlag) {
		e_HandleError(curToken, 5); return;
	}
}

void FlowScriptInterpreter::s_ParseGraphDefinition(std::vector<token> command, int lineNum) {
	std::cout << "Handling graph definition" << std::endl;

	// Error guard measuring command size
	if (command.size() != 2) {
		e_HandleError(lineNum, 6); return; //
		return;
	}

	int curIndex = 0;
	token curToken = command[curIndex];

	if (curToken.type != TWORD) {
		e_HandleError(curToken, 7); // Graph definition error
		return;
	}

	if (!graphType.find(curToken.type)) {
		e_HandleError(curToken, 21); // Graph definition error
		return;
	}

	graphType = curToken.value; std::cout << "graph type = " << graphType << std::endl;

	curIndex++;
	curToken = command[curIndex];

	if (curToken.type != TWORD) {
		e_HandleError(curToken, 8); // Graph definition error
		return;
	}

	graphName = curToken.value; std::cout << "graph name = " << graphName << std::endl;
}

void FlowScriptInterpreter::s_ParseNodeStatement(std::vector<token> command) {
	int index = 0;
	auto commandIter = command.begin();

	std::cout << "Handling node statement" << std::endl;

	// Possible stopping point
	if ((*commandIter).type != TWORD) e_HandleError(*commandIter, 9);
	if (e_Flag) return;
	std::string startNode = (*commandIter).value;
	existingJobs.insert(startNode);
	commandIter++; index++;
	if (commandIter == command.end()) {
		dependencyMap[startNode]; // Add node to map
		std::cout << "Added " << startNode << std::endl;
		return;
	}

	if ((*commandIter).type != ARROW) e_HandleError(*commandIter, 10);
	if (e_Flag) return;
	commandIter++; index++;

	// Possible stopping point
	if ((*commandIter).type != TWORD) e_HandleError(*commandIter, 11);
	if (e_Flag) return;
	std::string endNode = (*commandIter).value;
	existingJobs.insert(endNode);
	commandIter++; index++;
	if (commandIter == command.end()) {
		std::pair<std::string, std::string> startNodePair{startNode, ""};
		dependencyMap[endNode].push_back(startNodePair); // Add node to map
		std::cout << "Added " << startNode << " -> " << endNode << std::endl;
		return;
	}

	if ((*commandIter).type != LBRACKET) e_HandleError(*commandIter, 12);
	if (e_Flag) return;
	commandIter++; index++;

	if ((*commandIter).type != TWORD) e_HandleError(*commandIter, 13);
	if ((*commandIter).value != "condition") e_HandleError(*commandIter, 14);
	if (e_Flag) return;
	commandIter++; index++;

	if ((*commandIter).type != EQUALS) e_HandleError(*commandIter, 15);
	if (e_Flag) return;
	commandIter++; index++;

	if ((*commandIter).type != QUOTE) e_HandleError(*commandIter, 16);
	if (e_Flag) return;
	commandIter++; index++;

	if ((*commandIter).type != TWORD) e_HandleError(*commandIter, 17);
	if (e_Flag) return;
	std::string condition = (*commandIter).value;
	commandIter++; index++;

	if ((*commandIter).type != QUOTE) e_HandleError(*commandIter, 18);
	if (e_Flag) return;
	commandIter++; index++;

	if ((*commandIter).type != RBRACKET) e_HandleError(*commandIter, 19);
	if (e_Flag) return;
	commandIter++; index++;

	std::pair<std::string, std::string> startNodePair{startNode, condition};
	dependencyMap[endNode].push_back(startNodePair); // Add node to map
	std::cout << "Added " << startNode << " -> " << endNode << " | Condition: " << condition << std::endl;

	if (commandIter != command.end()) e_HandleError(*commandIter, 20);
	if (e_Flag) return;
}









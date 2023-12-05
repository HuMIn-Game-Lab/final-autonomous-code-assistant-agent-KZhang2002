//
// Created by Kenny on 11/7/2023.
//

#ifndef LAB_3_FLOWSCRIPT_INTERPRETER_KZHANG2002_FLOWSCRIPTINTERPRETER_H
#define LAB_3_FLOWSCRIPT_INTERPRETER_KZHANG2002_FLOWSCRIPTINTERPRETER_H

#include "JobSystem.h"
#include <fstream>
#include <utility>
#include <regex>
#include <cctype>
#include <unordered_set>
#include <filesystem>

enum tokenType {
	SEMICOLON,
	TWORD,
	ARROW,
	LBRACE,
	RBRACE,
	LBRACKET,
	RBRACKET,
	EQUALS,
	QUOTE,
	INVALID,
	EMPTY
};

static const char* tokenTypeNames[] = {
	"SEMICOLON",
	"TWORD",
	"ARROW",
	"LBRACE",
	"RBRACE",
	"LBRACKET",
	"RBRACKET",
	"EQUALS",
	"QUOTE",
	"INVALID",
	"EMPTY"
};

static const std::vector<tokenType> punctuation = {
	SEMICOLON,
	LBRACE,
	RBRACE,
	EMPTY
};

static const std::map<tokenType, std::string> regexMap = {
	{SEMICOLON, R"(;)"},
	{ARROW,     R"(->)"},
	{LBRACE,    R"(\{)"},
	{RBRACE,    R"(\})"},
	{LBRACKET,  R"(\[)"},
	{RBRACKET,  R"(\])"},
	{EQUALS,    R"(=)"},
	{QUOTE,     R"(")"},
	{TWORD,     R"(\w+)"},
	{INVALID,   R"(.*)"}
};

static const std::vector<std::string> graphTypes = {
	"digraph",
	"graph"
};

static const std::unordered_map<int, std::string> eMessageMap = {
		{1,  R"(;)"},
		{2,  R"(;)"},
		{3,  R"(->)"},
		{4,  R"(\{)"},
		{5,  R"(\})"},
		{6,  R"(\[)"},
		{7,  R"(\])"},
		{8,  R"(=)"},
		{9,  R"(")"},
		{10, R"(\w+)"},
		{11, R"(.*)"}
};

static std::unordered_map<int, std::string> errorMap = {
		{1, "Invalid token processed. The only valid tokens are ' { ', ' } ', ' [ ', ' ] ', ' = ', ' “ ', and words. Only words can only contain characters a-z, A-Z, 0-9, and/or underscores."},
		{2, "Duplicate ‘{‘ used. Nested brackets are not allowed."},
		{3, "Right brace has no matching left brace."},
		{4, "Semicolons cannot be used outside of braces."},
		{5, "Left brace has no matching right brace."},
		{6, "Too many/too few arguments for graph definition."},
		{7, "Graph type is not a word."},
		{8, "Graph name is invalid. Names can only contain characters a-z, A-Z, 0-9, and/or underscores."},
		{9, "Node definition does not include a valid name for the source node."},
		{10, "Node definition does not include a valid arrow operator."},
		{11, "Node definition does not include a valid name for the end node."},
		{12, "Node definition has too many tokens."},
		{13, "Node attribute is not a word."},
		{14, "Node attribute is invalid."},
		{15, "Invalid attribute operator."},
		{16, "Invalid attribute assignment value. Value must be enclosed with quotation marks."},
		{17, "Attribute assignment value is not a word."},
		{18, "Attribute assignment value is missing end quotes. Value must be fully enclosed with quotation marks."},
		{19, "Bracket pair is not matched."},
		{20, "Node assignment has too many tokens."},
		{21, "Graph type does not exist."},
		{22, "Statement does not end properly. Use brackets, braces, or semicolons to end statements."}
};


struct token {
public:
	token() = default;
	token(tokenType type) : type(type){}
	token(tokenType type, std::string& value) : type(type), value(value){}
	token(tokenType type, std::string& value, int lineIndex, int tokenIndex)
		: type(type), value(value), lineIndex(lineIndex), tokenIndex(tokenIndex){}

	tokenType type = INVALID;
	std::string value;

	int lineIndex = -1;
	int tokenIndex = -1;

	friend std::ostream& operator<<(std::ostream& os, const token& token);
	bool operator==(tokenType other) const;
};

class FlowScriptInterpreter {
public:
	FlowScriptInterpreter() = default;
	FlowScriptInterpreter(const std::string& relativePath);
	FlowScriptInterpreter(std::ifstream &input);

	//main driver functions
	void Interpret(std::ifstream &input);
	bool Execute();
	void RunJobs();

	//l_Lexer functions, maybe move to private
	void l_Lexer(std::ifstream &input);
	std::vector<token> l_LexLine(const std::string& str, int currentRow);
	static tokenType l_GetTokenType(std::string &str);

	//Syntax parser, maybe move to private
	void s_ParseSyntax();
	void s_ParseGraphDefinition(std::vector<token> command, int lineNum);
	void s_ParseNodeStatement(std::vector<token> command);

	// printer functions
	void printLexResult();
	void printMap();
	bool isGood() { return e_Flag; }

	std::string graphName;
	std::string graphType;
	std::vector<std::string> rawCodeText;
	std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> dependencyMap;
	std::unordered_set<std::string> existingJobs;
	std::vector<std::vector<token>> lexResult;

private:
	//for error handling
	void e_HandleError(const token& token, int errorCode);
	void e_HandleError(int lineNum, int errorCode); // for lexical

	bool e_Flag      	= false;
	int e_Row       	= -1;
	int e_Token    		= -1;
	std::string e_Value;

	//for lex iterator
	void i_ResetIterator();
	bool i_Increment();
	token i_GetCurrent();
	token i_GetNext();

	bool i_braceFlag    = false;
	//bool i_bracketFlag  = false;
	//bool i_quoteFlag    = false;
	bool i_endFlag      = false;

	int  i_curLine      = 0;
	int  i_curToken     = 0;

	JobSystem* js = JobSystem::CreateOrGet();
};


#endif //LAB_3_FLOWSCRIPT_INTERPRETER_KZHANG2002_FLOWSCRIPTINTERPRETER_H

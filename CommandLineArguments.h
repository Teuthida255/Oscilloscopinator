#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <vector>

//class for command line options, with optional arguments
class Option {
public:
	Option(char s, std::string l, bool a, std::string odescription, std::string adescription = "No arguments.", std::string d = "");

	void describeOption();

	bool testString(std::string compare);
	bool getOptFlag();
	bool hasArg();

	void setArgument(std::string a);
	std::string getArgument();
	bool argIsDefined();

private:
	std::string shortopt;
	std::string longopt;

	std::string optdescription;
	std::string argdescription;

	std::string arg;


	bool optflag;
	bool hasarg;
	bool argflag;
};


//handles command line arguments
void handleOptions(int argc, char* argv[], std::vector<Option*> opttable);
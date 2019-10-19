#include "CommandLineArguments.h"

void handleOptions(int argc, char* argv[], std::vector<Option*> opttable) {

	if (argc <= 1) {
		std::cout << "No arguments found!\n";
	}
	else {
		bool errflag = true;
		//stores argument that couldn't be parsed
		std::vector <std::string> errtable;
		//std::cout << argc << '\n';

		//check each argument sequentially
		for (int i = 1; i < argc; i++) {
			//std::cout << i << " in main loop\n";
			errflag = true;
			//compare each argument with the list of options
			for (auto j : opttable) {
				//make sure the option hasn't already been set
				if (!j->getOptFlag()) {
					if (j->testString(argv[i])) {
						errflag = false;
						if (j->hasArg()) {
							i++;
							//std::cout << i << " in secondary loop\n";

							//checks for arguments if the option has them
							if (i < argc) {
								//if the argument isn't an option
								if (argv[i][0] != '-') {
									j->setArgument(argv[i]);
								}
								else {
									i--;
									break;
								}
							}
							else {
								break;
							}
						}
						break;
					}
				}
			}
			if (errflag) {
				errtable.push_back(argv[i]);
			}
		}
		if (errtable.size() != 0) {
			std::cout << "The following arguments could not be parsed:\n";
			for (auto i : errtable) {
				std::cout << i << "\n";
			}
		}
	}
}

//loads names for options
//'s' is the short option name
//'l' is the long option name
//the strings will be formatted automatically if no -- is included
//set 'a' to true to include an argument with the option
//pass a value to d to give the argument a default value
Option::Option(char s, std::string l, bool a, std::string odescription, std::string adescription, std::string d) {
	std::stringstream ss;
	ss << '-' << s;
	ss >> shortopt;

	ss.clear();
	if (l.at(1) != '-') {
		if (l.at(0) != '-') {
			ss << "-";
		}
		ss << "-" << l;
		ss >> longopt;
	}


	optdescription = odescription;
	argdescription = adescription;

	arg = d;

	hasarg = a;
	optflag = false;
}

//gives a short description of what the option does
void Option::describeOption() {
	std::cout << shortopt << ", " << longopt << '\n';
	std::cout << optdescription << '\n';
	if (argdescription != "") {
		std::cout << argdescription << "\n";
	}
	if (argIsDefined()) {
		std::cout << "(Default argument: " << arg << ")\n";
	}
	std::cout << '\n';
}

//checks string for compatibility with command line argument
bool Option::testString(std::string compare) {
	for (auto i = compare.begin(); i != compare.end(); i++) {
		*i = tolower(*i);
	}

	optflag = (shortopt == compare || longopt == compare);
	if (optflag) {
		std::cout << "Match found!\n";
	}
	else {
		std::cout << "Match not found!\n";
	}
	return optflag;
}

bool Option::getOptFlag() {
	return optflag;
}

bool Option::hasArg() {
	return hasarg;
}

void Option::setArgument(std::string a) {
	arg = a;
	argflag = true;
}

std::string Option::getArgument() {
	return arg;
}

bool Option::argIsDefined() {
	return (arg != "");
}
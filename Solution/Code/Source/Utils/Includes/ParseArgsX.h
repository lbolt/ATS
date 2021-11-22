#pragma once

#include "../Includes/CommonX.h"
#include "../Includes/StringX.h"




// --------------------------------
// CLASS DECLARATIONS
// --------------------------------
class CommandLineArgs 
{
public:
	//
	CommandLineArgs() { parseOK = false; };
	CommandLineArgs(int, wchar_t*[]);
	~CommandLineArgs() {};

	bool FillHash(int, wchar_t*[]);
	//
	WStringHash args;
	int num;
private:
	//
	bool parseOK;
	string options;

};

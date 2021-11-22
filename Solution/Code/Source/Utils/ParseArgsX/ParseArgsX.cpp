#include "stdafx.h"
#include "../Includes/ParseArgsX.h"



CommandLineArgs::CommandLineArgs(int argc, wchar_t* argv[])
{
	if (FillHash(argc, argv))
		parseOK = true;
}


bool CommandLineArgs::FillHash(int argc, wchar_t* argv[])
{

	wstring key, value;
	//wchar_t single[256];
	__int64 singleCount = 0;
	// parse out the ARGV array
	args.clear();
	// Arg[0] is the command - the key part will be specified as "CMD"
	// Arg[1..n] single char switch | single char switch with value string
	wchar_t** pArg = argv;
	// Process CMD part
	key.assign(L"CMD");
	value.assign((wchar_t*)&pArg[0][0]);
	args.insert(WStringHash::value_type(key, value));
	// Process remaining args
	for (++pArg; *pArg != NULL; pArg++) {
		key.erase(); value.erase();
		if (pArg[0][0] == L'-' || pArg[0][0] == L'/') {
			// its a switch
			key.assign((wchar_t*)&pArg[0][1]);
		}
		if ((pArg + 1)[0] == NULL)
			goto UpdateHash;
		// Check for a value part
		if ((pArg + 1)[0][0] != L'-' && (pArg + 1)[0][0] != L'/') {
			value.assign((wchar_t*)&(pArg + 1)[0][0]);
			++pArg;
		}
		// update hash
	UpdateHash:
		{
			args.insert(WStringHash::value_type(key, value));
		}
	}

	return S_OK;
}

#pragma once

#include "Windows.h"
#include "ATS_Defines.h"
#include "ATS_CODES.h"
#include <ctime>
#include <ratio>
#include <chrono>


using namespace ATS_Defines;
using namespace ATS_CODES;



#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4430)		// Compiler bug


#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

//typedef int(__cdecl *LOADPROC)(LPWSTR);
typedef int(__cdecl *LOADPROC)(void*);
typedef unsigned int(__cdecl *LOADTHREAD)(void*);
//typedef UINT(CALLBACK* LPFNDLLFUNC1)(DWORD, UINT);


class CommonX
{
public:
	CommonX();
	~CommonX();
};


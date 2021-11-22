#pragma once
//#include <stdio.h>
//#include <stddef.h>
#include <wchar.h>
#include "ATS_CONST_CODES.h"


namespace ATS_CODES
{
	using namespace ATS_CONST_CODES;

	// ATS return codes
	static const int ATS_OK					= 0;
	static const int ATS_FALSE				= 1;
	static const int ATS_ERROR				= -1;
	static const int ATS_C_SUCCESS			= 1000;
	static const int ATS_C_FAIL				= 1001;
	static const int ATS_C_UNKNOWN			= 1002;
	static const int ATS_C_NONEXIST			= 1003;
	static const int ATS_C_EMPTY			= 1004;
	static const int ATS_C_RANGE			= 1005;
	static const int ATS_C_NOMEM			= 1006;
	static const int ATS_C_FOUND			= 1007;
	static const int ATS_C_BOUNDS			= 1008;

	static const int ATS_C_AE_NOTREND		= 1020;
	static const int ATS_C_AE_TRENDING		= 1021;
	static const int ATS_C_AE_TRENDCHANGE	= 1022;
	static const int ATS_C_AE_TRENDLIMITOUT	= 1023;
	static const int ATS_C_AE_TRENDERROR	= 1024;
	static const int ATS_C_AE_DATA_COMPRESS	= 1025;
	static const int ATS_C_AE_DATA_GAP		= 1026;
	static const int ATS_C_AE_INCOMPLETE	= 1027;


	// ATS return code to descriptor mappings
	struct ATS_PROGRAM_CODES {
		int code;
		const wchar_t* desc;
	};
	static ATS_PROGRAM_CODES return_codes[] = {
		/*1000*/ {ATS_C_SUCCESS,	L"ATS_C_SUCCESS"},
		/*1001*/ {ATS_C_FAIL,		L"ATS_C_FAIL"},
		/*1002*/ {ATS_C_NONEXIST,	L"ATS_C_NONEXIST"},
		/*1003*/ {ATS_C_EMPTY,		L"ATS_C_EMPTY"},
		/*1004*/ {ATS_C_RANGE,		L"ATS_C_RANGE"},
		/*1005*/ {ATS_C_NOMEM,		L"ATS_C_NOMEM"},
		/*1006*/ {ATS_C_FOUND,		L"ATS_C_FOUND"},
		/*1007*/ {ATS_C_UNKNOWN,	L"ATS_C_UNKNOWN"}
	};

	typedef int ATS_CODE;



} // namespace ATS_CODES









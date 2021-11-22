#pragma once

#include "../Utils/Includes/CommonX.h"
#include "../Utils/Includes/PropsX.h"

#include "ATS_CODES.h"
#include "../AT/Shared/ATServerAPIDefines.h"

#include "../AT/ActiveTickServerAPI/ActiveTickServerAPI.h"
#include "../AT/ActiveTickServerAPI/ActiveTickStreamListener.h"
#include "../AT/ActiveTickServerAPI/ActiveTickServerRequestor.h"
#include "DataSecurityStructs.h"

#include <string>
#include <vector>

#include "time.h"


using namespace ATS_CODES;
using namespace std;

class AT_Helper
{
public:
	AT_Helper();
	~AT_Helper();

	///////////////////////////////////
	// String conversion
	///////////////////////////////////
	static ATGUID StringToATGuid(const std::string& guid);
	static ATS_CODE ConvertWStrtoCStr(const wstring&, string&);
	static ATS_CODE ConvertWStrtoWChar(const wstring&, wchar_t&, int);
	static ATS_CODE CopyWChartoWCharPadded(const wchar_t&, int, wchar_t&, int);

	static wstring s2ws(const std::string& str);
	static string ws2s(const std::wstring& wstr);

	static bool ConvertString(const char* src, wchar16_t* dest, uint32_t destcount);
	static wstring AT_SymbolToWString(wchar16_t* src);
	static ATSYMBOL StringToATSymbol(const std::string& symbol);
	static vector<ATSYMBOL> StringToATSymbols(const std::string& symbols);

	static ATS_CODE ConvertATTimeToATSTimeWString(ATTIME&, ATS_TIME_POINT&);
	static ATS_CODE ConvertATSTimeWStringToATTime(ATS_TIME_POINT&, ATTIME&);

	static ATS_CODE ConvertATTimeToMS(const ATTIME&, int&);
	static ATS_CODE ConvertATSTimeToMS(const SYSTEMTIME&, int&);

private:
	// Declare private methods and objects
	AT_Helper(const AT_Helper &);
	AT_Helper& operator=(const AT_Helper&);


};


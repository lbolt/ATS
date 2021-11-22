#include "stdafx.h"
#include "AT_Helper.h"


AT_Helper::AT_Helper()
{
}


AT_Helper::~AT_Helper()
{
}

//
//	Convert String to GUID
//
/*static*/ ATGUID AT_Helper::StringToATGuid(const std::string& guid)
{
	ATGUID atGuid = { 0 };

	if (guid.length() == 32)
	{
		for (uint32_t i = 0; i < 16; i += 2)
		{
			uint32_t n = strtoul(guid.substr(i, 2).c_str(), NULL, 16);

			if (i < 8)
				((uint8_t*)&atGuid)[3 - (i / 2)] = n;
			else if (i < 12)
				((uint8_t*)&atGuid)[9 - (i / 2)] = n;
			else if (i < 16)
				((uint8_t*)&atGuid)[13 - (i / 2)] = n;
		}

		for (uint32_t i = 16; i < 32; i += 2)
		{
			uint32_t n = strtoul(guid.substr(i, 2).c_str(), NULL, 16);
			((uint8_t*)&atGuid)[i / 2] = n;
		}
	}

	return atGuid;
}

//
//	Converts Wide String to C String
//
/*static*/ ATS_CODE AT_Helper::ConvertWStrtoCStr(const wstring &src, string &dst)
{


	return ATS_C_SUCCESS;
}

/*static*/ ATS_CODE AT_Helper::ConvertWStrtoWChar(const wstring& src, wchar_t& dst, int dstSize)
{
	int len = dstSize;
	int trimCount = 0;

	if (dstSize > src.length())
	{
		trimCount = dstSize - (int)src.length();
	}
	wmemcpy( (wchar_t*)&dst, src.data(), len);
	if (trimCount > 0)
	{
		wmemset((wchar_t*)(&dst + len), 0, trimCount);
	}

	return ATS_C_SUCCESS;
}

/*static*/ ATS_CODE AT_Helper::CopyWChartoWCharPadded(const wchar_t& src, int srcSize, wchar_t& dst, int dstSize)
{
	int len = dstSize;
	int trimCount = 0;

	if (dstSize > srcSize)
	{
		trimCount = dstSize - srcSize;
	}
	else
	{
		len = srcSize - 1;
	}
	wmemcpy((wchar_t*)&dst, (wchar_t*)&src, len);
	if (trimCount > 0)
	{
		wmemset((wchar_t*)(&dst + srcSize), 0, trimCount);
	}

	return ATS_C_SUCCESS;
}


//
//	String to Wide String
//
/*static*/ wstring AT_Helper::s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
};

//
//	Wide String to String
//
/*static*/ string AT_Helper::ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
};


//
//	String of Symbols to Vector
//
/*static*/ vector<ATSYMBOL> AT_Helper::StringToATSymbols(const std::string& symbols)
{
	//symbol1,symbol2,...

	std::vector<ATSYMBOL> out;
	std::size_t pos = 0;
	std::size_t prevpos = 0;

	while ((pos = symbols.find(",", pos)) != std::string::npos)
	{
		std::string symbol = symbols.substr(prevpos, pos - prevpos);
		out.push_back(AT_Helper::StringToATSymbol(symbol));

		++pos;
		prevpos = pos;
	}

	out.push_back(AT_Helper::StringToATSymbol(symbols.substr(prevpos)));

	return out;
}





/*static*/ ATS_CODE AT_Helper::ConvertATTimeToATSTimeWString(ATTIME &, ATS_TIME_POINT &)
{
	return ATS_CODE();
}

/*static*/ ATS_CODE AT_Helper::ConvertATSTimeWStringToATTime(ATS_TIME_POINT&, ATTIME&)
{
	return ATS_CODE();
}


/*static*/ ATSYMBOL AT_Helper::StringToATSymbol(const std::string& symbol)
{
	ATSYMBOL atSymbol;
	memset(&atSymbol, 0, sizeof(atSymbol));

	if (symbol.empty() == true)
		return atSymbol;

	//SYMBOL.EXCHANGE@COUNTRY -> CSCO.Q@U, CSCO.Q, CSCO, CSCO@U; .ABCDE.Q@U, .ABCDE.Q, .ABCDE;

	if (symbol.empty() == false)
	{
		switch (symbol.at(0))
		{
		case '.':	atSymbol.symbolType = SymbolStockOption; break;
		case '$':	atSymbol.symbolType = SymbolIndex; break;
		case '^':	atSymbol.symbolType = SymbolMutualFund; break;
		case '#':	atSymbol.symbolType = SymbolCurrency; break;
		case '&':	atSymbol.symbolType = SymbolBond; break;
		default:	atSymbol.symbolType = SymbolStock; break;
		}

		if (atSymbol.symbolType == SymbolCurrency)
		{
			atSymbol.countryType = CountryInternational;
			atSymbol.exchangeType = ExchangeForex;
		}
		else
		{
			atSymbol.countryType = CountryUnitedStates;
			atSymbol.exchangeType = ExchangeComposite;
		}
	}

	//find .EXCHANGE
	std::size_t exchangeIndex = symbol.find('.', 1);
	if (exchangeIndex != std::string::npos && symbol.length() > exchangeIndex + 1)
		atSymbol.exchangeType = (uint8_t)symbol.at(exchangeIndex + 1);

	//find @COUNTRY
	std::size_t countryIndex = symbol.find('@', 1);
	if (countryIndex != std::string::npos && symbol.length() > countryIndex + 1)
		atSymbol.countryType = (uint8_t)symbol.at(countryIndex + 1);

	std::size_t symbolBodyLen = 0;

	if (exchangeIndex != std::string::npos)
		symbolBodyLen = exchangeIndex;
	else if (countryIndex != std::string::npos)
		symbolBodyLen = countryIndex;
	else
		symbolBodyLen = symbol.length();
	if (symbolBodyLen > 0)
	{
#pragma warning(disable:4267)
		if (atSymbol.symbolType == SymbolStock)
			AT_Helper::ConvertString(symbol.c_str(), atSymbol.symbol, symbolBodyLen);
		else
			AT_Helper::ConvertString(symbol.c_str() + 1, atSymbol.symbol, symbolBodyLen - 1);
#pragma warning(default:4267)
	}
	return atSymbol;
}


///
//	Convert AT Symbol to Wide String
//
/*static*/ wstring AT_Helper::AT_SymbolToWString(wchar16_t* src)
{
	wstring dst;
	dst = src;
	return dst;
}


/*static*/ bool AT_Helper::ConvertString(const char* src, wchar16_t* dest, uint32_t destcount)
{
	bool rc = true;
	uint32_t i = 0;

	while (src[i])
	{
		if (i >= destcount)
		{
			rc = false;
			break;
		}

		dest[i] = (wchar16_t)src[i];
		++i;
	}

	if (rc == true && i < destcount)
		dest[i] = 0;
	else
		rc = false;

	return rc;
}


/*static*/ ATS_CODE AT_Helper::ConvertATTimeToMS(const ATTIME& ATTimePoint, int& msTimePoint)
{
	int	hh_to_ms = (int)ATTimePoint.hour * 60 * 60 * 1000;
	int mm_to_ms = (int)ATTimePoint.minute * 60 * 1000;
	int ss_to_ms = (int)ATTimePoint.second * 1000;
	msTimePoint = hh_to_ms + mm_to_ms + ss_to_ms + (int)ATTimePoint.milliseconds;
	return ATS_C_SUCCESS;
}


/*static*/ ATS_CODE AT_Helper::ConvertATSTimeToMS(const SYSTEMTIME& ATSTimePoint, int& msTimePoint)
{
	int	hh_to_ms = (int)ATSTimePoint.wHour * 60 * 60 * 1000;
	int mm_to_ms = (int)ATSTimePoint.wMinute * 60 * 1000;
	int ss_to_ms = (int)ATSTimePoint.wSecond * 1000;
	msTimePoint = hh_to_ms + mm_to_ms + ss_to_ms + (int)ATSTimePoint.wMilliseconds;
	return ATS_C_SUCCESS;
}





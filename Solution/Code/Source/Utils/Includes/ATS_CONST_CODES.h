#pragma once

///////////////////////////////////////////////////////////////////////////////
//	Data Dictionary
//	- Static constant codes apply to both program and database symbols
//	
///////////////////////////////////////////////////////////////////////////////

namespace ATS_CONST_CODES
{

	//	Program Codes
	//	- All providers will map into these ATS max constants
	//	Note: Database symbol sizes will also leverage these values
	//
	static const int Stock_SymbolMaxLength = 30;
	static const int Option_SymbolMaxLength = 8;
	static const int Index_SymbolMaxLength = 16;
	static const int Commodity_SymbolMaxLength = 16;
	static const int ForEx_SymbolMaxLength = 16;
	static const int Exchange_SymbolMaxLength = 16;
	static const int Short_Symbol_Desc_Length = 40;
	static const int Long_Symbol_Desc_Length = 80;


	//	Data Dictionary
	//
	//	- Watch Pool Table
	//	-	[symbol]			[nchar]		(Stock_SymbolMaxLength)		NOT NULL
	//	-	[avg_vol]			[int]									NOT NULL
	//	-	[watch_eligible]	[bit]									NOT NULL
	//	-	[trade_eligible]	[bit]									NOT NULL
	//	-	[create_dt]			[datetime2]								NOT NULL
	//	-	[update_dt]			[datetime2]								NOT NULL
	//	-	[exchange]			[nchar]		(Exchange_SymbolMaxLength)	NULL
	//	-	[desc]				[nchar]		(Short_Symbol_Desc_Length)	NULL



} // ATS_CONST_CODES


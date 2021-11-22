///////////////////////////////////////////////////////////////////////////////
//	Holds all Connector Trade Security Structures
//
//	This is meant to support multiple providers in the future.
//
//	- Active Tick
//	- Polygon.io (future)
//	- Lightspeed (future)
///////////////////////////////////////////////////////////////////////////////


#pragma once
#include "CommonX.h"
#include <string>
#include <vector>
#include "CircularBuffer.h"
#include "../AT/Shared/ATServerAPIDefines.h"

using namespace ATS_DS;


//	Trade and Entry Time
typedef struct _ATS_TIME_POINT
{
	std::chrono::time_point< std::chrono::system_clock> time_point;
} ATS_TIME_POINT;


typedef struct _ATS_TRADE_TIME
{
	ATTIME trade_time;
} ATS_TRADE_TIME;


//	Container for conversion from "provider" trade object to ATS trade object
//	Note: Purpose is to capture tick data
typedef struct _ATS_TRADE_DATA {
	double				trade_amt;
	unsigned int		trade_vol;
	int					db_sequence;
	SYSTEMTIME			time_added_to_buffer;
	ATS_TRADE_TIME		time_of_trade;
	UINT8				precision;	// Review AT Money representation
	wchar_t				symbol[ATS_StockSymbolMaxLength];
	wchar_t				lastExchange[ATS_lastExchangeLen];
} TRADE_DATA, TICK_DATA, TRADE;

#ifdef _UNUSED
typedef struct _ATS_TICK_DATA {
	double				trade_amt;
	int					db_sequence;
	unsigned int		trade_vol;
	ATS_TIME_POINT		time_added_to_buffer;
	ATS_TRADE_TIME		time_of_trade;
	UINT8				precision;	// Review AT Money representation
	std::wstring		symbol;
	std::wstring		exchange;
} TICK_DATA;
#endif _UNUSED

typedef struct _ATS_SUPER_TICK_DATA {
	double				trade_amt;
	double				last_trade_amt;
	unsigned int		trade_vol;
	SYSTEMTIME			time_added_to_buffer;
	ATS_TRADE_TIME		time_of_trade;
	UINT8				precision;	// Review AT Money representation
	wchar_t				symbol[ATS_StockSymbolMaxLength];
	wchar_t				exchangeID[ATS_lastExchangeLen];
} S_TICK_DATA;

#ifdef _UNUSED_CODE_
//	Trade event wrapper
typedef struct _TRADE_EVENT
{
	//TRADE_EVENT_TYPE type;
	CIRCULAR_BUFFER_STATES state;
	//TRADE_DATA trade_data;
} TRADE_EVENT;
#endif _UNUSED_CODE_






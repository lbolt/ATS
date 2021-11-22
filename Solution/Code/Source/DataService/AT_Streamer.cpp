#include "stdafx.h"
#include "AT_Streamer.h"
#include "DataConnector.h"
#include "DataCollectorWorker.h"

//#include <chrono>
#include <ctime>


using namespace std::chrono;



///////////////////////////////////////////////////////////////////////////////
//	Data Connector Quote Streamer Constructor / Destructor
///////////////////////////////////////////////////////////////////////////////
AT_Streamer::AT_Streamer(void* Args, uint64_t& hSession)
	: m_hSession(hSession), ActiveTickServerRequestor(hSession), ActiveTickStreamListener(hSession, false)
{
	ATS_CODE rc;
	rc = initialize(Args);

}


AT_Streamer::~AT_Streamer()
{
}


///////////////////////////////////////////////////////////////////////////////
//	Data Connector Definitions
///////////////////////////////////////////////////////////////////////////////
ATS_CODE AT_Streamer::initialize(void* Args)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		pGlobals = (DS_Globals*)Args;
		pArgs = pGlobals;
		pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
		pDataLogger = (DataLoggerX*)pGlobals->Args[ARG_SNAP]; 
		pDS_Shared = (DS_Shared*)pGlobals->Args[ARG_SDS];

		///////////////////////////////////////////////////////////////////////
		//	Setup streamer
		///////////////////////////////////////////////////////////////////////
		//initConnCtxBlock();
		AutoCriticalSectionLock AT_Streamer_sso(streamer_sso);

	}
	catch (...)
	{
		//setATProvConnected(false);
		//pLogger->log_info(L"Failed to initialize DataConnector", L"DataConnector::initialize");
	}

	return rc;
}

ATS_CODE AT_Streamer::unpackTradeUpdate(LPATQUOTESTREAM_TRADE_UPDATE tradeUpdate, TRADE& trade)
{
	ATS_CODE rc;
	SYSTEMTIME systemTime;

	memset(&trade, 0, ATS_StockSymbolMaxLength * sizeof(wchar_t));
	AT_Helper::CopyWChartoWCharPadded((wchar_t&)tradeUpdate->symbol, ATSymbolMaxLength / sizeof(wchar_t), (wchar_t&)trade.symbol, ATS_StockSymbolMaxLength);
	memset(&trade, 0, ATS_lastExchangeLen * sizeof(wchar_t));
	AT_Helper::CopyWChartoWCharPadded((wchar_t&)tradeUpdate->lastExchange, sizeof(tradeUpdate->lastExchange) / sizeof(wchar_t), (wchar_t&)trade.lastExchange, ATS_lastExchangeLen);
	trade.trade_vol = tradeUpdate->lastSize;
	trade.trade_amt = tradeUpdate->lastPrice.price;
	trade.precision = tradeUpdate->lastPrice.precision;

	trade.time_of_trade.trade_time = tradeUpdate->lastDateTime;
	GetLocalTime(&systemTime);

	int aaa = sizeof(SYSTEMTIME);
	memcpy(&trade.time_added_to_buffer, &systemTime, sizeof(SYSTEMTIME));

	// Lookup db sequence for symbol
	std::map<std::wstring, unsigned int>::iterator it;
	it = pDS_Shared->pSequenceLookupMap->symToseqNumLookupHash.find(trade.symbol);
	if (it == pDS_Shared->pSequenceLookupMap->symToseqNumLookupHash.end())
	{
		trade.db_sequence = 0;
		rc = ATS_C_NONEXIST;
	}
	else
	{
		trade.db_sequence = it->second;
		rc = ATS_C_SUCCESS;
	}
	return rc;
}


ATS_CODE AT_Streamer::convertTradeMsgToWString(TRADE& trade, wchar_t* tradeMsg, int msgSize)
{
	// Convert trade record to wide string for snapshot
	ATS_CODE rc= ATS_C_SUCCESS;

	swprintf(tradeMsg, msgSize, L"%6s\t %4.4f\t %5u\t %1u", trade.symbol, trade.trade_amt, trade.trade_vol, trade.precision);

	return rc;
}


///////////////////////////////////////////////////////////////////////////////
//	AT Streaming Trade Callback Routine
//	- This update function is called for all subscribed equities
//	- Operates in the context of a single thread - does not get interrupted
//	  by other threads
///////////////////////////////////////////////////////////////////////////////
/*virtual*/ void AT_Streamer::OnATStreamTradeUpdate(LPATQUOTESTREAM_TRADE_UPDATE pUpdate)
{
	ATS_CODE ats_rc;
	DS_Globals* pGlobalArgs;
	DataConnector* pDataConn;
	DataCollectorWorker* pDataCollectorWorker;
	DS_Active_CTX* pDSACtx;
	DS_Context* pDSCtx;
	TRADE trade;
	int sleepTimeMS;

	pGlobalArgs = (DS_Globals*)pArgs;
	pDataConn = (DataConnector*)pGlobalArgs->Args[ARG_CONN];
	pDataCollectorWorker = (DataCollectorWorker*)pGlobalArgs->Args[ARG_COLL];
	sleepTimeMS = pDataConn->connCtx.callbackCycleDelay;

	try
	{
		wchar_t logMsg[LOGGER_MSG_MEDIUMBUF];

		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
			pLogger->log_func_entry(L".........Entered Stream Trade Update............", L"AT_Streamer::OnATStreamTradeUpdate");

		ats_rc = unpackTradeUpdate(pUpdate, trade);		//	_ATS_TRADE_DATA

			//	Used to track time-of-trade drift for provider (TESTING ONLY)
			//	- A "symbol" for tracking must be set in configuration file under this provider
			//	- The "symbol" must also be set as watch eligible in the watch pool table
			//	- Statics logging must be enabled
			//	- Stats are dumped to the Data Collector log file on each trade instance
			//if (trade.symbol == pDataConn->connCtx.trackStatsForSymbol)
		if (wcscmp(trade.symbol, pDataConn->connCtx.trackStatsForSymbol) == 0)
		{
			//	Trade received and processed time
			wchar_t AddedtoBuf[16];
			wsprintf(AddedtoBuf, L"%2d:%2d:%2d.%3d", trade.time_added_to_buffer.wHour, trade.time_added_to_buffer.wMinute, trade.time_added_to_buffer.wSecond, trade.time_added_to_buffer.wMilliseconds);
			int ATS_msTime;
			AT_Helper::ConvertATSTimeToMS((SYSTEMTIME)trade.time_added_to_buffer, ATS_msTime);
			//	Trade Time
			wchar_t TimeofTrade[16];
			wsprintf(TimeofTrade, L"%2d:%2d:%2d.%3d",
				trade.time_of_trade.trade_time.hour, trade.time_of_trade.trade_time.minute, trade.time_of_trade.trade_time.second, trade.time_of_trade.trade_time.milliseconds);
			int AT_msTime;
			AT_Helper::ConvertATTimeToMS((ATTIME)trade.time_of_trade.trade_time, AT_msTime);

			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_STATS))
			{
				swprintf(logMsg, sizeof logMsg, L"Added Trade [#%8u] to RDQ: %6s\t %4.4f\t %5u\t x%1u ToB:%s\t ToT:%s\t %d(ms)",
					pDS_Shared->insertionCountRDQ,
					trade.symbol,
					trade.trade_amt,
					trade.trade_vol,
					trade.precision,
					AddedtoBuf,
					TimeofTrade,
					ATS_msTime - AT_msTime
				);
				pLogger->log_stats(logMsg, L"AT_Streamer::OnATStreamTradeUpdate");
			}
		}


		//	Add trade event to the RDQ
		if (trade.db_sequence != 0)
		{
			//	Added for direct insertion to <T> = _ATS_TRADE_DATA
			pDSACtx = pDS_Shared->pActiveCTXBlock->active_CTX_Container[trade.db_sequence];
			pDSCtx = pDSACtx->pCtx;

			//	Insert trade record into DS_ACTIVE_CTX Circular Buffer
			//	- Disabling RDQ insertions is used to test provider trade message time drift
			//	-	Make certain to enable STATS logging when disablement is set "true"
			if (pDataConn->connCtx.disableRDQInsertion == false)
			{
				pDSACtx->DS_Active_sso.lock();
				pDSCtx->tickBuf.insert_back((TICK_DATA*)&trade);
				pDS_Shared->insertionCountRDQ++;	//	Keep track of count only
				pDSACtx->DS_Active_sso.release();

				// Send event notification to Analytics Engine
				ats_rc = pDataCollectorWorker->processAnalyticsNotification(trade.db_sequence);

				//	Send a notification event to the Analytics Engine  <-----------------needed?
				SetEvent(pDS_Shared->hPostAnalyticsNRQEvent);

				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
				{
					swprintf(logMsg, sizeof logMsg, L"Inserted trade record into Active Context TICK buffer [symbol %6s, db_seq %d, cb_active %d]",
						trade.symbol, trade.db_sequence, pDSCtx->tickBuf.num_active_elements());
					pLogger->log_debug(logMsg, L"AT_Streamer::OnATStreamTradeUpdate");
				}
			}
			else
			{
				pDS_Shared->insertionCountRDQ++;	//	Keep track of count only
			}


			//	Statistics
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_STATS))
			{
				if ((int)pDS_Shared->insertionCountRDQ % 5000 == 0)
				{
					int thisCycleCount = pDS_Shared->insertionCountRDQ - pDS_Shared->lastRDQCycleCount;
					pDS_Shared->lastRDQCycleCount = pDS_Shared->insertionCountRDQ;
					//
					chrono::duration<double> timeDiff = std::chrono::system_clock::now() - pDS_Shared->lastRDQCycleTime;
					pDS_Shared->lastRDQCycleTime = std::chrono::system_clock::now();
					//
					swprintf(logMsg, sizeof logMsg, L"Current trade count: #%8u\t CB-Size: %d\t Active-Elements: %u\t TPS: %4.2fs/%4.2fm",
						pDS_Shared->insertionCountRDQ,
						pDS_Shared->pRDQ->size_of_container(),
						pDS_Shared->pRDQ->num_active_elements(),
						(double)thisCycleCount / timeDiff.count(),
						((double)thisCycleCount / timeDiff.count())*60.0
					);
					pLogger->log_stats(logMsg, L"AT_Streamer::OnATStreamTradeUpdate");
				}
			}

			//	Output this trade record to SNAPSHOT file
			if (pDataLogger->isEnabled())
			{
				if (pDataLogger->getLoggerLevel() == DS_DATALOGGER_LEVEL::SNAP_OUTPUT)
				{
					wchar_t tradeMsg[DATALOGGER_MSG_MEDIUMBUF];
					// Convert Trade Message to wide string
					convertTradeMsgToWString(trade, tradeMsg, sizeof (tradeMsg)/2);
					pDataLogger->output(tradeMsg);
				}
			}

			// Dump this trade record to log file (TESTING ONLY)
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
			{
				swprintf(logMsg, sizeof logMsg, L"Added Trade [#%8u] to RDQ: %6s\t %4.4f\t %5u\t x%1u %d:%u",
													pDS_Shared->insertionCountRDQ, 
													trade.symbol, 
													trade.trade_amt, 
													trade.trade_vol, 
													trade.precision, 
													pDS_Shared->pRDQ->size_of_container(), 
													pDS_Shared->pRDQ->num_active_elements()
				);
				pLogger->log_debug(logMsg, L"AT_Streamer::OnATStreamTradeUpdate");
			}
#ifdef _UNUSED
			//	Used to track time-of-trade drift for provider (TESTING ONLY)
			//	- A "symbol" for tracking must be set in configuration file under this provider
			//	- The "symbol" must also be set as watch eligible in the watch pool table
			//	- Statics logging must be enabled
			//	- Stats are dumped to the Data Collector log file on each trade instance
			//if (trade.symbol == pDataConn->connCtx.trackStatsForSymbol)
			if(wcscmp(trade.symbol, pDataConn->connCtx.trackStatsForSymbol) == 0)
			{
				//	Trade received and processed time
				wchar_t AddedtoBuf[16];
				wsprintf(AddedtoBuf, L"%2d:%2d:%2d.%3d", trade.time_added_to_buffer.wHour, trade.time_added_to_buffer.wMinute, trade.time_added_to_buffer.wSecond, trade.time_added_to_buffer.wMilliseconds);
				int ATS_msTime;
				AT_Helper::ConvertATSTimeToMS((SYSTEMTIME)trade.time_added_to_buffer, ATS_msTime);
				//	Trade Time
				wchar_t TimeofTrade[16];
				wsprintf(TimeofTrade, L"%2d:%2d:%2d.%3d", 
						trade.time_of_trade.trade_time.hour, trade.time_of_trade.trade_time.minute, trade.time_of_trade.trade_time.second, trade.time_of_trade.trade_time.milliseconds);
				int AT_msTime;
				AT_Helper::ConvertATTimeToMS((ATTIME)trade.time_of_trade.trade_time, AT_msTime);

				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_STATS))
				{
					swprintf(logMsg, sizeof logMsg, L"Added Trade [#%8u] to RDQ: %6s\t %4.4f\t %5u\t x%1u ToB:%s\t ToT:%s\t %d(ms)",
						pDS_Shared->insertionCountRDQ,
						trade.symbol,
						trade.trade_amt,
						trade.trade_vol,
						trade.precision,
						AddedtoBuf,
						TimeofTrade,
						ATS_msTime - AT_msTime
					);
					pLogger->log_stats(logMsg, L"AT_Streamer::OnATStreamTradeUpdate");
				}
			}
#endif _UNUSED
		}
		else
		{
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			{
				swprintf(logMsg, sizeof logMsg, L"Symbol:\tsymbol(%s)\tNot found - ignored", trade.symbol);
				pLogger->log_warn(logMsg, L"AT_Streamer::OnATStreamTradeUpdate");
			}
		}
	}
	catch (...)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			pLogger->log_warn(L"Failed on creation of TRADE_EVENT - ignore for now", L"AT_Streamer::OnATStreamTradeUpdate");
	}
	
	//	Adjustable stream trade callback cycle delay
	//	- Added to prevent callback throttling
	if (pDataConn->connCtx.callbackCycleDelay != -1)
	{
		Sleep(sleepTimeMS);	
	}

	return;
}




#ifdef _UNUSED_CODE_

	typedef struct _ATQUOTESTREAM_TRADE_UPDATE
	{
		ATSYMBOL symbol;
		ATTradeMessageFlags flags;
		ATTradeConditionType condition[ATTradeConditionsCount];
		ATExchangeType lastExchange;
		ATPRICE lastPrice;
		uint32_t lastSize;
		ATTIME lastDateTime;
	} ATQUOTESTREAM_TRADE_UPDATE, *LPATQUOTESTREAM_TRADE_UPDATE;

	typedef struct _ATSYMBOL
	{
		wchar16_t symbol[ATSymbolMaxLength];
		uint8_t symbolType;
		uint8_t exchangeType;
		uint8_t countryType;
	} ATSYMBOL, *LPATSYMBOL;

	typedef struct _ATTIME
	{
		uint16_t year;
		uint16_t month;
		uint16_t dayOfWeek;
		uint16_t day;
		uint16_t hour;
		uint16_t minute;
		uint16_t second;
		uint16_t milliseconds;
	} ATTIME, *LPATTIME;

	typedef struct _TRADE_DATA {
		unsigned __int64	time_added_to_buffer;
		unsigned __int64	time_of_trade;
		double				trade_amt;
		unsigned int		trade_vol;
		std::wstring		symbol;
		std::wstring		exchange;
		// ...more...
	} TRADE_DATA, TRADE;

	typedef struct _TRADE_EVENT
	{
		TRADE_EVENT_TYPE type;
		CIRCULAR_BUFFER_STATES state;
		TRADE_DATA trade_data;
	} TRADE_EVENT;

	printf("RECV: [%0.2d:%0.2d:%0.2d] STREAMTRADE [symbol:%s last:%0.*f lastSize:%d]\n",
		pUpdate->lastDateTime.hour, pUpdate->lastDateTime.minute, pUpdate->lastDateTime.second,
//		Helper::ConvertString(pUpdate->symbol.symbol, _countof(pUpdate->symbol.symbol)).c_str(),
		pUpdate->lastPrice.precision, pUpdate->lastPrice.price,
		pUpdate->lastSize);

	//print out trade info, plus how many updates are currently queued up locally
	/*printf("RECV: [%0.2d:%0.2d:%0.2d] STREAMTRADE(%u) [symbol:%s last:%0.*f lastSize:%d]\n",
		pUpdate->lastDateTime.wHour, pUpdate->lastDateTime.wMinute, pUpdate->lastDateTime.wSecond,
		ATGetSessionQueuedStreamUpdates(m_session.GetSessionHandle()),
		Helper::ConvertString(pUpdate->symbol.symbol, _countof(pUpdate->symbol.symbol)).c_str(),
		pUpdate->lastPrice.precision, pUpdate->lastPrice.price,
		pUpdate->lastSize);*/


	std::chrono

	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) -
		std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

	std::cout << std::put_time(std::localtime(&time), "%H h %M m %S s ");
	std::cout << ms.count() << " ms" << std::endl;

Output:

	21 h 24 m 22 s 428 ms

#endif _UNUSED_CODE_





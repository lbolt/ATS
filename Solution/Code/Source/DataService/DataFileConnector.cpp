////////////////////////////////////////////////////////////////////////////////
//
//	Data File Connector
//	- Currently supports Snapshot File-based provider
//
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DataFileConnector.h"
#include "DataCollectorWorker.h"
#include <string>
#include <vector>


using namespace ATS_DATASERVICE;


//	(Extern) Initialize and make visible in Module


unsigned int DataFileConnectorThreads::DataFileConnectorManager(void* Args)
{
	//
	//	Loop on raw data processing thread and system status
	//
	DS_Globals* pGlobals = (DS_Globals*)Args;
	DS_Shared* pSDShare = (DS_Shared*)pGlobals->Args[ARG_SDS];
	LoggerX* pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	DataFileConnector* pDataFileConn = (DataFileConnector*)pGlobals->Args[ARG_FILE_CONN];

	HRESULT hRC = S_OK;
	int sysState;
	int subSysState;
	int	sleepDormantTimeMS = 500; 
	int	sleepTimeMS = sleepDormantTimeMS;
	wchar_t logMsg[256];
	

	for (;;)
	{
		//	ConnectorManager
		Sleep(sleepTimeMS);

		sysState = g_pStates->DS_SYSTEM_ST;
		subSysState = g_pStates->DS_FILE_CONNECTOR_ST;

		switch (subSysState)
		{
		case ATS_DATASERVICE::RUN_ST:
			///////////////////////////////////////////////////////////////////
			//	Perform Snapshot data file processing
			//	- Part 1) Load buffered message queue data
			//	- Part 2) Process message queue data 
			//	Note: 
			//		- Does not use a callback
			//		- Future: calculate sleepTimeMS based on the number of 
			//		  TRADE records and their begin/end timestamps
			///////////////////////////////////////////////////////////////////
			ATS_CODE rc;
			rc = pDataFileConn->processSnapshotFile(pGlobals);
			if (rc == ATS_CODES::ATS_C_EMPTY)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					pLogger->log_info(L"DataFile (snapshot reader) Provider Connector set to DORMANT state", L"DataFileConnectorThreads::DataFileConnectorManager");
				g_pStates->DS_FILE_CONNECTOR_ST = ATS_DATASERVICE::DORMANT_ST;
			}
			break;

		case ATS_DATASERVICE::START_ST:
			//	
			if (pDataFileConn->isConnectorEnabled() == true)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					pLogger->log_info(L"DataFile (snapshot reader) Provider Connector subsystem is starting", L"DataFileConnectorThreads::DataFileConnectorManager");
				g_pStates->DS_FILE_CONNECTOR_ST = ATS_DATASERVICE::INPROGESS_UP1_ST;
			}
			else
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					pLogger->log_info(L"DataFile (snapshot reader) Provider Connector subsystem is ***disabled***", L"DataFileConnectorThreads::DataFileConnectorManager");
				g_pStates->DS_FILE_CONNECTOR_ST = ATS_DATASERVICE::DORMANT_ST;
				sleepTimeMS = sleepDormantTimeMS;
			}
			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			//	Perform preliminary connection step
			if (pDataFileConn->isDataFileProvConnected() == false)
			{
				if (pDataFileConn->connectToService() == ATS_C_SUCCESS)
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
						pLogger->log_info(L"Connected to DataFile (snapshot reader) provider service", L"DataFileConnectorThreads::DataFileConnectorManager");
					g_pStates->DS_FILE_CONNECTOR_ST = ATS_DATASERVICE::INPROGESS_UP2_ST;
				}
				else
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
						pLogger->log_warn(L"Unable to connect to DataFile (snapshot reader) provider service", L"DataFileConnectorThreads::DataFileConnectorManager");
					g_pStates->DS_FILE_CONNECTOR_ST = ATS_DATASERVICE::DORMANT_ST;
				}
			}
			break;

		case ATS_DATASERVICE::INPROGESS_UP2_ST:
			//	Wait for connection callback and issue service login
			if (pDataFileConn->isDataFileProvConnected() == true)
			{
				g_pStates->DS_FILE_CONNECTOR_ST = ATS_DATASERVICE::RUN_ST;
				sleepTimeMS = pDataFileConn->dataFileConnCtx.frequency; // Speed-up
				// Save connector start time
				pSDShare->providerConnectionTime = std::chrono::system_clock::now();
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
				{
					swprintf(logMsg, sizeof logMsg, L"DataFile (snapshot reader) provider service cycle time (%d) milliseconds", sleepTimeMS);
					pLogger->log_debug(logMsg, L"DataFileConnector::processSnapshotFile");
				}
				continue;
			}
			//	FIX: query connection timeout
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			// Set state for dependent subsystem(s)

			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			//	if the subsystem is not down then go into shutdown mode
			
			break;

		case ATS_DATASERVICE::DOWN_ST:
			//	
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				pLogger->log_info(L"DataFile Connector subsystem is stopped", L"DataFileConnectorThreads::DataFileConnectorManager");
			_endthreadex(hRC);
			break;

		case ATS_DATASERVICE::IDLE_ST:
			//	Idempotent
			sleepTimeMS = sleepDormantTimeMS;
			break;

		case ATS_DATASERVICE::DORMANT_ST:
			//	Idempotent
			sleepTimeMS = sleepDormantTimeMS;
			break;

		default:
			//	NOOP
			break;

		}
	}
}



///////////////////////////////////////////////////////////////////////////////
//	Data File Connector Definitions
///////////////////////////////////////////////////////////////////////////////
DataFileConnector::DataFileConnector(void* Args)
{
	int rc = initialize(Args);
}


DataFileConnector::~DataFileConnector()
{
}


ATS_CODE DataFileConnector::initialize(void* Args)
{
	//	Process Args input parameters
	//	- Objects are declared in PropsX::DS_globals
	//	- DS_Globals is created in PropsX
	//	- LoggerX is created in DataManager
	try
	{
		pGlobals = (DS_Globals*)Args;
		pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
		pDS_Shared = (DS_Shared*)pGlobals->Args[ARG_SDS];

		///////////////////////////////////////////////////////////////////////
		//	Setup the Connection Block
		///////////////////////////////////////////////////////////////////////
		initConnCtxBlock();
		master_tuid = DFC_MASTER_TUID;
	}
	catch (...)
	{
		//setATProvConnected(false);
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
			pLogger->log_info(L"Failed to initialize DataConnector", L"DataConnector::initialize");
	}

	return (ATS_C_SUCCESS);
}


//
//	Check Connector Enabled State
//
bool DataFileConnector::isConnectorEnabled()
{
	return (dataFileConnCtx.bEnabled);
}


// 
//	Initializes Provider Service Connection and Login information
//
ATS_CODE DataFileConnector::initConnCtxBlock()
{
	//	Configuration data
	(_wcsicmp(pGlobals->pProps->prop_DataFile_enabled.c_str(), L"true") == 0) ? dataFileConnCtx.bEnabled = true : dataFileConnCtx.bEnabled = false;
	(_wcsicmp(pGlobals->pProps->prop_DataFile_DisableRDQInsertion.c_str(), L"true") == 0) ? dataFileConnCtx.bDisableRDQInsertion = true : dataFileConnCtx.bDisableRDQInsertion = false;
	dataFileConnCtx.snapshot_name = pGlobals->pProps->prop_DataFile_snapshot_name.c_str();
	(_wcsicmp(pGlobals->pProps->prop_DataFile_insert_timestamp.c_str(), L"true") == 0) ? dataFileConnCtx.insert_timestamp = true : dataFileConnCtx.insert_timestamp = false;
	dataFileConnCtx.frequency = _wtoi(pGlobals->pProps->prop_DataFile_frequency.c_str());
	dataFileConnCtx.tradeRecordsPerCycle = _wtoi(pGlobals->pProps->prop_DataFile_trade_records_per_cycle.c_str());

	//	File processing data
	dataFileConnCtx.bConnected = false;
	dataFileConnCtx.curr_pos = 0;
	dataFileConnCtx.snapshot_path.append(pGlobals->moduleBasePath);
	dataFileConnCtx.snapshot_path.append(L"snapshot\\");
	dataFileConnCtx.snapshot_path.append(dataFileConnCtx.snapshot_name);
	dataFileConnCtx.bSnapshotFileEmpty = false;
	dataFileConnCtx.tradeRecordCount = 0;
	dataFileConnCtx.queueThreshold = 0;
	dataFileConnCtx.queueFillLimit = 1000;
	return ATS_C_SUCCESS;
}


//
//	Check for service connection
//
bool DataFileConnector::isDataFileProvConnected()
{
	return (dataFileConnCtx.bConnected);
}


// 
//	Provider Service Bind Routine
//
ATS_CODE DataFileConnector::connectToService()
{
	DataFileConnector* pDataFileConn = (DataFileConnector*)pGlobals->Args[ARG_FILE_CONN];
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
		pLogger->log_trace(L"Initiate API Session connect", L"DataFileConnector_Routines::connectToService");
	pDataFileConn->dataFileConnCtx.bConnected = true;
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
		pLogger->log_info(L"DataFile service connected", L"DataFileConnector_Routines::connectToService");

	return ATS_C_SUCCESS;
}


ATS_CODE DataFileConnector::convertToTradeRec(wchar_t* snapMsg, TRADE& tradeRec)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	wchar_t* next_token;
	wchar_t* token;

	//	This is a quick hack
	token = wcstok_s(snapMsg, L" \t\n", &next_token);
	if (token == NULL) return ATS_C_EMPTY;

	AT_Helper::CopyWChartoWCharPadded((wchar_t&)*token, (int)wcslen(token), (wchar_t&)tradeRec.symbol, ATS_StockSymbolMaxLength);
	token = wcstok_s(NULL, L" \t\n", &next_token);
	tradeRec.trade_amt = _wtof(token);
	token = wcstok_s(NULL, L" \t\n", &next_token);
	tradeRec.trade_vol = _wtoi(token);
	token = wcstok_s(NULL, L" \t\n", &next_token);
	tradeRec.precision = (UINT8)_wtoi(token);

	//tradeRec.tuid = master_tuid++;

	//tradeRec.time_of_trade = 

	//tradeRec.time_added_to_buffer.time_point = std::chrono::system_clock::now();  // Uses system clock time-point

		// Lookup db sequence for symbol
	std::map<std::wstring, unsigned int>::iterator it;
	it = pDS_Shared->pSequenceLookupMap->symToseqNumLookupHash.find(tradeRec.symbol);
	if (it == pDS_Shared->pSequenceLookupMap->symToseqNumLookupHash.end())
	{
		tradeRec.db_sequence = 0;
		rc = ATS_C_NONEXIST;
	}
	else
	{
		tradeRec.db_sequence = it->second;
		rc = ATS_C_SUCCESS;
	}

	//
	return rc;
}


ATS_CODE DataFileConnector::processSnapshotFile(void* pArgs)						// <----------------------------------- Process Snapshot and Trade Records -------------------------------------------
{
	ATS_CODE rc = ATS_C_SUCCESS;
	wchar_t logMsg[MSG_BUFFER_WSIZE::MEDIUM];

	DS_Globals* pGlobalArgs;	//	Added
	DS_Active_CTX* pDSACtx;		//	Added
	DS_Context* pDSCtx;			//	Added
	DataCollectorWorker* pDataCollectorWorker;

	pGlobalArgs = (DS_Globals*)pArgs;
	pDataCollectorWorker = (DataCollectorWorker*)pGlobalArgs->Args[ARG_COLL];

	///////////////////////////////////////////////////////
	//	Part 1.
	//	- When below queue threshold, add more records 
	//	  to the queue
	//	Returns:
	//		ATS_C_SUCESS
	//		ATS_C_EMPTY
	///////////////////////////////////////////////////////

	if (dataFileConnCtx.bSnapshotFileEmpty == false)
	{
		dataFileConnCtx.snapshot_fs.open(dataFileConnCtx.snapshot_path, ios_base::in);
		if (dataFileConnCtx.snapshot_fs.is_open())
		{
			//	Process snapshot data file
			wchar_t snapMsg[MSG_BUFFER_WSIZE::SMALL];
			TRADE tradeRec;
			//  Check queue size against threshold
			if (dataFileConnCtx.tradeMsg.size() <= dataFileConnCtx.queueThreshold)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
				{
					swprintf(logMsg, sizeof logMsg, L"Get more (%d) records from Snapshot file", dataFileConnCtx.queueFillLimit);
					pLogger->log_debug(logMsg, L"DataFileConnector::processSnapshotFile");
				}
				//	Add more data to queue until fill-limit has been met
				for (int count = dataFileConnCtx.queueFillLimit; count > 0; count--)
				{
					dataFileConnCtx.snapshot_fs.seekg(dataFileConnCtx.curr_pos);
					dataFileConnCtx.snapshot_fs.getline(snapMsg, MSG_BUFFER_WSIZE::SMALL, L'\n');
					if (dataFileConnCtx.snapshot_fs.eof())
					{
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
						{
							swprintf(logMsg, sizeof logMsg, L"Reached end of Snapshot File: %s, Record Count: %d", dataFileConnCtx.snapshot_path.c_str(), dataFileConnCtx.tradeRecordCount);
							pLogger->log_info(logMsg, L"DataFileConnector::processSnapshotFile");
						}
						dataFileConnCtx.bSnapshotFileEmpty = true;
						rc = ATS_C_EMPTY;
						count = 0;	// break
					}
					else
					{
						//	Process snapshot data file trade record
						rc = convertToTradeRec(snapMsg, tradeRec);
						if (rc == ATS_C_SUCCESS)
						{
							dataFileConnCtx.tradeMsg.push(tradeRec);
							dataFileConnCtx.tradeRecordCount++;
						}
						else
						{
							//	Not watching this symbol - ignore
							;
						}
						dataFileConnCtx.curr_pos = dataFileConnCtx.snapshot_fs.tellg();
					}
				}
			}
			dataFileConnCtx.snapshot_fs.close();
		}
		else
		{
			//	Some sort of error - shut down reader
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			{
				swprintf(logMsg, sizeof logMsg, L"Unable to open Snapshot File Path: %s", dataFileConnCtx.snapshot_path.c_str());
				pLogger->log_warn(logMsg, L"DataFileConnector::processSnapshotFile");
			}
			rc = ATS_C_EMPTY;
		}
	}
	
	///////////////////////////////////////////////////////
	//	Part 2.
	//	- Process next message in the snapshot trade queue
	//
	//	Note: The sleep timer wakes on a per message 
	//		  basis. The timer value is calculated based
	//		  on the number of messages in the snapshot
	//		  file and the begin/end timestamps
	///////////////////////////////////////////////////////

	if (rc != ATS_C_SUCCESS && rc != ATS_C_EMPTY && rc != ATS_C_NONEXIST)
	{
		if (rc == ATS_C_NONEXIST)
		{
			return ATS_C_SUCCESS;
		}
		else 
		{
			//	A serious error occurred. Let the FSM change the thread state to DORMANT
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			{
				pLogger->log_error(L"An error occurred while adding records to message queue", L"DataFileConnector::processSnapshotFile");
			}
		}
		return ATS_C_SUCCESS;
	}

	//	This will always be > 0 unless there are no remaining records in which case the reader goes DORMANT
	if (dataFileConnCtx.tradeMsg.size() > 0)
	{
		rc = ATS_C_SUCCESS;
		//	Process next snapshot trade queue entry
		TRADE tradeRecFromQ;
		try
		{
			int iterations = 0;
			if (dataFileConnCtx.tradeMsg.size() > dataFileConnCtx.tradeRecordsPerCycle)
			{
				iterations = dataFileConnCtx.tradeRecordsPerCycle;
			}
			else
			{
				iterations = 1;
			}
			for (; iterations > 0; iterations--)
			//for (int iterations = dataFileConnCtx.tradeRecordsPerCycle; iterations>0; iterations--)
			{
				tradeRecFromQ = dataFileConnCtx.tradeMsg.front();	//	Next trade record to process
				dataFileConnCtx.tradeMsg.pop();

				if (tradeRecFromQ.db_sequence != 0)
				{
					//	Added for direct insertion to <T> = _ATS_TRADE_DATA
					pDSACtx = pDS_Shared->pActiveCTXBlock->active_CTX_Container[tradeRecFromQ.db_sequence];
					pDSCtx = pDSACtx->pCtx;

					//	Insert TRADE record into RDQ
					if (dataFileConnCtx.bDisableRDQInsertion == false)
					{
						pDSACtx->DS_Active_sso.lock();
						pDSCtx->tickBuf.insert_back((TICK_DATA*)&tradeRecFromQ);
						pDS_Shared->insertionCountRDQ++;	//	Keep track of count only
						pDSACtx->DS_Active_sso.release();

						//	Send NRQ event message and post notification
						rc = pDataCollectorWorker->processAnalyticsNotification(tradeRecFromQ.db_sequence);
						if (rc != ATS_C_SUCCESS)
						{
							if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
							{
								swprintf(logMsg, sizeof logMsg, L"processAnalyticsNotification failed rc=(%d)", rc);
								pLogger->log_warn(logMsg, L"DataFileConnector::processSnapshotFile");
							}
						}

						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
						{
							swprintf(logMsg, sizeof logMsg, L"Inserted trade record into Active Context TICK buffer [symbol %6s, db_seq %d, cb_active %d]",
								tradeRecFromQ.symbol, tradeRecFromQ.db_sequence, pDSCtx->tickBuf.num_active_elements());
							pLogger->log_debug(logMsg, L"DataFileConnector::processSnapshotFile");
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
							chrono::duration<double> timeDiff = std::chrono::system_clock::now() - pDS_Shared->providerConnectionTime;
							swprintf(logMsg, sizeof logMsg, L"Current trade count: #%8u\t CB-Size: %d\t Active-Elements: %u\t TPS: %4.2fs/%4.2fm",
								pDS_Shared->insertionCountRDQ,
								pDS_Shared->pRDQ->size_of_container(),
								pDS_Shared->pRDQ->num_active_elements(),
								(double)pDS_Shared->insertionCountRDQ / timeDiff.count(),
								((double)pDS_Shared->insertionCountRDQ / timeDiff.count())*60.0
							);
							pLogger->log_info(logMsg, L"DataFileConnector::processSnapshotFile");
						}
					}

					// For testing purposes - dump trade record
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
					{
						swprintf(logMsg, sizeof logMsg, L"Added Trade [#%8u] to RDQ: %6s\t %4.4f\t %5u\t x%1u %d:%u",
							pDS_Shared->insertionCountRDQ,
							tradeRecFromQ.symbol,
							tradeRecFromQ.trade_amt,
							tradeRecFromQ.trade_vol,
							tradeRecFromQ.precision,
							pDS_Shared->pRDQ->size_of_container(),
							pDS_Shared->pRDQ->num_active_elements()
						);
						pLogger->log_debug(logMsg, L"DataFileConnector::processSnapshotFile");
					}

				}
			}
		}
		catch (...)
		{
			//	Sanity check
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			{
				pLogger->log_error(L"Caught ***EXCEPTION*** while processing next message in the snapshot trade queue", L"DataFileConnector::processSnapshotFile");
			}
		}
		rc = ATS_C_SUCCESS;
	}
	else 
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
		{
			pLogger->log_debug(L"Snapshot trade queue empty - enter DORMANT state", L"DataFileConnector::processSnapshotFile");
		}
		rc = ATS_C_EMPTY;
	}

	return rc;
}



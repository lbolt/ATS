#include "stdafx.h"
#include "DataCollector.h"




///////////////////////////////////////////////////////////////////////////////////////////
//	Runnable thread
//	- Manages data collection processing threads
//	- Performs housekeeping operations on the raw data queue 
///////////////////////////////////////////////////////////////////////////////////////////
unsigned DataCollectorThreads::CollectorManager(void* Args)
{
	//	Process Args input parameters
	//	- Objects are declared in PropsX::DS_globals
	//	- DS_Globals is created in PropsX
	//	- LoggerX is created in DataManager
	DS_Globals* pGlobals = (DS_Globals*)Args;
	LoggerX* pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	DataCollector* pDataColl = (DataCollector*)pGlobals->Args[ARG_COLL];
	// ++ Might be able to eliminate
	DataConnector* pDataConn = (DataConnector*)pGlobals->Args[ARG_CONN];
	DataFileConnector* pDataFileConn = (DataFileConnector*)pGlobals->Args[ARG_FILE_CONN];
	// -- Might be able to eliminate

	ATS_CODE rc = S_OK;
	int sysState;
	int subSysState;
	int	sleepTimeMS = 500;

	//	Main loop:
	//	- Cycle through state engine based on specified timeout value
	//	- Start worker threads and put subsystem in RUN_ST
	//	- Worker threads will cycle through work as it is delivered by the data provider callback routine
	//	
	for (;;)
	{
		//	CollectorManager
		//	Change to wait on multiple objects = TMO + Event --- FIX
		Sleep(sleepTimeMS);

		sysState = g_pStates->DS_SYSTEM_ST;
		subSysState = g_pStates->DS_COLLECTOR_ST;
		switch (subSysState)
		{

		case ATS_DATASERVICE::RUN_ST:
			///////////////////////////////////////////////////////////////////
			//	Perform necessary housekeeping
			///////////////////////////////////////////////////////////////////
			
			break;

		case ATS_DATASERVICE::START_ST:
			//	Started by DBManager
			// ...do required initializations...
			g_pStates->DS_COLLECTOR_ST = ATS_DATASERVICE::INPROGESS_UP1_ST;
			g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::START_ST;
			g_pStates->DS_FILE_CONNECTOR_ST = ATS_DATASERVICE::START_ST;
			// Starts the collector workers separately in  pDataColl->manageWorkers
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
				pLogger->log_trace(L"Collector subsystem is starting", L"DataCollectorThreads::ConnectorManager");
			break;

		case ATS_DATASERVICE::RESTART_ST:
			//	NOOP
			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			//	Check Prerequisites

			//	Manage worker threads
			if (pGlobals->workerDCThreadsIndexFull() == false)
			{
				//	Start threads
				rc = pDataColl->manageWorkers(DataServiceCommands::CMD_START_THREADS);
				if (rc == ATS_C_SUCCESS)
				{
					continue;
				}
				else
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
						pLogger->log_info(L"Collector subsystem experienced an error while starting workers", L"DataCollectorThreads::CollectorManager");
					g_pStates->DS_COLLECTOR_ST = ATS_DATASERVICE::SHUTDOWN_ST;
					continue;
				}
			}
			// Check connector state
#ifdef _UNUSED_CODE_
			if (pDataConn->isConnected() == true)
			{
				g_pStates->DS_COLLECTOR_ST = ATS_DATASERVICE::RUN_ST;
				pLogger->log_info(L"Collector subsystem has started", L"DataCollectorThreads::CollectorManager");
				continue;
			}
#endif _UNUSED_CODE_
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				pLogger->log_info(L"Collector subsystem has started", L"DataCollectorThreads::ConnectorManager");
			g_pStates->DS_COLLECTOR_ST = ATS_DATASERVICE::RUN_ST; 
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			//	Set state for dependent subsystem(s)
			if (g_pStates->DS_DCWORKER_ST == ATS_DATASERVICE::RUN_ST || g_pStates->DS_DCWORKER_ST == ATS_DATASERVICE::DORMANT_ST)
			{
				g_pStates->DS_DCWORKER_ST = ATS_DATASERVICE::SHUTDOWN_ST;
			}
			if (g_pStates->DS_CONNECTOR_ST == ATS_DATASERVICE::RUN_ST || g_pStates->DS_CONNECTOR_ST == ATS_DATASERVICE::DORMANT_ST)
			{
				g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::SHUTDOWN_ST;
			}
			if (g_pStates->DS_FILE_CONNECTOR_ST == ATS_DATASERVICE::RUN_ST || g_pStates->DS_FILE_CONNECTOR_ST == ATS_DATASERVICE::DORMANT_ST)
			{
				g_pStates->DS_FILE_CONNECTOR_ST = ATS_DATASERVICE::SHUTDOWN_ST;
			}
			g_pStates->DS_COLLECTOR_ST = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			//	All subsystems must be down before setting to DONE_ST
			if (g_pStates->DS_CONNECTOR_ST != ATS_DATASERVICE::DOWN_ST)
			{
				continue;
			}
			if (g_pStates->DS_FILE_CONNECTOR_ST != ATS_DATASERVICE::DOWN_ST)
			{
				continue;
			}
			rc = pDataColl->manageWorkers(DataServiceCommands::CMD_STOP_THREADS);
			if (rc == ATS_C_SUCCESS)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					pLogger->log_info(L"Collector subsystem experienced an error while stopping workers", L"DataCollectorThreads::CollectorManager");
			}
			//	When complete set subsystem state to STOPPED
			g_pStates->DS_COLLECTOR_ST = ATS_DATASERVICE::DOWN_ST;
			break;

		case ATS_DATASERVICE::DOWN_ST:
			//	Log shutdown message && exit the thread
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				pLogger->log_info(L"Collector subsystem has stopped", L"DataCollectorThreads::CollectorManager");
			_endthreadex(rc);
			break;

		case ATS_DATASERVICE::IDLE_ST:
			//	Idempotent
			break;

		case ATS_DATASERVICE::DORMANT_ST:
			//	Idempotent
			break;

		default:
			//	NOOP
			break;
		}
	}

	//	Subsystem is shutdown
	return(S_OK);
}





///////////////////////////////////////////////////////////////////////////////
//	Data Collector Definitions
DataCollector::DataCollector(void* Args)
{
	bCollectionEnabled = false;
	int rc = initialize(Args);
}


DataCollector::~DataCollector()
{
}


///////////////////////////////////////////////////////////////////////////////
//	Check collection service status
bool DataCollector::isCollectionEnabled()
{
	return bCollectionEnabled;
}

///////////////////////////////////////////////////////////////////////////////
//	Check workers status
bool DataCollector::workersInitialized()
{
	return bWorkersInitialized;
}

///////////////////////////////////////////////////////////////////////////////
//	Perform all Collector initializations
int DataCollector::initialize(void* Args)
{
	//	Process Args input parameters
	//	- Objects are declared in PropsX::DS_globals
	//	- DS_Globals is created in PropsX
	//	- LoggerX is created in DataManager
	pGlobals = (DS_Globals*)Args;
	pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	pSDShare = (DS_Shared*)pGlobals->Args[ARG_SDS];
	pDataConn = (DataConnector*)pGlobals->Args[ARG_CONN];


	ATS_CODE rc=ATS_C_SUCCESS;

	try
	{
		


	}
	catch (...)
	{

	}

	return (rc);
}

///////////////////////////////////////////////////////////////////////////////
//	Manage Data Collection  worker threads
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataCollector::manageWorkers(int command)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	wchar_t msg[LOGGER_MSG_SMALLBUF];
	try
	{
		///////////////////////////////////////////////////////////////////////
		//	Manage data collection processing worker threads
		//	- There may be many of these worker threads (see MAX_DCWORKER)
		//	- Each worker thread is designed to acquire packets from the RDQ, 
		//		restructure the data and move it to security specific queues for 
		//		subsequent analytic processing
		///////////////////////////////////////////////////////////////////////
		if (command == DataServiceCommands::CMD_START_THREADS)
		{
			while (g_pGlobals->DC_THRD_COUNT < MAX_DCWORKER)
			{
				int workerIndex;
				//	Start worker thread pool
				rc = g_pGlobals->getNextDCThreadIndex(workerIndex);	//	Get next available index
				if (rc != ATS_C_SUCCESS)
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
					{
						wsprintf(msg, L"Unable to create Collector worker thread Slot #%d with return = #%d", workerIndex, rc);
						pLogger->log_error(msg, L"DataCollector");
					}
					break;
				}
				g_pGlobals->gDCTState[workerIndex] = DS_SYSTEM_STATES::IDLE_ST;
				g_pGlobals->gDCTAddr[workerIndex] = (HANDLE)::_beginthreadex(
					NULL,
					0,
					&DataCollectorThreads::CollectorWorker,
					pGlobals,
					0,
					(unsigned int*)&g_pGlobals->gDCTIDs[workerIndex]
				);
				//	TODO: verify thread creation !!!

				g_pGlobals->incrDCThreadIndexCount();
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				{
					wsprintf(msg, L"Started Collector worker thread Slot #%d - TID #%d (0x%x)", workerIndex, g_pGlobals->gDCTIDs[workerIndex], g_pGlobals->gDCTIDs[workerIndex]);
					pLogger->log_info(msg, L"DataCollector");
				}
			}

		}
		else if (command == DataServiceCommands::CMD_STOP_THREADS)
		{
			//	Shutdown worker thread pool


			rc = ATS_C_SUCCESS;
		}
	}
	catch (...)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			pLogger->log_error(L"Exception during DC worker thread startup.", L"DataCollector");
		rc = ATS_C_FAIL;
	}

	return (rc);
}


#ifdef _UNUSED_CODE_
///////////////////////////////////////////////////////////////////////////////
//	Add/Remove Dequeues from DequeBlock
int DataCollector::updateDequeBlock()
{
	int rc = ATS_OK;


	try 
	{
		//	Check to see if there are differences in size
		if (pSDShare->pDequeBlock->DCDBlock.size() != pSDShare->pWatchPool->watchPoolItems.size())
		{
			//	Add/Remove work items to/from DequeBlock
			if (pSDShare->pDequeBlock->DCDBlock.size() == 0)	//	Is it the first time
			{
				std::list<WatchPoolItem*> pWP = pSDShare->pWatchPool->watchPoolItems;
				DataCollectionDequeBlock* pDB = pSDShare->pDequeBlock;
				
				//	Add entire watch pool list
				std::list<WatchPoolItem*>::iterator it;
				WatchPoolItem* pWPI;
				for (it = pWP.begin(); it != pWP.end(); ++it)
				{
					pWPI = (WatchPoolItem*)*it;
					//	Set the sequence number and create a data collection dequeue for insertion
					DataCollectionDeque* pNewDB = new DataCollectionDeque();
					pDB->DCDBlock.insert(std::pair<unsigned int, DataCollectionDeque*>(pWPI->sequence, pNewDB));
				}
			}
			else
			{
				//	TODO: Implement update scenario
				//	- Figure out differences and apply them

			}
		}
		//rc = ATS_FAILED;
	}
	catch (...)
	{
		pLogger->log_warn(L"Unspecified error", L"DataCollector::updateDequeBlock");
	}


	//	Returns the number of items effected by update or -1 for errors
	return rc;
}
#endif _UNUSED_CODE_


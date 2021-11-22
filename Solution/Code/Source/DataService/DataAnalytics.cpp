#include "stdafx.h"
#include "DataAnalytics.h"
#include "DataManagerDB.h"


using namespace ATS_DATASERVICE;



///////////////////////////////////////////////////////////////////////////////////////////
//	Runnable thread
//	- Manages ANALYTICS processing threads
//	- Performs housekeeping for Analytics workers
///////////////////////////////////////////////////////////////////////////////////////////
unsigned DataAnalyticsThreads::AnalyticsManager(void* Args)
{
	DS_Globals* pGlobals = (DS_Globals*)Args;
	LoggerX* pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	DataManagerDB* pDBManager = (DataManagerDB*)pGlobals->Args[ARG_DBM];
	DataAnalytics* pDataAnalytics = (DataAnalytics*)pGlobals->Args[ARG_ANA];

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
		//	AnalyticsManager
		//	Change to wait on multiple objects = TMO + Event --- FIX
		Sleep(sleepTimeMS);
		
		sysState = g_pStates->DS_SYSTEM_ST;
		subSysState = g_pStates->DS_ANALYTICS_ST;
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
			g_pStates->DS_ANALYTICS_ST = ATS_DATASERVICE::INPROGESS_UP1_ST;
			// Starts the Analytics workers separately in  pDataAnalytics->manageWorkers
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
				pLogger->log_trace(L"Analytics subsystem is starting", L"DataAnalyticsThreads::AnalyticsManager");
			break;

		case ATS_DATASERVICE::RESTART_ST:
			//	NOOP
			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			//	Check Prerequisites

			//	Manage worker threads
			if (pGlobals->workerDAThreadsIndexFull() == false)
			{
				//	Start threads
				rc = pDataAnalytics->manageWorkers(DataServiceCommands::CMD_START_THREADS);
				if (rc == ATS_C_SUCCESS)
				{
					continue;
				}
				else
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
						pLogger->log_info(L"Analytics subsystem experienced an error while starting workers", L"DataAnalyticsThreads::AnalyticsManager");
					g_pStates->DS_ANALYTICS_ST = ATS_DATASERVICE::SHUTDOWN_ST;
					continue;
				}
			}
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				pLogger->log_info(L"Analytics subsystem has started", L"DataAnalyticsThreads::AnalyticsManager");
			g_pStates->DS_ANALYTICS_ST = ATS_DATASERVICE::RUN_ST;
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			//	Set state for dependent subsystem(s)
			if (g_pStates->DS_DAWORKER_ST == ATS_DATASERVICE::RUN_ST || g_pStates->DS_DAWORKER_ST == ATS_DATASERVICE::RUN_ST)
			{
				g_pStates->DS_DAWORKER_ST = ATS_DATASERVICE::SHUTDOWN_ST;
			}
			g_pStates->DS_ANALYTICS_ST = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			//	All subsystems must be down before setting to DONE_ST

			rc = pDataAnalytics->manageWorkers(DataServiceCommands::CMD_STOP_THREADS);
			if (rc == ATS_C_SUCCESS)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					pLogger->log_info(L"Analytics subsystem experienced an error while stopping workers", L"DataAnalyticsThreads::DataAnalytics");
			}
			//	When complete set subsystem state to STOPPED
			g_pStates->DS_ANALYTICS_ST = ATS_DATASERVICE::DOWN_ST;
			break;

		case ATS_DATASERVICE::DOWN_ST:
			//	Log shutdown message && exit the thread
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				pLogger->log_info(L"Analytics subsystem has stopped", L"DataAnalyticsThreads::DataAnalytics");
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
//	Data Analytics Definitions
DataAnalytics::DataAnalytics(void* Args)
{
	bAnalyticsEnabled = false;
	int rc = initialize(Args);

}


DataAnalytics::~DataAnalytics()
{
}


///////////////////////////////////////////////////////////////////////////////
//	Check Analytics service status
bool DataAnalytics::isAnalyticsEnabled()
{
	return bAnalyticsEnabled;
}

///////////////////////////////////////////////////////////////////////////////
//	Check workers status
bool DataAnalytics::workersInitialized()
{
	return bWorkersInitialized;
}


///////////////////////////////////////////////////////////////////////////////
//	Perform all Analytics initializations
int DataAnalytics::initialize(void* Args)
{
	//	Process Args input parameters
	//	- Objects are declared in PropsX::DS_globals
	//	- DS_Globals is created in PropsX
	//	- LoggerX is created in DataManager
	pGlobals = (DS_Globals*)Args;
	pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	pSDShare = (DS_Shared*)pGlobals->Args[ARG_SDS];

	
	ATS_CODE rc = ATS_C_SUCCESS;

	try
	{



	}
	catch (...)
	{

	}

	return (rc);
}


///////////////////////////////////////////////////////////////////////////////
//	Manage Data Analytics worker threads
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataAnalytics::manageWorkers(int command)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	wchar_t msg[LOGGER_MSG_SMALLBUF];
	try
	{
		///////////////////////////////////////////////////////////////////////
		//	Manage data Analytics processing worker threads
		//	- There may be many of these worker threads (see MAX_DAWORKER)
		//	- Each worker thread is designed to acquire packets from the RDQ, 
		//		restructure the data and move it to security specific queues for 
		//		subsequent analytic processing
		///////////////////////////////////////////////////////////////////////
		if (command == DataServiceCommands::CMD_START_THREADS)
		{
			while (g_pGlobals->DA_THRD_COUNT < MAX_DAWORKER)
			{
				int workerIndex;
				//	Start worker thread pool
				rc = g_pGlobals->getNextDAThreadIndex(workerIndex);	//	Get next available index
				if (rc != ATS_C_SUCCESS)
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
					{
						wsprintf(msg, L"Unable to create Analytics worker thread Slot #%d with return = #%d", workerIndex, rc);
						pLogger->log_error(msg, L"DataAnalytics");
					}
					break;
				}
				g_pGlobals->gDATState[workerIndex] = DS_SYSTEM_STATES::IDLE_ST;
				g_pGlobals->gDATAddr[workerIndex] = (HANDLE)::_beginthreadex(
					NULL,
					0,
					&DataAnalyticsThreads::AnalyticsWorker,
					pGlobals,
					0,
					(unsigned int*)&g_pGlobals->gDATIDs[workerIndex]
				);
				//	TODO: verify thread creation !!!

				g_pGlobals->incrDAThreadIndexCount();
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				{
					wsprintf(msg, L"Started Analytics worker thread Slot #%d - TID #%d (0x%x)", workerIndex, g_pGlobals->gDATIDs[workerIndex], g_pGlobals->gDATIDs[workerIndex]);
					pLogger->log_info(msg, L"DataAnalytics");
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
			pLogger->log_error(L"Exception during DA worker thread startup.", L"DataAnalytics");
		rc = ATS_C_FAIL;
	}

	return (rc);
}



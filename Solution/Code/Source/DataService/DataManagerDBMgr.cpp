#include "stdafx.h"
#include "DataManagerDB.h"
#include "DataManagerDBMgr.h"



// 
//	Runnable thread
//	- Manages logger subsystem 
//	- Perform scheduled wake and move items from queue to logfile
//	- Last thread to terminate prior to shutdown of DataManager
//
unsigned DataManagerDBMgrThreads::DBManager(void* Args)
{
	//	Process Args input parameters
	//	- Objects are declared in PropsX::DS_globals
	//	- DS_Globals is created in PropsX
	//	- LoggerX is created in DataManager
	DS_Globals* pGlobals = (DS_Globals*)Args;
	LoggerX* pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	DataManagerDB* pDBManager = (DataManagerDB*)pGlobals->Args[ARG_DBM];

	ATS_CODE rc = ATS_OK;
	int subSysState;
	int	sleepTimeMS = 1000;


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Main thread loop
	//	- Process outstanding messages on the queue
	//	- Manage the Logger queue thread state
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	for (;;)
	{
		//	DBManager
		//	Change to wait on multiple objects = TMO + Event --- FIX
		Sleep(sleepTimeMS);

		subSysState = g_pStates->DS_DBMANAGER_ST;	// Subsystem State
		switch (subSysState)
		{
		case ATS_DATASERVICE::RUN_ST:
			///////////////////////////////////////////////////////////////////
			//	Perform necessary housekeeping
			///////////////////////////////////////////////////////////////////
			if (pDBManager->isConnected() == false) {
				// Connection lost, will need to restart
				g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::RESTART_ST;
			}
			break;

		case ATS_DATASERVICE::START_ST:
			//	Start/Restart DB Manager if necessary
			g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::INPROGESS_UP1_ST;
			break;

		case ATS_DATASERVICE::RESTART_ST:
			//	Start/Restart DB Manager if necessary
			if (pDBManager->isConnected() == false)
			{
				//	Reconnect to the database
				int rc = pDBManager->connectDB();
				if (rc == ATS_OK)
				{
					pDBManager->bConnected = true;
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
						pLogger->log_trace(L"Database connection has been reestablished", L"DataManagerDBMgrThreads::DBManager");
					g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::RUN_ST;
				}
				else
				{
					// Failed - continue retry loop
				}
			}
			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			ATS_CODE rc;
			if (pDBManager->isConnected() == false)
			{
				rc = pDBManager->connectDB();
				if (rc == ATS_OK)
				{
					pDBManager->bConnected = true;
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
						pLogger->log_trace(L"Database manager is connected", L"DataManagerDBMgrThreads::DBManager");
				}
				else
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
						pLogger->log_error(L"Database manager did not start - see logfile", L"DataManagerDBMgrThreads::DBManager");
					g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::SHUTDOWN_ST;
					continue;
				}
			}
			//	Connect to the database
			if (pDBManager->isWatchInitialized() == false)
			{
				//	Initialize the Watchlist
				rc = pDBManager->loadWatchRecs();
				if (rc == ATS_OK)
				{
					pDBManager->bWatchInitialized = true;
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
						pLogger->log_info(L"Database manager has started", L"DataManagerDBMgrThreads::DBManager");
					g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::RUN_ST;
					// Start Analytics and Data Collection subsystems
					g_pStates->DS_ANALYTICS_ST = ATS_DATASERVICE::START_ST;
					g_pStates->DS_COLLECTOR_ST = ATS_DATASERVICE::START_ST;
				}
				else
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
						pLogger->log_error(L"Database manager failed to initialize watch list - see logfile", L"DataManagerDBMgrThreads::DBManager");
					g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::SHUTDOWN_ST;
					continue;
				}
			}
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			// Set state for dependent subsystem(s)
			if (g_pStates->DS_ANALYTICS_ST == ATS_DATASERVICE::RUN_ST || g_pStates->DS_ANALYTICS_ST == ATS_DATASERVICE::DORMANT_ST)
			{
				g_pStates->DS_ANALYTICS_ST = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			}
			if (g_pStates->DS_COLLECTOR_ST == ATS_DATASERVICE::RUN_ST || g_pStates->DS_COLLECTOR_ST == ATS_DATASERVICE::DORMANT_ST)
			{
				g_pStates->DS_COLLECTOR_ST = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			}
			g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			// All subsystems must be down before setting to DONE_ST
			if (g_pStates->DS_ANALYTICS_ST != ATS_DATASERVICE::DOWN_ST)
			{
				continue;
			}
			if (g_pStates->DS_COLLECTOR_ST != ATS_DATASERVICE::DOWN_ST)
			{
				continue;
			}
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
				pLogger->log_trace(L"Database  manager is stopped", L"DataManagerDBMgrThreads::DBManager");
			g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::DOWN_ST;
			break;

		case ATS_DATASERVICE::DOWN_ST:
			//	Log shutdown message && exit the thread
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

	// Set RDP state to RDP_DOWN
	return(ATS_OK);
}

DataManagerDBMgr::DataManagerDBMgr()
{
}


DataManagerDBMgr::~DataManagerDBMgr()
{
}



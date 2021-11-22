#include "stdafx.h"
#include "DataServiceUtilityMgr.h"
#include "DataManagerDB.h"


DataServiceUtilityMgr::DataServiceUtilityMgr()
{
}


DataServiceUtilityMgr::~DataServiceUtilityMgr()
{
}


///////////////////////////////////////////////////////////////////////////////
//	Runnable thread
//	- Manages Utilities subsystem 
//	- Perform scheduled wake and move items from queue to logfile
//	- Last thread to terminate prior to shutdown of DataManager
///////////////////////////////////////////////////////////////////////////////
unsigned DataServiceUtilityMgrThreads::UtilityManager(void* Args)
{
	//	Process Args input parameters
	//	- Objects are declared in PropsX::DS_globals
	//	- DS_Globals is created in PropsX
	//	- LoggerX is created in DataManager
	DS_Globals* pGlobals = (DS_Globals*)Args;
	LoggerX* pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	DataLoggerX* pDataLogger = (DataLoggerX*)pGlobals->Args[ARG_SNAP];
	DataManagerDB* pDBManager = (DataManagerDB*)pGlobals->Args[ARG_DBM];

	ATS_CODE rc = ATS_OK;
	int msgCount;
	int subSysState;
	int	sleepTimeMS = 1000;


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Main thread loop
	//	- Process outstanding messages on the queue
	//	- Manage the Logger queue thread state
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	for (;;)
	{
		//	UtilityManager
		//	Change to wait on multiple objects = TMO + Event --- FIX
		Sleep(sleepTimeMS);

		subSysState = g_pStates->DS_UTILITY_ST;
		switch (subSysState)
		{
		case ATS_DATASERVICE::RUN_ST:
			///////////////////////////////////////////////////////////////////
			//	Perform necessary housekeeping
			///////////////////////////////////////////////////////////////////
			if (pLogger->isEnabled() == true)
			{
				if (pLogger->isInitialized())
				{
					//	Flush Logger messages
					msgCount = pLogger->safeMsgCount();
					if (msgCount > 0)
					{
						pLogger->flushQueue(msgCount);
					}
				}
			}
			if (pDataLogger->isEnabled() == true)
			{
				if (pDataLogger->isInitialized() == true)
				{
					//	Flush Trade Records
					msgCount = pDataLogger->safeMsgCount();
					if (msgCount > 0)
					{
						pDataLogger->flushQueue(msgCount);
					}
				}
			}
			//	Check DB Manager and restart connection if necessary
			if (pDBManager->isConnected() == false)
			{
				if (g_pStates->DS_DBMANAGER_ST != ATS_DATASERVICE::RESTART_ST)
				{
					// Restart the connection
					g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::RESTART_ST;
				}
			}
			break;

		case ATS_DATASERVICE::START_ST:
			//	DS Manager thread sets START_ST
			g_pStates->DS_UTILITY_ST = ATS_DATASERVICE::INPROGESS_UP1_ST;
			continue;

		case ATS_DATASERVICE::RESTART_ST:
			//	NOOP
			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			// Start dependencies if they exist; then go into RUN_ST

			if (g_pStates->DS_DBMANAGER_ST == ATS_DATASERVICE::IDLE_ST)
			{
				// Start the dependent subsystem
				g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::START_ST;
				continue;
			}
			g_pStates->DS_UTILITY_ST = ATS_DATASERVICE::RUN_ST;
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				pLogger->log_info(L"Utility Manager has started", L"DataServiceUtilityMgrThreads::UtilityManager");
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			// Set state for dependent subsystem(s)
			if (g_pStates->DS_DBMANAGER_ST == ATS_DATASERVICE::RUN_ST || g_pStates->DS_DBMANAGER_ST == ATS_DATASERVICE::DORMANT_ST)
			{
				g_pStates->DS_DBMANAGER_ST = ATS_DATASERVICE::SHUTDOWN_ST;
			}
			g_pStates->DS_UTILITY_ST = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			// All subsystems must be down before setting to DONE_ST
			if (g_pStates->DS_DBMANAGER_ST != ATS_DATASERVICE::DOWN_ST)
			{
				continue;
			}
			if (pLogger->isInitialized() == true)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
					pLogger->log_trace(L"Logger queue management is stopped", L"DataServiceUtilityMgrThreads::UtilityManager");

				//	Empty the queue
				msgCount = pLogger->safeMsgCount();
				if (msgCount > 0)
				{
					pLogger->flushQueue(msgCount);
				}
			}
			g_pStates->DS_UTILITY_ST = ATS_DATASERVICE::DOWN_ST;
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
				pLogger->log_trace(L"Utility manager is stopped", L"DataServiceUtilityMgrThreads::UtilityManager");
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

	return(rc);
}
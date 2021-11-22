/////////////////////////////////////////////////////////////////////////////////////
//	DataManager.cpp : Defines code for data management object
//
//
// 
#include "stdafx.h"
#include "DataManager.h"


using namespace ATS_DATASERVICE;


DataManager::DataManager(int& control_signal) : control_signal_raised(control_signal)
{

}

DataManager::~DataManager()
{

}

//	
//	Starts and initializes the primary thread that manages the data collection, data connection and analytics processing subsystems
//	
ATS_CODE DataManager::initialize(int argc, wchar_t* argv[])
{
	ATS_CODE rc = ATS_OK;
	//	Parse command line and process arguments
	CommandLineArgs cl(argc, argv);
	//	TODO: Process args


	//	Setup system configuration properties
	try
	{
		/////////////////////////////////////////////////////////////////////////////
		//	Create the data service globally managed objects and load the Args array
		//	with relevant reference pointers
		/////////////////////////////////////////////////////////////////////////////

		pGlobals = new DS_Globals();

		//	Create the logger object - subsystem started separately
		pLogger = new LoggerX(pGlobals);
		g_pGlobals->Args[ARG_UTIL] = (LPVOID*)pLogger;

		//	Create the logger object - subsystem started separately
		pDataLogger = new DataLoggerX(pGlobals);
		g_pGlobals->Args[ARG_SNAP] = (LPVOID*)pDataLogger;

		//	Data Service data aggregates
		pSDShare = new DS_Shared(pGlobals);
		g_pGlobals->Args[ARG_SDS] = (LPVOID*)pSDShare;

		//	Create the DB Manager object - load DataService configuration data
		pDBManager = new DataManagerDB(pGlobals);
		g_pGlobals->Args[ARG_DBM] = (LPVOID*)pDBManager;

		//	Create the Connector Manager object
		pDataConn = new DataConnector(pGlobals);
		g_pGlobals->Args[ARG_CONN] = (LPVOID*)pDataConn;

		//	Create the File Connector Manager object
		pDataFileConn = new DataFileConnector(pGlobals);
		g_pGlobals->Args[ARG_FILE_CONN] = (LPVOID*)pDataFileConn;

		//	Create the Collector Manager object
		pDataColl = new DataCollector(pGlobals);
		g_pGlobals->Args[ARG_COLL] = (LPVOID*)pDataColl;

		//	Create the Analytics Manager object
		pDataAna = new DataAnalytics(pGlobals);
		g_pGlobals->Args[ARG_ANA] = (LPVOID*)pDataAna;


		//	Initialize subsystem signaling objects
		//	- TODO: Add detail regarding specific events
		//g_pGlobals->event_DS_RDP_EVENT.reset();
		//g_pGlobals->event_DS_PDP_EVENT.reset();
		
		//	Additional initializations
		//pDBManager->initialize(pGlobals);

		//	Set start state
		g_pStates->DS_SYSTEM_ST = START_ST;

	}
	catch (...)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			pLogger->log_error(L"Exception during object creation", L"DataManager::initialize");
		rc = ATS_ERROR;
	}

	//	Data manager & subsystems initialized
	//	- Data Service will switch state to run mode
	return rc;
}


///////////////////////////////////////////////////////////////////////////////
//	Service run loop (Implement as a Windows Service - future)
//
//	The data manager is meant to be implemented as a service.
//
//	All of the subsystem management threads are started from here.
//	-	Logger (utility manager)
//	-	Connector
//	-	Collection Manager
//	-	Analytics Manager
//
//	Each thread manages state control for subordinate subsystems.
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataManager::run(void)
{

	ATS_CODE rc = ATS_OK;

	//
	//	All common data structures WILL have been initialized at this point
	//
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
		pLogger->log_info(L"***ATS Startup***", L"DataManager::run");
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
		pLogger->log_trace(L"Starting subsystem threads", L"DataManager::run");

	//////////////////////////////////////
	//	Start the LOGGER main thread
	//////////////////////////////////////
	g_pGlobals->gTAddr[THRD_UTILITY] = (HANDLE)::_beginthreadex(
		NULL,
		0,
		&DataServiceUtilityMgrThreads::UtilityManager,
		pGlobals, // Args array contains all global data components
		0,
		(unsigned int*)&g_pGlobals->gTIDs[THRD_UTILITY]
	);

	//////////////////////////////////////
	//	Start the LOGGER main thread
	//////////////////////////////////////
	g_pGlobals->gTAddr[THRD_DBMANAGER] = (HANDLE)::_beginthreadex(
		NULL,
		0,
		&DataManagerDBMgrThreads::DBManager,
		pGlobals, 
		0,
		(unsigned int*)&g_pGlobals->gTIDs[THRD_DBMANAGER]
	);


	//////////////////////////////////////
	//	Start COLLECTOR main thread
	//	- The DataCollection main thread 
	//	  will start the Collector worker 
	//	  threads
	//////////////////////////////////////
	g_pGlobals->gTAddr[THRD_COLLECTION] = (HANDLE)::_beginthreadex(
		NULL,
		0,
		&DataCollectorThreads::CollectorManager,
		pGlobals,
		0,
		(unsigned int*)&g_pGlobals->gTIDs[THRD_COLLECTION]
	);


	//////////////////////////////////////
	//	Start CONNECTOR main thread
	//////////////////////////////////////
	g_pGlobals->gTAddr[THRD_CONNECTOR] = (HANDLE)::_beginthreadex(
		NULL,
		0,
		&DataConnectorThreads::ConnectorManager,
		pGlobals, // Send pGlobals
		0,
		(unsigned int*)&g_pGlobals->gTIDs[THRD_CONNECTOR]
	);

	//////////////////////////////////////
	//	Start CONNECTOR main thread
	//////////////////////////////////////
	g_pGlobals->gTAddr[THRD_FILE_CONNECTOR] = (HANDLE)::_beginthreadex(
		NULL,
		0,
		&DataFileConnectorThreads::DataFileConnectorManager,
		pGlobals, // Send pGlobals
		0,
		(unsigned int*)&g_pGlobals->gTIDs[THRD_FILE_CONNECTOR]
	);

	//////////////////////////////////////
	//	Start ANALYTICS thread 
	//	- The Analytics main thread 
	//	  will start the processing worker 
	//	  threads
	//////////////////////////////////////
	g_pGlobals->gTAddr[THRD_ANALYTICS] = (HANDLE)::_beginthreadex(
		NULL,
		0,
		&DataAnalyticsThreads::AnalyticsManager,
		pGlobals,
		0,
		(unsigned int*)&g_pGlobals->gTIDs[THRD_ANALYTICS]
	);



	//////////////////////////////////////////////////////////////////////////////
	//	Primary run loop for service RUN method
	//	- Keep track of threads
	//	- Manage system/subsystem state & orderly termination
	//	- Perform housekeeping routines
	//////////////////////////////////////////////////////////////////////////////
	int sysState;
	int	sleepTimeMS = 1000;

	
	//////////////////////////////////////////////////////////////////////////////
	// Manages the overall SYSTEM state
	//////////////////////////////////////////////////////////////////////////////
	for (;;)
	{
		//	DataManager
		//	Change to wait on multiple objects = TMO + Event --- FIX
		Sleep(sleepTimeMS);

		sysState = g_pStates->DS_SYSTEM_ST;
		switch (sysState)	//	(Primary) Data Manager
		{
		case ATS_DATASERVICE::RUN_ST:
			///////////////////////////////////////////////////////////////////
			//	Perform necessary housekeeping
			///////////////////////////////////////////////////////////////////
			if (control_signal_raised == 1)
			{
				//	System shutdown
				g_pStates->DS_SYSTEM_ST = ATS_DATASERVICE::SHUTDOWN_ST;
			}
			break;

		case ATS_DATASERVICE::START_ST:
			//	TODO: All subsystem objects created
			g_pStates->DS_SYSTEM_ST = ATS_DATASERVICE::INPROGESS_UP1_ST;
			continue;

		case ATS_DATASERVICE::RESTART_ST:
			//	NOOP
			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			//	Start logger
			g_pStates->DS_UTILITY_ST = ATS_DATASERVICE::START_ST;
			g_pStates->DS_SYSTEM_ST = ATS_DATASERVICE::RUN_ST;
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			// Set state for dependent subsystem(s)
			if (g_pStates->DS_UTILITY_ST == ATS_DATASERVICE::RUN_ST || g_pStates->DS_UTILITY_ST == ATS_DATASERVICE::DORMANT_ST)
			{
				g_pStates->DS_UTILITY_ST = ATS_DATASERVICE::SHUTDOWN_ST;
			}
			g_pStates->DS_SYSTEM_ST = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			// All subsystems must be down before setting to DONE_ST
			if (g_pStates->DS_UTILITY_ST == ATS_DATASERVICE::DOWN_ST)
			{
					g_pStates->DS_SYSTEM_ST = ATS_DATASERVICE::DOWN_ST;
			}
			break;

		case ATS_DATASERVICE::DOWN_ST:
			//	Terminate the thread
			_endthreadex(rc);
			break;

		case ATS_DATASERVICE::IDLE_ST:
			//	idempotent
			break;

		case ATS_DATASERVICE::DORMANT_ST:
			//	idempotent
			break;

		default:
			//	NOOP
			break;
		}
	}
}


ATS_CODE DataManager::shutdown(void)
{
	BOOL fFreeResult = FALSE;

	////fFreeResult = FreeLibrary(g_pGlobals->m_hCollectorDLL);

	return ATS_OK;
}




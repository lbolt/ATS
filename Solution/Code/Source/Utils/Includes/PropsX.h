#pragma once

///////////////////////////////////////////////////////////////////////////////
//	System-wide properties
//
//	This file contains classes of global scope, 
//	properties and subsystem state information
//	- Each subsystem has its own set of properties - the following are the 
//	  designations (Everything was bundled into this file for convenience)
//		DS	- Data Service
//		BS	- Blotter Service
//		TSS	- Trading Simulator Service
//		TS	- Trade Station
//
//	Note: Some of these properties (having default values) will come from 
//		  the Data Service configuration file
//
#include "../Includes/CommonX.h"
#include "../Includes/StringX.h"
#include "../Includes/ParseArgsX.h"
#include "../Includes/ThreadMgrX.h"
#include "../Includes/LoggerX.h" 
#include "../Includes/DataLoggerX.h"
//#include "../Includes/StructX.h"		//	<--------------REMOVED and added to dependent .h files
#include "../Includes/SafeSyncX.h"
#include "psapi.h"
#include <process.h>
#include "concrt.h"
#include <ole2.h>
#include <stdio.h>


//	Forward decls
class DS_Globals;
class DS_States;
class DS_Props;

//	Simplified reference
#define g_pGlobals		pGlobals
#define g_pStates		pGlobals->pStates
#define g_pProps		pGlobals->pProps


//	Thread block index
#define	THRD_UTILITY			0
#define	THRD_CONNECTOR			1
#define	THRD_COLLECTION			2
#define	THRD_ANALYTICS			3
#define THRD_DBMANAGER			4
#define THRD_FILE_CONNECTOR		5
#define	THRD_LAST				6
#define THRD_MAX			THRD_LAST


//	Thread argument block (Shared objects)
#define	ARG_UTIL			0	//	Logger calls (LoggerX)
#define ARG_DBM				1	//	Database calls - provides access to stored procedures (DataManagerDB)
#define ARG_COLL			2	//	Data Collector
#define ARG_CONN			3	//	Data Connector
#define ARG_FILE_CONN		4	//	Data File Connector
#define	ARG_ANA				5	//	Data Analytics
#define	ARG_SDS				6	//	Shared data structures - typed upon use (DS_Shared)
#define ARG_SNAP			7	//	Snapshots
#define	ARG_LAST			8	//	Placeholder
#define	ARG_MAX				ARG_LAST

#define	MAX_WORKER_CONTAINER	21	//	Maximum worker thread container size (4-Utility + n-DC + m-DA)
#define	MAX_DCWORKER			1	//	Maximum number of Data Collector worker threads
#define	MAX_DAWORKER			8	//	Maximum number of Data Analytics worker threads


//	Database
#define	MAXATDESCFIELDSIZE	100	//	Maximum provider field size in wide characters




namespace ATS_DATASERVICE {

	enum MSG_BUFFER_WSIZE
	{
		XSMALL		= 64,
		SMALL		= 128,
		MEDIUM		= 256,
		LARGE		= 512,
		XLARGE		= 1024
	};

	enum DS_SYSTEM_STATES
	{
		IDLE_ST=0,
		DORMANT_ST,
		START_ST,
		INPROGESS_UP1_ST,
		INPROGESS_UP2_ST,
		INPROGESS_UP3_ST,
		INPROGESS_UP4_ST,
		INPROGESS_UP5_ST,
		RUN_ST,
		RESTART_ST,
		SHUTDOWN_ST,
		INPROGRESS_DOWN1_ST,
		INPROGRESS_DOWN2_ST,
		INPROGRESS_DOWN3_ST,
		INPROGRESS_DOWN4_ST,
		INPROGRESS_DOWN5_ST,
		DOWN_ST
	};

	enum DataServiceCommands
	{
		CMD_NOOP = 0,
		CMD_START_THREADS,
		CMD_STOP_THREADS
	};

};

using namespace ATS_DATASERVICE;



/////////////////////////////////////////////////////////////////////////////
// Class to hold and operate on properties
//	- Static properties are set one time from config file(s)
//	- Dynamic properties start with values derived from the properties file upon load. Values can be changed via program control.

class DS_States {
public:
	DS_States();
	~DS_States();
	const wchar_t*	moduleName = L"PropX.DS_States";

	int initialize(DS_Globals*);

	/////////////////////////////////////////////////////////////////////////
	//	Subsystem States
	/////////////////////////////////////////////////////////////////////////
	int DS_SYSTEM_ST			= ATS_DATASERVICE::IDLE_ST;
	int DS_COLLECTOR_ST			= ATS_DATASERVICE::IDLE_ST;
	int DS_CONNECTOR_ST			= ATS_DATASERVICE::IDLE_ST;
	int DS_FILE_CONNECTOR_ST	= ATS_DATASERVICE::IDLE_ST;
	int DS_CALLBACK_ST			= ATS_DATASERVICE::IDLE_ST;
	int DS_ANALYTICS_ST			= ATS_DATASERVICE::IDLE_ST;
	int DS_UTILITY_ST			= ATS_DATASERVICE::IDLE_ST;
	int DS_DBMANAGER_ST			= ATS_DATASERVICE::IDLE_ST;
	int DS_DCWORKER_ST			= ATS_DATASERVICE::IDLE_ST;
	int DS_DAWORKER_ST			= ATS_DATASERVICE::IDLE_ST;


	/////////////////////////////////////////////////////////////////////////
	// Static properties
	/////////////////////////////////////////////////////////////////////////




	/////////////////////////////////////////////////////////////////////////
	// Dynamic properties
	/////////////////////////////////////////////////////////////////////////




private:
	DS_States(const DS_States &);


};



/////////////////////////////////////////////////////////////////////////////
// Class to hold and operate on properties
//	- Static properties are set one time from config file(s)
//	- Dynamic properties start with values derived from the properties file upon load. Values can be changed via program control.

class DS_Props {
public:
	DS_Props();
	~DS_Props();
	const wchar_t*	moduleName = L"PropX.DS_Props";

	int initialize(DS_Globals*);

	///////////////BEGIN///////////////
	// Database Manager Properties
	///////////////////////////////////	
	wstring	prop_dbDSN = L"";		//	future use
	wstring	prop_dbDriver = L"";
	wstring	prop_dbServer = L"";
	wstring	prop_dbName = L"";
	wstring	prop_dbUser = L"";
	wstring	prop_dbCred = L"";
	////////////////END////////////////


	///////////////BEGIN///////////////
	// Logger Manager Properties
	///////////////////////////////////
	wstring	prop_logging_enabled = L"";
	wstring	prop_logging_level = L"";
	UINT	prop_log_level = 0;
	wstring	prop_logging_rollover = L"";
	////////////////END////////////////

	///////////////BEGIN///////////////
	// Logger Manager Properties
	///////////////////////////////////
	wstring	prop_snapshot_enabled = L"";
	wstring	prop_snapshot_level = L"";
	wstring	prop_snapshot_rollover = L"";
	////////////////END////////////////

	///////////////BEGIN///////////////
	// Worker Thread Manager Properties
	///////////////////////////////////
	wstring	prop_dc_worker_max = L"";
	wstring	prop_da_worker_max = L"";
	////////////////END////////////////


	///////////////BEGIN///////////////
	// AT Data Connector Properties
	///////////////////////////////////
	wstring	prop_ATenabled = L""; 
	wstring prop_ATDisableRDQInsertion = L"";
	wstring	prop_ATuserid = L"";
	wstring	prop_ATpassword = L"";
	wstring	prop_ATguid_key = L"";
	wstring	prop_ATprimary_server = L"";
	wstring	prop_ATsecondary_server = L"";
	wstring	prop_ATserver_port = L"";
	wstring	prop_ATuse_internal_queue = L"";
	wstring prop_ATCallbackCycleDelay = L"";
	wstring prop_ATTrackStatsForSymbol = L"";
	////////////////END////////////////

	///////////////BEGIN///////////////
	// Data File Connector Properties
	///////////////////////////////////
	wstring	prop_DataFile_enabled = L"";
	wstring prop_DataFile_DisableRDQInsertion = L"";
	wstring	prop_DataFile_snapshot_name = L"";
	wstring	prop_DataFile_insert_timestamp = L"";
	wstring	prop_DataFile_frequency = L"";
	wstring	prop_DataFile_trade_records_per_cycle = L"";
	////////////////END////////////////

	///////////////BEGIN///////////////
	// Queue Manager Properties
	///////////////////////////////////
	wstring	prop_QM1 = L"";
	wstring	prop_QM2 = L"";
	////////////////END////////////////


	///////////////BEGIN///////////////
	// StructX Properties
	///////////////////////////////////
	wstring	prop_SYMBOL_MAXSIZE = L"";					//	Maximum size for symbol - needs to be setup in some sort of commons
	wstring	prop_DATA_COMPRESSION_ENABLED = L"";		//	Process data compression
	wstring	prop_DATA_GAP_ENABLED = L"";				//	Process data gaps
	wstring	prop_OUTLIER_COLLECTION_SIZE = L"";			//	Default number of ticks required for collection to be considered an out-lier
	wstring	prop_MAX_OUTLIER_DELTA = L"";				//	Percentage above last trade value to identify trade value as a potential outlier
	wstring	prop_SUPERTICK_SIZE = L"";					//	Default number of ticks in a SuperTick
	wstring	prop_MIN_SLOTS_FOR_TREND_CALCS = L"";		//	Minimum slots required to perform trend calculations
	wstring	prop_NUM_SLOTS_TO_ESTABLISH_TREND = L"";	//	Number of slots required in window during ESTABLISH phase to identify a trade worthy trend
	wstring	prop_MAX_SLOTS_ON_FOLLOW = L"";				//	Maximum slots allowed in processing window during trend FOLLOW phase 
	wstring	prop_ACTIVE_DS_TRIM_SIZE = L"";				//	Default buffer window size for Active DS container based on 25m daily tick average; yields ~5 minute window
	////////////////END////////////////

	///////////////BEGIN///////////////
	// AnalyticX Properties
	///////////////////////////////////
	wstring	prop_TREND_UPPER_GRADIENT = L"";			//	Trend slope range upper bound
	wstring	prop_TREND_LOWER_GRADIENT = L"";			//	Trend slope range lower bound	
	wstring	prop_CHANNEL_COMPRESS_RANGE_LIMIT = L"";	//	Compress range limit multiplier as a percentage in decimal form (.01 is 1 %)
	wstring	prop_CHANNEL_GAP_RANGE_LIMIT = L"";			//	Gap range limit multiplier as a percentage in decimal form (.01 is 1 %)
	wstring	prop_BASIS_POINT_ADJ = L"";					//	Basis point adjustment value (.0001 equals 1 BP)
	////////////////END////////////////

	///////////////BEGIN///////////////
	// NumericX Properties
	///////////////////////////////////
	wstring	prop_TREND_AGGR_SIZE_LIMIT = L"";			//	Starting aggregate size
	////////////////END////////////////

private:
	DS_Props(const DS_Props &);


};


/////////////////////////////////////////////////////////////////////////////
//	This class holds all globally exposed data in a single container
//	

class DS_Globals {
public:
	DS_Globals();
	~DS_Globals();

	const wchar_t*	moduleName = L"PropX.DS_Globals";
	const wchar_t*	serviceName = L"DataService";

	/////////////////////////////////////////////////////////////////////////
	//	Global objects
	//	- Pointers for Props and States are created here
	//	- LoggerX gets created by the DataServiceLoggerMgr.
	//	  Pointers are stored in Args below (includes DS_Globals and LoggerX)
	/////////////////////////////////////////////////////////////////////////
	DS_Props*	pProps;
	DS_States*	pStates;

	//	Picked-up from EXE module base path
	wstring	moduleBasePath;
	wstring	logFilePath;
	wstring	configFilePath;
	wstring	snapshotFilePath;
	wstring	dllCollectorFilePath;
	wstring	dllAnalyticsFilePath;

	ATS_CODE initialize(void);
	ATS_CODE getModuleBasePathW(const wchar_t*);



	//	Events
	//	- All DataService events defined here
	//Concurrency::event event_DS_RDP_EVENT;	//	Raw data packet event
	//Concurrency::event event_DS_PDP_EVENT;	//	Processed data packet event



	///////////////////////////////////////////////////////////////////////////
	//	Thread Management
	///////////////////////////////////////////////////////////////////////////
	//	Subsystem Thread Block
	//	- All thread address handles and IDs are stored here
	///////////////////////////////////////////////////////////////////////////

	//	Primary Threads
	HANDLE	gTAddr[THRD_MAX];
	DWORD*	gTIDs[THRD_MAX];

	//	Data Collector Worker Threads
	HANDLE	gDCTAddr[MAX_WORKER_CONTAINER];
	DWORD	gDCTIDs[MAX_WORKER_CONTAINER];
	DS_SYSTEM_STATES gDCTState[MAX_WORKER_CONTAINER];
	int		DC_THRD_COUNT;	//	Current collector worker thread count

	//	Data Analytics Worker Threads
	HANDLE	gDATAddr[MAX_WORKER_CONTAINER];
	DWORD	gDATIDs[MAX_WORKER_CONTAINER];
	DS_SYSTEM_STATES gDATState[MAX_WORKER_CONTAINER];
	int		DA_THRD_COUNT;	//	Current analytic worker thread count

	//	Thread related functions
	//	Data Collection
	ATS_CODE getNextDCThreadIndex(int&);
	ATS_CODE incrDCThreadIndexCount(void);
	ATS_CODE convertDCThreadIDToIndex(DWORD, int&);
	ATS_CODE clearDCThreadIndex(int&);
	bool workerDCThreadsIndexFull(void);
	//	Data Analytics
	ATS_CODE getNextDAThreadIndex(int&);
	ATS_CODE incrDAThreadIndexCount(void);
	ATS_CODE convertDAThreadIDToIndex(DWORD, int&);
	ATS_CODE clearDAThreadIndex(int&);
	bool workerDAThreadsIndexFull(void);



	//	Args Array
	//	- Container holding addresses for system-wide global objects
	//	- All subsystems are maintained in threads and each engine is instantiated
	//		as an object. This array holds object addresses.
	LPVOID* Args[ARG_MAX];


	///////////////////////////////////////////////////////////////////////////
	//	Synchronization Objects
	///////////////////////////////////////////////////////////////////////////
	//	Safe Sync objects for Data Collection and Data Analytics thread management
	AutoCriticalSection DCWORKER_sso;
	AutoCriticalSection DAWORKER_sso;

#ifdef _UNUSED_CODE_
	//	Safe Sync object for watch list block
	CS_Sync_External* WATCH_LIST_DATA_sso;

	//	Safe Sync object for watch list notifications queue
	CS_Sync_External* WATCH_LIST_NOTIFY_QUEUE_sso;
#endif _UNUSED_CODE_


private:
	DS_Globals(const DS_Globals &);
	DS_Globals& operator=(const DS_Globals &);

};









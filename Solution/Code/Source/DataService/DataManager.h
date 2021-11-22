/////////////////////////////////////////////////////////////////////////////////////
//
//
//
#pragma once
#include "CommonX.h"
#include "PropsX.h"
#include "DataManagerDB.h"
#include "DataCollector.h"
#include "DataConnector.h"
#include "DataFileConnector.h"
#include "DataAnalytics.h"
#include "DS_Shared.h"


class DataManager
{
public:
	// Declare public methods and objects
	DataManager(int&);
	~DataManager();
	const wchar_t*	moduleName = L"DataManager";
	int& control_signal_raised;

	
	ATS_CODE initialize(int argc, wchar_t* argv[]);
	ATS_CODE run(void);
	ATS_CODE shutdown(void);

	//	Managed objects
	DS_Globals*			pGlobals;
	DS_Shared*			pSDShare;
	LoggerX*			pLogger;
	DataManagerDB*		pDBManager;
	DataCollector*		pDataColl;
	DataConnector*		pDataConn;
	DataFileConnector*	pDataFileConn;
	DataAnalytics*		pDataAna;
	DataLoggerX*		pDataLogger;


	//	Data Service data aggregates (reference leveraging DS_Shared)
	WatchPool*	pWatchPoolList;




private:
	// Declare private methods and objects
	DataManager();
	DataManager(const DataManager &);
	DataManager& operator=(const DataManager&);


};

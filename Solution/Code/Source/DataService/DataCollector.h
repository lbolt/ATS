#pragma once
//
#include "CommonX.h"
#include "PropsX.h"
#include "../Includes/StructX.h"	//<----------------ADDED
#include "DS_Shared.h"
#include "DataConnector.h"
#include "DataFileConnector.h"
#include "CircularBuffer.h"



class DataCollector
{
public:
	DataCollector(void*);
	~DataCollector();
	const wchar_t*	moduleName = L"DataCollector";


	int initialize(void*);
	int manageWorkers(int);
	bool isCollectionEnabled();
	bool workersInitialized();

	//	Reference pointers to global objects
	DS_Globals* pGlobals;
	LoggerX*	pLogger;
	DS_Shared*	pSDShare;
	DataConnector* pDataConn;
	DataFileConnector* pDataFileConn;

private:
	// Declare private methods and objects
	DataCollector();
	DataCollector(const DataCollector &);
	DataCollector& operator=(const DataCollector&);
	bool bCollectionEnabled;
	bool bWorkersInitialized;

};


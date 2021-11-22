#pragma once

#include "DataCollector.h"
//#include "../Utils/Includes/CommonX.h"
//#include "../Utils/Includes/PropsX.h"
//#include "DS_Shared.h"



class DataCollectorWorker
{
public:
	DataCollectorWorker(void*);
	~DataCollectorWorker();
	const wchar_t*	moduleName = L"DataCollectorWorker";

	ATS_CODE initialize(void*);
	ATS_CODE processRDQEvent(void);
	ATS_CODE processAnalyticsNotification(int);

	//	References to global object
	DS_Globals* pGlobals;
	LoggerX*	pLogger;
	DS_Shared*	pSDShare;

	//	Managed objects
	DataCollectorWorker* pDCWorker;


private:
	// Declare private methods and objects
	DataCollectorWorker();
	DataCollectorWorker(const DataCollectorWorker &);
	DataCollectorWorker& operator=(const DataCollectorWorker&);
	

};


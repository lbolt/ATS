#pragma once

#include "DataAnalytics.h"
//#include "../Utils/Includes/CommonX.h"
//#include "../Utils/Includes/PropsX.h"
//#include "DS_Shared.h"



class DataAnalyticsWorker
{
public:
	DataAnalyticsWorker(void*);
	~DataAnalyticsWorker();
	const wchar_t*	moduleName = L"DataAnalyticsWorker";

	ATS_CODE initialize(void*);
	ATS_CODE processNRQEvent(void);
	ATS_CODE getNRQSeq(unsigned int&, unsigned int&);
	ATS_CODE ungetNRQSeq(unsigned int&, unsigned int&);
	ATS_CODE processOutlierFilter(void);


	//	References to global object
	DS_Globals* pGlobals;
	LoggerX*	pLogger;
	DS_Shared*	pSDShare;

	//	Managed objects
	DataAnalyticsWorker* pDAWorker;


private:
	// Declare private methods and objects
	DataAnalyticsWorker();
	DataAnalyticsWorker(const DataAnalyticsWorker &);
	DataAnalyticsWorker& operator=(const DataAnalyticsWorker&);
};


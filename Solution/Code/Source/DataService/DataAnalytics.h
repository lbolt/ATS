#pragma once
//
#include "CommonX.h"
#include "PropsX.h"
#include "../Includes/StructX.h"	//	<---------------ADDED
#include "DS_Shared.h"
#include "CircularBuffer.h"


enum ATS_ALGO_ID
{
	ATS_LINEAR_TREND_ID = 0,
	ATS_LINEAR_TREND_WITH_SMOOTHING_ID,
	ATS_LINEAR_TREND_WITH_AI_SMOOTHING_ID
};


//	Forward declarations for Analytic Functions 


///////////////////////////////////////////////////////////////////////////////
//	Common Data Analytics Object
///////////////////////////////////////////////////////////////////////////////
class DataAnalytics
{
public:
	DataAnalytics(void*);
	~DataAnalytics();
	const wchar_t*	moduleName = L"DataAnalytics";


	int initialize(void*);
	int manageWorkers(int);
	bool isAnalyticsEnabled();
	bool workersInitialized();


	//	References to global objects
	DS_Globals* pGlobals;
	LoggerX*	pLogger;
	DS_Shared*	pSDShare;

	//	Define ATS Analytics



private:
	// Declare private methods and objects
	DataAnalytics();
	DataAnalytics(const DataAnalytics &);
	DataAnalytics& operator=(const DataAnalytics&);

	bool bAnalyticsEnabled;
	bool bWorkersInitialized;
};


#pragma once

#include "../Includes/CommonX.h"
#include "../Includes/PropsX.h"
#include "../Includes/StructX.h"
#include "../Includes/NumnericX.h"



///////////////////////////////////////////////////////////////////////////////
//	AnalyticsX
//
//	Contains processing routines used by analytic Handlers
//	- All low-level MATH routines are located in the NumericX library
//
//	Processes:
//		- X-Value Series Derivation
//		- Data Series Linear Trend (Slope calculation)
//		- Linear Trend Prediction
//	
//	Base Math Library:
//		- Relies on standard C++ math library and 64-bit floating point
//
//	Speed-up:
//		- Off-load Analytic processes to a GPU subsystem (future consideration)
//
//
//	- Analytics list:
//
//		LTLS Analysis		- Linear Trend Analysis
//		ETLS Analysis		- Exponential Trend Analysis
//
///////////////////////////////////////////////////////////////////////////////



//	Forward Declarations




///////////////////////////////////////////////////////
//	AnalyticX 
//	- Base Class for all algorithms
///////////////////////////////////////////////////////
class AnalyticX
{
public:
	AnalyticX();
	AnalyticX(void*);
	~AnalyticX();

	static DS_Globals* pGlobals;	//	Set explicitly upon new object creation; does not inherit from CoreAnalyticCtx : StructX
	static NumnericX*	pNELib;		//	Numeric engine lib


	virtual ATS_CODE establishTrend(DataAggregateContainer*);
	virtual ATS_CODE followTrend(DataAggregateContainer*);


private:

	AutoCriticalSection	AnalyticX_cs;

	static int isInitialized;
	static int getObjectState();
	static int setObjectState();

	ATS_CODE initialize(void*);

};


///////////////////////////////////////////////////////
//	LTLS Analytic Processing Routines (Interface)
//	- Linear Trend Analysis
///////////////////////////////////////////////////////
class LTLS_Analysis : public AnalyticX
{
public:
	LTLS_Analysis(void* Args);
	~LTLS_Analysis();

	ATS_CODE processTrend(DataAggregateContainer*);
	ATS_CODE adjustTrendYDataValues(DataAggregateContainer*, double&);


private:
	LTLS_Analysis();
	LTLS_Analysis(const LTLS_Analysis &);
	LTLS_Analysis& operator=(const LTLS_Analysis&);

	ATS_CODE initialize(void*);
};


///////////////////////////////////////////////////////
//	ETLS Analytic Processing Routines
//	- Exponential Trend Analysis
///////////////////////////////////////////////////////
class ETLS_Analysis : AnalyticX
{
public:
	ETLS_Analysis();
	~ETLS_Analysis();

	ATS_CODE establishTrend(DataAggregateContainer*);
	ATS_CODE adjustTrendYDataValues(DataAggregateContainer*);

private:
	ETLS_Analysis(const ETLS_Analysis &);
	ETLS_Analysis& operator=(const ETLS_Analysis&);

	ATS_CODE initialize(void);

};





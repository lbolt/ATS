#include "stdafx.h"
#include "DS_Shared.h"


//	Forward decls




DS_Shared::DS_Shared(void* Args)
{
	int rc = initialize(Args);
}


DS_Shared::~DS_Shared()
{
		//	TODO - delete new objects

}

///////////////////////////////////////////////////////////////////////////////
//	Performs single instance initialization for common structures for both 
//	Data Collection and Data Analytics
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DS_Shared::initialize(void* Args)
{
	try {
		///////////////////////////////////////////////////
		///////////////////////////////////////////////////
		//	++ DS_Shared Object Creation
		///////////////////////////////////////////////////
		///////////////////////////////////////////////////

		if (isInitialized() == true)
			return ATS_C_SUCCESS;

		pGlobals = (DS_Globals*)Args;
		initState = true;

		systemStartTimePoint = std::chrono::steady_clock::now();
		systemStartWallTime = std::chrono::system_clock::now();
		lastRDQCycleTime = systemStartWallTime;
		lastRDQCycleCount = 0;

		///////////////////////////////////////////////////
		//	Create primary Data Collection objects
		///////////////////////////////////////////////////
		pWatchPool = new WatchPool();
		pSequenceLookupMap = new WatchPoolItemSequence();
		pActiveCTXBlock = new DS_Active_CTX_Block();
		pRDQ = new CircularBuffer<TRADE>(100);
		pPredBusyHash = new PredecessorBusyHash();

		///////////////////////////////////////////////////
		//	Create primary Data Analytics objects
		///////////////////////////////////////////////////
		pActiveAnalyticCtxBlock = new ActiveAnalyticCtxBlock(Args);
		pNRQ = new CircularBuffer<int>(500); 
		pNRQBusyHash = new NextReadyQBusyHash();
		insertionCountRDQ = 0;



		///////////////////////////////////////////////////
		//	Shared Notification Events
		///////////////////////////////////////////////////
		hPostCollectionRDQEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
		hPostAnalyticsNRQEvent = CreateEventW(NULL, FALSE, FALSE, NULL);


		///////////////////////////////////////////////////
		///////////////////////////////////////////////////
		//	-- DS_Shared Object Creation
		///////////////////////////////////////////////////
		///////////////////////////////////////////////////

	}
	catch (...)
	{
		// Add logging code

	}

	return ATS_OK;
}



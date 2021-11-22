#pragma once

#include "../Utils/Includes/CommonX.h"
#include "../Utils/Includes/PropsX.h"
#include "../Includes/StructX.h"	//	<-------------ADDED
#include "DataSecurityStructs.h"

using namespace chrono;


///////////////////////////////////////////////////////////////////////////////
//	DS_Shared
//	- Contains Data Service common object definitions for all collection 
//	  data aggregates. Common to both Collection and Analytics subsystems.
//
//	- DataManager owns the lifecycle of the DS_Shared object
//
//	NOTE: This file should not include class declarations other than DS_Shared
//			(See StructX for data and message declarations)
//
class DS_Shared
{
public:
	DS_Shared(void*);
	~DS_Shared();

	CRITICAL_SECTION* DS_Shared_sso;
	ATS_CODE initialize(void*);

	//////////////////////////////////////////////////////////////////////////////
	//	Shared data object lifecycle definitions
	//////////////////////////////////////////////////////////////////////////////

	//	Data Collection - Watch Pool Components
	WatchPool*						pWatchPool;				// Complete Map of securities acquired from database
	DS_Active_CTX_Block*			pActiveCTXBlock;		// Map of active data collection contexts
	WatchPoolItemSequence*			pSequenceLookupMap;		// Symbol to Sequence number mapping table
	CircularBuffer<TRADE>*			pRDQ;					// ***RDQ (Raw Data Queue)
	unsigned int					insertionCountRDQ;		// Keeps track of total insertions
	PredecessorBusyHash*			pPredBusyHash;			// Predecessors Hash

	//	Data Analytics - Processing Components	<----------------------------------------------------- BUILDING THIS NOW
	ActiveAnalyticCtxBlock*			pActiveAnalyticCtxBlock;// Map of active analytic contexts
	CircularBuffer<int>*			pNRQ;					// ***NRQ (Next Ready Queue)
	NextReadyQBusyHash*				pNRQBusyHash;			// NextReadyQ Busy Hash

	//////////////////////////////////////////////////////////////////////////////
	//	++Statistics and time related
	//////////////////////////////////////////////////////////////////////////////
	chrono::time_point< chrono::steady_clock, duration<double> > systemStartTimePoint;	//	Begin epoch reference time_point for calculations
	chrono::time_point< chrono::system_clock, duration<double> > systemStartWallTime;		//	Begin epoch wall time
	chrono::time_point< chrono::system_clock, duration<double> > providerConnectionTime;	//	Connection wall time
	chrono::time_point< chrono::system_clock, duration<double> > lastRDQCycleTime;		//	Last RDQ insertion count cycle time (enable DS_LOG_LEVEL::DSL_STATS)

	int	lastRDQCycleCount;

	//////////////////////////////////////////////////////////////////////////////
	//	--Statistics
	//////////////////////////////////////////////////////////////////////////////


	//	Events for Notifications
	HANDLE		hPostCollectionRDQEvent;
	HANDLE		hPostAnalyticsNRQEvent;



private:
	// Declare private methods and objects
	DS_Shared();
	DS_Shared(const DS_Shared &);
	DS_Shared& operator=(const DS_Shared&);

	DS_Globals* pGlobals=NULL;
	bool isInitialized() {return initState;}
	bool initState = false;


};


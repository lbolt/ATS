#include "stdafx.h"
#include "DataAnalyticsWorker.h"



///////////////////////////////////////////////////////////////////////////////////////////
//	Runnable thread
//	- Data collection WORKER processing thread(s)
//	- Performs housekeeping operations on the raw data queue 
///////////////////////////////////////////////////////////////////////////////////////////
unsigned DataAnalyticsThreads::AnalyticsWorker(void* Args)
{

	//
	//	Process incoming trade events
	//
	ATS_CODE rc = ATS_OK;
	int subSysState;
	int sleepTimeMS = 500;

	DS_Globals* pGlobals = (DS_Globals*)Args;
	LoggerX* pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	DS_Shared* pSDShare = (DS_Shared*)pGlobals->Args[ARG_SDS];

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Main thread loop
	//	- Process outstanding collection objects on the next ready queue
	//	- 
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	DWORD tid;
	DWORD status_wait;
	int tIndex;
	wchar_t logMsg[LOGGER_MSG_MEDIUMBUF];
	tid = GetCurrentThreadId(); // Each worker thread maintains separate state
	rc = g_pGlobals->convertDAThreadIDToIndex(tid, tIndex);
	//	Check return ...............

	//	Create a data collection worker object for this thread
	DataAnalyticsWorker* pDAWorker = new DataAnalyticsWorker(pGlobals);

	for (;;)
	{
		//	AnalyticsWorker
		//	Waitloop: wait on dependent subsystem ready + tmo
		status_wait = WaitForSingleObject(pSDShare->hPostAnalyticsNRQEvent, sleepTimeMS);
		if (status_wait == WAIT_TIMEOUT)
		{
			//	Do housekeeping if required;
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Analytics worker WaitForObject returned with (WAIT_TIMEOUT)");
				pLogger->log_trace(logMsg, L"DataAlanyticsThreads::AnalyticsWorker");
			}
		}
		else if (status_wait == WAIT_OBJECT_0)																			//<--------------- TEST (notification wakeup)
		{
			//	Object is signaled, do processing
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Analytics worker WaitForObject returned with (WAIT_OBJECT_0)");
				pLogger->log_trace(logMsg, L"DataAlanyticsThreads::AnalyticsWorker");
			}
		}
		else
		{
			//	Report something went wrong
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Analytics worker WaitForObject returned error (%d)", GetLastError());
				pLogger->log_trace(logMsg, L"DataAlanyticsThreads::AnalyticsWorker");
			}
		}

		subSysState = g_pGlobals->gDATState[tIndex];

		switch (subSysState)	//	Analytics Worker
		{

		case ATS_DATASERVICE::RUN_ST:
			///////////////////////////////////////////////////////////////////
			//	Perform necessary housekeeping
			///////////////////////////////////////////////////////////////////
			//	Cycle next ready event (NRE) to process
			for (;;)
			{
				ATS_CODE rc;
				rc = pDAWorker->processNRQEvent();
				if (rc == ATS_C_SUCCESS)
				{
					continue;
				}
				else if (rc == ATS_C_EMPTY || rc == ATS_C_NONEXIST)
				{
					break;
				}
				else
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
					{
						swprintf(logMsg, sizeof logMsg, L"Data Analytics worker thread returned error (%d) while processing DS", rc);
						pLogger->log_trace(logMsg, L"DataAnalyticsThreads::AnalyticsWorker");
					}
					break;
				}
			}
			break;

		case ATS_DATASERVICE::START_ST:
			//	No dependencies
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Analytics worker thread (%d) at index (%d) set to RUN_STATE", tid, tIndex);
				pLogger->log_trace(logMsg, L"DataAnalyticsThreads::AnalyticsWorker");
			}
			g_pGlobals->gDATState[tIndex] = ATS_DATASERVICE::INPROGESS_UP1_ST;
			break;

		case ATS_DATASERVICE::RESTART_ST:
			//	Not implemented
			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			//	Next state
			g_pGlobals->gDATState[tIndex] = ATS_DATASERVICE::RUN_ST;
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			// No dependencies
			g_pGlobals->gDATState[tIndex] = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			// All subsystems must be down before setting to DONE_ST

			g_pGlobals->gDATState[tIndex] = ATS_DATASERVICE::DOWN_ST;
			break;

		case ATS_DATASERVICE::DOWN_ST:
			//	Log shutdown message && exit the thread
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Analytics worker thread (%d) at index (%d) has stopped", tid, tIndex);
				pLogger->log_info(logMsg, L"DataAnalyticsThreads::AnalyticsWorker");
			}
			//	Clear this threads slot, then exit
			g_pGlobals->clearDAThreadIndex(tIndex);
			_endthreadex(rc);
			break;

		case ATS_DATASERVICE::IDLE_ST:
			//	
			g_pGlobals->gDATState[tIndex] = ATS_DATASERVICE::START_ST;
			break;

		case ATS_DATASERVICE::DORMANT_ST:
			//	Idempotent
			break;

		default:
			//	NOOP
			break;
		}
	}

	// Set RDP state to RDP_DOWN
	return(ATS_OK);
}




//	CTOR
DataAnalyticsWorker::DataAnalyticsWorker(void* Args)
{
	ATS_CODE rc = initialize(Args);
}


//	DTOR
DataAnalyticsWorker::~DataAnalyticsWorker()
{
}


ATS_CODE DataAnalyticsWorker::initialize(void* Args)
{
	//	Process Args input parameters
	//	- Objects are declared in PropsX::DS_globals
	//	- DS_Globals is created in PropsX
	//	- LoggerX is created in DataManager
	pGlobals = (DS_Globals*)Args;
	pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	pSDShare = (DS_Shared*)pGlobals->Args[ARG_SDS];


	return ATS_OK;
}


///////////////////////////////////////////////////////////////////////////////
//	Process Next ReadyQ Event
//
//	Process next ready Analytic Data Structure (DS)
//	- Iterates through list of Active Contexts for next event sequence#
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataAnalyticsWorker::processNRQEvent(void)
{
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
		pLogger->log_func_entry(L"Process next ready Analytic Data Structure", L"DataAnalyticsWorker::processNRQEvent");

	ATS_CODE rc = ATS_C_SUCCESS;
	unsigned int readySeq;
	unsigned int nrqUid;

	///////////////////////////////////////////////////////
	//	Part I
	//	- Get Next ReadyQ Event (Seq #)
	///////////////////////////////////////////////////////

	//	Lock NRQ Buffer + NRQ Busy Hash
	pSDShare->pNRQ->buffer_lock();
	pSDShare->pNRQBusyHash->nextReadyQBusyHash_sso.lock();

	rc = getNRQSeq(readySeq, nrqUid);																	//<---------------TEST (On multiple events)

	if (rc == ATS_C_UNKNOWN)
	{
		// Exception occurred
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			pLogger->log_warn(L"Exception occurred while processing NRQ Event", L"DataAnalyticsWorker::processNRQEvent");
		// Reset NRQ Buffer and Hash status
		ungetNRQSeq(readySeq, nrqUid);
	}

	//	Unlock NRQ Busy Hash + NRQ Buffer
	pSDShare->pNRQBusyHash->nextReadyQBusyHash_sso.release();
	pSDShare->pNRQ->buffer_release();

	if (rc != ATS_C_FOUND)
	{
		return rc;
	}

	///////////////////////////////////////////////////////
	//	Part II
	//	- Iterate through all registered Analytics Handlers
	//	  and execute processes for each Active Context
	///////////////////////////////////////////////////////

	try
	{
		//	Get the active context list<> for this readySeq#
		//	Create the Queue that gets a copy of the list
		//	Lookup readySeq in map<unsigned int readySeq, ActiveAnalyticCtx*> which holds the list<CoreAnalyticCtx*>
		//	Copy the List<> of registered handlers to the Queue<>
		std::map<unsigned int, ActiveAnalyticCtx*>* pMap = &pSDShare->pActiveAnalyticCtxBlock->activeAnalyticCtxContainer;
		std::map<unsigned int, ActiveAnalyticCtx*>::iterator itMap;
		itMap = pMap->find(readySeq);
		ActiveAnalyticCtx* pList = (ActiveAnalyticCtx*)itMap->second;
		std::queue< CoreAnalyticCtx*> analyticCtxQueue;

		//	++Lock Active Analytic Context
		pList->ActveAnalyticCtx_sso.lock();
		//	Copy lists for this active context to Q
		pList->copyListtoQ(pList->activeAnalyticCtxList, analyticCtxQueue);

		//	Invoke Registered Analytics
		while (!analyticCtxQueue.empty())
		{
			CoreAnalyticCtx* pActiveAnalyticCtx = analyticCtxQueue.front();
			//	Invoke Analytic Handlers for this sequence# 
			//	- Provide db_seq for event symbol and source DS for newly added event data
			rc = pActiveAnalyticCtx->invokeHandlers(readySeq, pSDShare->pActiveCTXBlock->active_CTX_Container[readySeq]);		//<---------------Invokes next analyticX handler)

			// Test rc for SUCCESS <--------- TODO

			analyticCtxQueue.pop();
		}

		//	--Unlock Active Context
		pList->ActveAnalyticCtx_sso.release();
	}
	catch (...)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			pLogger->log_warn(L"Exception occurred during Analytics Handler processing", L"DataAnalyticsWorker::processNRQEvent");
	}


	///////////////////////////////////////////////////////
	//	Part III
	//	- Mandatory release Clean-up for NRQ Buffer 
	//	  and NRQ Hash 
	///////////////////////////////////////////////////////

	//	Lock NRQ Buffer + NRQ Busy Hash
	pSDShare->pNRQ->buffer_lock();
	pSDShare->pNRQBusyHash->nextReadyQBusyHash_sso.lock();

	//	Invalidate NRQ Buffer entry; Unset NRQ Hash Busy flag; Decrement Hash count
	pSDShare->pNRQ->invalidate_entry(nrqUid);
	pSDShare->pNRQBusyHash->set_state(readySeq, NRQ_ELEMENT_BUSY_FLAG::NOT_BUSY);
	pSDShare->pNRQBusyHash->decr_count(readySeq);

	//	Unlock NRQ Busy Hash + NRQ Buffer
	pSDShare->pNRQBusyHash->nextReadyQBusyHash_sso.release();
	pSDShare->pNRQ->buffer_release();

	return rc; // Review at adjust rc at end
}


///////////////////////////////////////////////////////////////////////////////
//	Get Next Available NRQ Sequence# and Buffer uid
//	- Returns either FOUND, EMPTY or UNKNOWN
//
//	Note: Assumes locking occurs prior to call to this function
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataAnalyticsWorker::getNRQSeq(unsigned int& seq, unsigned int& uid)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	Container_forward_iterator it, it_end;
	CONTAINER_CTX_BLOCK<int>* ctx = NULL;
	NRQ_ELEMENT nrq_elem;
	bool flag = false;

	try
	{
		it = pSDShare->pNRQ->begin();
		it_end = pSDShare->pNRQ->end();
		rc = ATS_C_EMPTY;
		// Find "next ready" Event on NTQ
		while (it != it_end)
		{
			// Convert "it" to "container" element context
			pSDShare->pNRQ->convert_forward_iterator_to_addr(it, ctx);
			// Check "hash" element for BUSY
			pSDShare->pNRQBusyHash->get_value(ctx->element, nrq_elem);
			if (nrq_elem.flag != NRQ_ELEMENT_BUSY_FLAG::BUSY)
			{
				// Found a suitable one - set buffer element status BUSY
				ctx->state = CIRCULAR_BUFFER_STATES::BUSY;
				// Provide feedback to caller
				seq = ctx->element;
				uid = ctx->uid;
				// Set "hash" element value flag to BUSY
				pSDShare->pNRQBusyHash->set_state(ctx->element, NRQ_ELEMENT_BUSY_FLAG::BUSY);
				rc = ATS_C_FOUND;
				break;
			}
			// Try next
			it = pSDShare->pNRQ->container_iterator_incr(it);
		}
	}
	catch (...)
	{
		// Unknown exception occurred
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			pLogger->log_warn(L"Encountered an unknown exception", L"DataAnalyticsWorker::getNRQSeq");
		// Invalidate buffer element allowing it to be removed


		rc = ATS_C_UNKNOWN;
	}

	return rc;
}


///////////////////////////////////////////////////////////////////////////////
//	Reset NRQ Sequence# and Buffer states 
//	- Returns SUCCESS upon completion
//
//	Note: Assumes locking occurs prior to call to this function
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataAnalyticsWorker::ungetNRQSeq(unsigned int& seq, unsigned int& uid)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	Container_forward_iterator it;
	CONTAINER_CTX_BLOCK<int>* ctx = NULL;

	try
	{
		// Get NRQ buffer element address
		rc = pSDShare->pNRQ->find_element_by_uid(uid, it);
		if (rc == ATS_C_FOUND)
		{
			rc = pSDShare->pNRQ->convert_forward_iterator_to_addr(it, ctx);
			if (rc == ATS_C_SUCCESS)
			{
				ctx->state = CIRCULAR_BUFFER_STATES::NEW;
			}
		}
		pSDShare->pNRQBusyHash->set_state(seq, NRQ_ELEMENT_BUSY_FLAG::NOT_BUSY);
		rc = ATS_C_SUCCESS;
	}
	catch (...)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			pLogger->log_warn(L"Exception occurred while performing unget operation", L"DataAnalyticsWorker::ungetNRQSeq");
		rc = ATS_C_UNKNOWN;
	}
	return rc;
}


///////////////////////////////////////////////////////////////////////////////
//	Processing FILTER
//	- Outlier smoothing filter
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataAnalyticsWorker::processOutlierFilter(void)
{
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
		pLogger->log_func_entry(L"Process Outliers", L"DataAnalyticsWorker::processOutlierFilter");

	ATS_CODE rc = ATS_C_SUCCESS;
	unsigned int readySeq = 0;



	return rc;
}


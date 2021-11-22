#include "stdafx.h"
#include "DataCollectorWorker.h"



///////////////////////////////////////////////////////////////////////////////////////////
//	Runnable thread
//	- Data collection WORKER processing thread(s)
//	- Performs housekeeping operations on the raw data queue 
///////////////////////////////////////////////////////////////////////////////////////////
unsigned DataCollectorThreads::CollectorWorker(void* Args)
{

	//
	//	Process incoming RDQ data inserted via provider callback
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
	rc = g_pGlobals->convertDCThreadIDToIndex(tid, tIndex);
	//	Check return ...

	//	Create a data collection worker object for this thread
	DataCollectorWorker* pDCWorker = new DataCollectorWorker(pGlobals);

	for (;;)
	{
		//	CollectorWorker
		//	Waitloop: wait on dependent subsystem ready + tmo
		status_wait = WaitForSingleObject(pSDShare->hPostCollectionRDQEvent, sleepTimeMS);
		if (status_wait == WAIT_TIMEOUT)
		{
			//	Do housekeeping if required;
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Collector worker WaitForObject returned with (WAIT_TIMEOUT)");
				pLogger->log_trace(logMsg, L"DataCollectorThreads::CollectorWorker");
			}
		}
		else if (status_wait == WAIT_OBJECT_0)
		{
			//	Object is signaled, do processing
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Collector worker WaitForObject returned with (WAIT_OBJECT_0)");
				pLogger->log_trace(logMsg, L"DataCollectorThreads::CollectorWorker");
			}
		}
		else
		{
			//	Report something went wrong
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Collector worker WaitForObject returned error (%d)", GetLastError());
				pLogger->log_trace(logMsg, L"DataCollectorThreads::CollectorWorker");
			}
		}

		subSysState = g_pGlobals->gDCTState[tIndex];		//	<---------------FIX: periodic failure during startup 

		switch (subSysState)	//	Collector Worker
		{

		case ATS_DATASERVICE::RUN_ST:
			///////////////////////////////////////////////////////////////////
			//	Perform necessary housekeeping
			///////////////////////////////////////////////////////////////////
			//	Cycle next raw data packet (RDP) to process
			for (;;)
			{
				ATS_CODE rc;
				rc = pDCWorker->processRDQEvent();
				if (rc == ATS_C_SUCCESS)
				{
					continue;
				}
				else if (rc == ATS_C_EMPTY)
				{
					break;
				}
				else
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
					{
						swprintf(logMsg, sizeof logMsg, L"Data Collector worker thread returned error (%d) while processing RDP", rc);
						pLogger->log_trace(logMsg, L"DataCollectorThreads::CollectorWorker");
					}
					break;
				}
			}
			break;

		case ATS_DATASERVICE::START_ST:
			//	No dependencies
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Collector worker thread (%d) at index (%d) set to RUN_STATE", tid, tIndex);
				pLogger->log_trace(logMsg, L"DataCollectorThreads::CollectorWorker");
			}
			g_pGlobals->gDCTState[tIndex] = ATS_DATASERVICE::INPROGESS_UP1_ST;
			break;

		case ATS_DATASERVICE::RESTART_ST:
			//	Not implemented
			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			//	Next state
			g_pGlobals->gDCTState[tIndex] = ATS_DATASERVICE::RUN_ST;
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			// No dependencies
			g_pGlobals->gDCTState[tIndex] = ATS_DATASERVICE::INPROGRESS_DOWN1_ST;
			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			// All subsystems must be down before setting to DONE_ST

			g_pGlobals->gDCTState[tIndex] = ATS_DATASERVICE::DOWN_ST;
			break;

		case ATS_DATASERVICE::DOWN_ST:
			//	Log shutdown message && exit the thread
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
			{
				swprintf(logMsg, sizeof logMsg, L"Data Collector worker thread (%d) at index (%d) has stopped", tid, tIndex);
				pLogger->log_info(logMsg, L"DataCollectorThreads::CollectorWorker");
			}
			//	Clear this threads slot, then exit
			g_pGlobals->clearDCThreadIndex(tIndex);
			_endthreadex(rc);
			break;

		case ATS_DATASERVICE::IDLE_ST:
			//	
			g_pGlobals->gDCTState[tIndex] = ATS_DATASERVICE::START_ST;
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
DataCollectorWorker::DataCollectorWorker(void* Args)
{
	ATS_CODE rc = initialize(Args);
}


//	DTOR
DataCollectorWorker::~DataCollectorWorker()
{
}


ATS_CODE DataCollectorWorker::initialize(void* Args)
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
//	Process next available Raw Data Packet
//	
//	- No recovery is required due to a processing error 
//	
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataCollectorWorker::processRDQEvent()
{
	ATS_CODE rc = ATS_C_EMPTY;

	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
		pLogger->log_func_entry(L"Entered processRDQEvent", L"DataCollectorWorker::processRDQEvent");

	DWORD tid;
	wchar_t logMsg[LOGGER_MSG_MEDIUMBUF];
	tid = GetCurrentThreadId();
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
	{
		swprintf(logMsg, sizeof logMsg, L"Process next available RDQ data packet - thread (%d)", tid);
		pLogger->log_debug(logMsg, L"DataCollectorWorker::processRDQEvent");
	}

	///////////////////////////////////////////////////////////////////////////
	//	PART I - processNextRDP
	//
	//	- Get the next NEW (non-busy) element from RDQ
	//	- Entry is set from NEW to IN_USE
	///////////////////////////////////////////////////////////////////////////
	Container_forward_iterator it, it_end;
	CONTAINER_CTX_BLOCK<TRADE>* ctx=NULL;
	unsigned int uid=0;
	int db_seq=0;
	TICK_DATA new_tick;
	bool flag=false;
	std::map<unsigned int, DS_Active_CTX*>::iterator itSearch;

	// Lock 'em
	pSDShare->pRDQ->buffer_lock();
	pSDShare->pPredBusyHash->predBusyHash_sso.lock();

	try
	{
		it = pSDShare->pRDQ->begin();
		it_end = pSDShare->pRDQ->end();
		rc = ATS_C_EMPTY;
		// Find "next available" RDQ entry that's not set to BUSY in predecessor hash
		while (it != it_end)
		{
			// Check state for ::NEW
			pSDShare->pRDQ->convert_forward_iterator_to_addr(it, ctx);
			if (ctx->state == CIRCULAR_BUFFER_STATES::NEW)
			{
				// Check for Active Context
				itSearch = pSDShare->pActiveCTXBlock->active_CTX_Container.find((unsigned int)ctx->element.db_sequence);
				if (itSearch == pSDShare->pActiveCTXBlock->active_CTX_Container.end()) {
					// No container context exists for this symbol - ignore it
					ctx->state = CIRCULAR_BUFFER_STATES::EMPTY;
				}
				else
				{
					// Check for SEQ# busy
					pSDShare->pPredBusyHash->value(ctx->element.db_sequence, flag);
					if (flag != true)
					{
						// Found a suitable one - set statuses and repackage
						ctx->state = CIRCULAR_BUFFER_STATES::BUSY;
						pSDShare->pPredBusyHash->set(ctx->element.db_sequence);
						uid = ctx->uid;
						db_seq = ctx->element.db_sequence;
						new_tick.time_added_to_buffer = ctx->element.time_added_to_buffer;
						new_tick.time_of_trade = ctx->element.time_of_trade;
						new_tick.trade_amt = ctx->element.trade_amt;
						new_tick.trade_vol = ctx->element.trade_vol;
						//new_tick.tuid = ctx->element.tuid;	// Copy of unique tuid from original RDQ element
						rc = ATS_C_FOUND;
						break;
					}
				}
			}
			// Try next
			it = pSDShare->pRDQ->container_iterator_incr(it);
		}
	}
	catch (...)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			pLogger->log_warn(L"Encountered an exception during (Part I) processed of data packet", L"DataCollectorWorker::processRDQEvent");
		rc = ATS_C_EMPTY;
	}

	// Unlock 'em
	pSDShare->pPredBusyHash->predBusyHash_sso.release();
	pSDShare->pRDQ->buffer_release();

	// No need to continue if nothing found
	if (rc != ATS_C_FOUND)
	{
		return ATS_C_EMPTY;
	}

	///////////////////////////////////////////////////////////////////////////
	//	PART II - Process NEW element
	//	
	//	- Add to TICK buffer
	//		- Repackage provider trade element to ATS TICK
	//		- Compress if required
	//		- Check for out-lier
	//	- Send Event Notification
	//		- Update Next Ready Queue
	//		- Notify Analytics Engine that a collection buffer has been updated
	///////////////////////////////////////////////////////////////////////////
		// Setup active context
	DS_Active_CTX* pDSACtx = pSDShare->pActiveCTXBlock->active_CTX_Container[db_seq];
	DS_Context* pDSCtx = pDSACtx->pCtx;

	// Lock ACTIVE Context for this security based on db_seq
	pDSACtx->DS_Active_sso.lock();
	try
	{
		// Insert into this Active Context TICK buffer
		pDSCtx->tickBuf.insert_back(&new_tick);
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
		{
			swprintf(logMsg, sizeof logMsg, L"Inserted trade record into Active Context TICK buffer [symbol %6s, uid %u, db_seq %d, cb_active %d] for thread (%d)",
						ctx->element.symbol, uid, db_seq, pDSCtx->tickBuf.num_active_elements(), tid);
			pLogger->log_debug(logMsg, L"DataCollectorWorker::processRDQEvent");
		}
	}
	catch (...)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			pLogger->log_warn(L"Encountered an exception during (Part II) processed of data packet", L"DataCollectorWorker::processRDQEvent");
	}

	// Unlock ACTIVE Context for db_seq
	pDSACtx->DS_Active_sso.release();

	// Add recently processed RDQ entry to NRQ and send event notification to Analytics Engine
	processAnalyticsNotification(db_seq);															//<------------------- TEST (Let Analytics threads know there is something new in DS)

	///////////////////////////////////////////////////////////////////////////
	//	PART III - Clear entry from RDQ
	//
	//	- Invalidate entry in buffer - Reset to UNUSED
	//	- Reset Predecessor Busy Hash entry
	///////////////////////////////////////////////////////////////////////////
		// Lock 'em
	pSDShare->pRDQ->buffer_lock();
	pSDShare->pPredBusyHash->predBusyHash_sso.lock();

	// Set RDQ entry to UNUSED based on UID (invalidates entry making it eligible for clean-up)
	pSDShare->pRDQ->invalidate_entry(uid);
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
	{
		swprintf(logMsg, sizeof logMsg, L"Invalidate entry [uid %u] in PredBusyHash - thread [%d] - RDQ Count [%u],  Begin(%u), End(%u)", 
				uid, tid, pSDShare->pRDQ->num_active_elements(), pSDShare->pRDQ->begin(), pSDShare->pRDQ->end());
		pLogger->log_debug(logMsg, L"DataCollectorWorker::processRDQEvent");
	}

	// Reset Predecessor Busy Hash value
	pSDShare->pPredBusyHash->reset(db_seq);

	// Unlock 'em
	pSDShare->pPredBusyHash->predBusyHash_sso.release();
	pSDShare->pRDQ->buffer_release();

	return ATS_C_EMPTY;
}


///////////////////////////////////////////////////////////////////////////////
//	Update NRQ + Busy Hash for seq#
//	- Collector inserts a message into NRQ
//	- Send event notification to Analytics worker threads
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DataCollectorWorker::processAnalyticsNotification(int seq)							//<------------- TEST (Process Analytics notification here)
{
	ATS_CODE rc = ATS_CODES::ATS_C_SUCCESS;
	int _seq = seq;
	NRQ_ELEMENT nrq_element;
	wchar_t logMsg[LOGGER_MSG_MEDIUMBUF];

	// Lock NRQ + Busy hash
	pSDShare->pNRQ->buffer_lock();
	pSDShare->pNRQBusyHash->nextReadyQBusyHash_sso.lock(); 

	// Lookup seq and process the event
	rc = pSDShare->pNRQBusyHash->get_value(_seq, nrq_element);
	if (rc == ATS_CODES::ATS_C_FOUND)
	{
		if (nrq_element.count == NRQ_ELEMENT_LIMIT)
		{
			// We're at the limit, nothing to do
			rc = ATS_CODES::ATS_C_SUCCESS;
		}
		else
		{
			if (nrq_element.count >= 0 && nrq_element.count <= 1)
			{
				// Insert seq into NRQ, modify NRQ Busy Hash element and send an event notification
				++nrq_element.count;
				pSDShare->pNRQ->insert_back(&_seq);
				pSDShare->pNRQBusyHash->modify_count(_seq, nrq_element);
				if (nrq_element.flag == NRQ_ELEMENT_BUSY_FLAG::NOT_BUSY)
				{
					// Send notification to waiting threads
					SetEvent(pSDShare->hPostAnalyticsNRQEvent);
				}
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
				{
					swprintf(logMsg, sizeof logMsg, L"NRQ Insertion (seq, container, active) %d:%d:%d", _seq, pSDShare->pNRQ->size_of_container(), pSDShare->pNRQ->num_active_elements());
					pLogger->log_debug(logMsg, L"DataCollectorWorker::processAnalyticsNotification");
				}
				rc = ATS_CODES::ATS_C_SUCCESS;
			}
			else
			{
				// There's a range issue
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
				{
					swprintf(logMsg, sizeof logMsg, L"Requested sequence number (%d) experienced a range issue (%d)", _seq, nrq_element.count);
					pLogger->log_warn(logMsg, L"DataCollectorWorker::processAnalyticsNotification");
				}
				rc = ATS_CODES::ATS_C_FAIL;
			}
		}
	}
	else
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"Requested sequence number (%d) does not found or some other error occurred", _seq);
			pLogger->log_warn(logMsg, L"DataCollectorWorker::processAnalyticsNotification");
		}
		rc = ATS_CODES::ATS_C_FAIL;
	}

	// Unlock NRQ + Busy hash
	pSDShare->pNRQBusyHash->nextReadyQBusyHash_sso.release();
	pSDShare->pNRQ->buffer_release();

	return rc;
}
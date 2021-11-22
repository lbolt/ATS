#include "stdafx.h"
#include "../Includes/StructX.h"




int StructX::isInitialized{ 0 };
DS_Globals*	StructX::pGlobals{ NULL };
LoggerX* StructX::pLogger{ NULL };



//	CTOR - No Globals Initializer
StructX::StructX()
{
	initialize(NULL);
}


//	CTOR - With Globals Initializer
//	- The actual StructX class is a container for 
//	  global shared common objects such as the Logger, etc.
//	- This constructor is called once by "  " to initialize
//	  the object
//	- All other classes that derive from StructX initiate the
//	  default constructor. No further initialization is required.
StructX::StructX(void* Args)
{
	ATS_CODE rc;
	rc = initialize(Args);
	//	Take additional steps here in case of error
}


//	Default DTOR
StructX::~StructX()
{
}


//	Setup pointers to commonly used global objects
ATS_CODE StructX::initialize(void* Args)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	if (Args == NULL)
		return rc;

	AutoCriticalSectionLock StructX_sso(StructX_cs);
	if (getObjectState() != 0)
		return rc;

	setObjectState();
	//	Setup Globals
	pGlobals = (DS_Globals*)Args;
	pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];

	/////////////////////////////
	//	++Set parameter values 
	/////////////////////////////

	//	StructX
	if (!pGlobals->pProps->prop_SYMBOL_MAXSIZE.empty())
	{
		param_SYMBOL_MAXSIZE = _wtoi(pGlobals->pProps->prop_SYMBOL_MAXSIZE.c_str());
	}
	if (!pGlobals->pProps->prop_DATA_COMPRESSION_ENABLED.empty())
	{
		(_wcsicmp(pGlobals->pProps->prop_DATA_COMPRESSION_ENABLED.c_str(), L"true") == 0) ? param_DATA_COMPRESSION_ENABLED = true : param_DATA_COMPRESSION_ENABLED = false;
	}
	if (!pGlobals->pProps->prop_DATA_GAP_ENABLED.empty())
	{
		(_wcsicmp(pGlobals->pProps->prop_DATA_GAP_ENABLED.c_str(), L"true") == 0) ? param_DATA_GAP_ENABLED = true : param_DATA_GAP_ENABLED = false;
	}
	if (!pGlobals->pProps->prop_OUTLIER_COLLECTION_SIZE.empty())
	{
		param_OUTLIER_COLLECTION_SIZE = _wtoi(pGlobals->pProps->prop_OUTLIER_COLLECTION_SIZE.c_str());
	}
	if (!pGlobals->pProps->prop_MAX_OUTLIER_DELTA.empty())
	{
		param_MAX_OUTLIER_DELTA = (double)_wtof(pGlobals->pProps->prop_MAX_OUTLIER_DELTA.c_str());
	}
	if (!pGlobals->pProps->prop_SUPERTICK_SIZE.empty())
	{
		param_SUPERTICK_SIZE = _wtoi(pGlobals->pProps->prop_SUPERTICK_SIZE.c_str());
	}
	if (!pGlobals->pProps->prop_MIN_SLOTS_FOR_TREND_CALCS.empty())
	{
		param_MIN_SLOTS_FOR_TREND_CALCS = _wtoi(pGlobals->pProps->prop_MIN_SLOTS_FOR_TREND_CALCS.c_str());
	}
	if (!pGlobals->pProps->prop_NUM_SLOTS_TO_ESTABLISH_TREND.empty())
	{
		param_NUM_SLOTS_TO_ESTABLISH_TREND = _wtoi(pGlobals->pProps->prop_NUM_SLOTS_TO_ESTABLISH_TREND.c_str());
	}
	if (!pGlobals->pProps->prop_MAX_SLOTS_ON_FOLLOW.empty())
	{
		param_MAX_SLOTS_ON_FOLLOW = _wtoi(pGlobals->pProps->prop_MAX_SLOTS_ON_FOLLOW.c_str());
	}
	if (!pGlobals->pProps->prop_ACTIVE_DS_TRIM_SIZE.empty())
	{
		param_ACTIVE_DS_TRIM_SIZE = (unsigned int)_wtoi(pGlobals->pProps->prop_ACTIVE_DS_TRIM_SIZE.c_str());
	}


	//	AnalyticX
	if (!pGlobals->pProps->prop_TREND_UPPER_GRADIENT.empty())
	{
		param_TREND_UPPER_GRADIENT = (double)_wtof(pGlobals->pProps->prop_TREND_UPPER_GRADIENT.c_str());
	}
	if (!pGlobals->pProps->prop_TREND_LOWER_GRADIENT.empty())
	{
		param_TREND_LOWER_GRADIENT = (double)_wtof(pGlobals->pProps->prop_TREND_LOWER_GRADIENT.c_str());
	}
	if (!pGlobals->pProps->prop_CHANNEL_COMPRESS_RANGE_LIMIT.empty())
	{
		param_CHANNEL_COMPRESS_RANGE_LIMIT = (double)_wtof(pGlobals->pProps->prop_CHANNEL_COMPRESS_RANGE_LIMIT.c_str());
	}
	if (!pGlobals->pProps->prop_CHANNEL_GAP_RANGE_LIMIT.empty())
	{
		param_CHANNEL_GAP_RANGE_LIMIT = (double)_wtof(pGlobals->pProps->prop_CHANNEL_GAP_RANGE_LIMIT.c_str());
	}
	if (!pGlobals->pProps->prop_BASIS_POINT_ADJ.empty())
	{
		param_BASIS_POINT_ADJ = (double)_wtof(pGlobals->pProps->prop_BASIS_POINT_ADJ.c_str());
	}
	/////////////////////////////
	//	--Set parameter values 
	/////////////////////////////


	return rc;
}


int StructX::getObjectState()
{
	return isInitialized;
}


int StructX::setObjectState()
{
	isInitialized = 1;
	return isInitialized;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	Data Collection Structures
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//	WatchPool
///////////////////////////////////////////////////////////////////////////////
WatchPool::WatchPool()
{

}


WatchPool::~WatchPool()
{

}


///////////////////////////////////////////////////////////////////////////////
//	WatchPoolItem
///////////////////////////////////////////////////////////////////////////////
WatchPoolItem::WatchPoolItem()
{

}


WatchPoolItem::~WatchPoolItem()
{

}


///////////////////////////////////////////////////////////////////////////////
//	DS_Active_CTX_Block
///////////////////////////////////////////////////////////////////////////////
DS_Active_CTX_Block::DS_Active_CTX_Block()
{

}


DS_Active_CTX_Block::~DS_Active_CTX_Block()
{

}


ATS_CODE DS_Active_CTX_Block::createActiveCTXRecord(WatchPoolItem* pCtxRecord)
{
	ATS_CODE rc;
	DS_Active_CTX* pDS_Ctx=NULL;
	DS_Context* pCtx=NULL;
	try
	{
		// Create DS Active Context and DS Context
		pDS_Ctx = new DS_Active_CTX();
		pCtx = new DS_Context();
		// Set-up DS Context
		pDS_Ctx->symbol = pCtxRecord->symbol;
		pDS_Ctx->pCtx = pCtx;
		// Insert DS Active Context
		active_CTX_Container.insert(std::pair<unsigned int, DS_Active_CTX*>(pCtxRecord->sequence, pDS_Ctx));
		rc = ATS_C_SUCCESS;
	}
	catch (...)
	{
		if (pDS_Ctx != NULL)
			delete pDS_Ctx;
		if (pCtx != NULL)
			delete pCtx;
		// LOG ERROR
		rc = ATS_C_NOMEM;
	}
	return rc;
}


ATS_CODE DS_Active_CTX_Block::removeActiveCTXRecord(unsigned int seq)
{

	return ATS_C_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//	DS_Active_CTX
///////////////////////////////////////////////////////////////////////////////
DS_Active_CTX::DS_Active_CTX()
{
	initialize();
}


DS_Active_CTX::~DS_Active_CTX()
{

}

ATS_CODE DS_Active_CTX::initialize(void)
{
	//	DS_Active_sso.sso initializes automatically
	return ATS_OK;
}


///////////////////////////////////////////////////////////////////////////////
//	DS_Context
///////////////////////////////////////////////////////////////////////////////
DS_Context::DS_Context()
{

}


DS_Context::~DS_Context()
{

}


ATS_CODE DS_Context::initialize()
{

	return ATS_C_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//	WatchPoolItemSequence
///////////////////////////////////////////////////////////////////////////////
WatchPoolItemSequence::WatchPoolItemSequence()
{

}


WatchPoolItemSequence::~WatchPoolItemSequence()
{

}


///////////////////////////////////////////////////////////////////////////////
//	PredecessorBusyHash
///////////////////////////////////////////////////////////////////////////////
PredecessorBusyHash::PredecessorBusyHash()
{
	ATS_CODE rc = initialize();
	return;
}

PredecessorBusyHash::~PredecessorBusyHash()
{

}

ATS_CODE PredecessorBusyHash::initialize(void)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	//	predBusyHash_sso initializes automatically
	return rc;
}

ATS_CODE PredecessorBusyHash::add(unsigned int seq)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	predecessor.insert(std::pair<unsigned int, DS_Active_CTX*>(seq, false));
	return rc;

}

ATS_CODE PredecessorBusyHash::value(unsigned int seq, bool& value)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	std::map<unsigned int, bool>::iterator it;
	it = predecessor.find(seq);
	value = it->second;
	return rc;

}

ATS_CODE PredecessorBusyHash::remove(unsigned int seq)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	predecessor.erase(seq);
	return rc;

}

ATS_CODE PredecessorBusyHash::set(unsigned int seq)
{
	ATS_CODE rc=ATS_C_SUCCESS;
	predecessor[seq] = true;
	return rc;
}

ATS_CODE PredecessorBusyHash::reset(unsigned int seq)
{
	ATS_CODE rc=ATS_C_SUCCESS;
	predecessor[seq] = false;
	return rc;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	Data Analytic Structures
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//	NextReadyQBusyHash
///////////////////////////////////////////////////////////////////////////////
NextReadyQBusyHash::NextReadyQBusyHash()
{
	ATS_CODE rc = initialize();
}

NextReadyQBusyHash::~NextReadyQBusyHash()
{

}

ATS_CODE NextReadyQBusyHash::initialize(void)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	//	nextReadyQBusyHash_sso initializes automatically
	return rc;
}

ATS_CODE NextReadyQBusyHash::add(unsigned int seq)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	NRQ_ELEMENT elem = { NRQ_ELEMENT_BUSY_FLAG::NOT_BUSY,0 };		// Initialize busy flag
	nextReady.insert(std::pair<unsigned int, NRQ_ELEMENT>(seq, elem));
	return rc;
}

ATS_CODE NextReadyQBusyHash::remove(unsigned int seq)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	nextReady.erase(seq);
	return rc;
}

ATS_CODE NextReadyQBusyHash::get_value(unsigned int seq, NRQ_ELEMENT& elem)
{
	ATS_CODE rc = ATS_C_FOUND;
	NRQ_ELEMENT _elem;

	std::map<unsigned int, NRQ_ELEMENT>::iterator it;
	it = nextReady.find(seq);		// Get iterator to seq from map object
	if (it == nextReady.end())
	{
		rc = ATS_C_NONEXIST;
	}
	else
	{
		_elem = it->second;
		elem = _elem;
	}
	return rc;
}


ATS_CODE NextReadyQBusyHash::modify_count(unsigned int seq, NRQ_ELEMENT& elem)		//	TODO: Clean-up  <--------------------------------------
{
	ATS_CODE rc = ATS_C_FOUND;
	NRQ_ELEMENT _elem;

	std::map<unsigned int, NRQ_ELEMENT>::iterator it;
	it = nextReady.find(seq);		// Get iterator to seq from map object
	if (it == nextReady.end())
	{
		rc = ATS_C_NONEXIST;
	}
	else
	{
		_elem = it->second;
		_elem.count = elem.count;	// Modify count field in local copy
		nextReady.at(seq) = _elem;	// Update map object with new count
	}
	return rc;
}


ATS_CODE NextReadyQBusyHash::set_state(unsigned int seq, NRQ_ELEMENT_BUSY_FLAG flag)
{
	ATS_CODE rc = ATS_C_FOUND;
	NRQ_ELEMENT elem;

	std::map<unsigned int, NRQ_ELEMENT>::iterator it;
	it = nextReady.find(seq);		// Get iterator to seq from map object
	if (it == nextReady.end())
	{
		rc = ATS_C_NONEXIST;
	}
	else
	{
		elem = nextReady[seq];
		elem.flag = flag;
		nextReady.at(seq) = elem;
	}
	return rc;
}


ATS_CODE NextReadyQBusyHash::incr_count(unsigned int seq)
{
	ATS_CODE rc = ATS_C_FOUND;
	NRQ_ELEMENT elem;

	std::map<unsigned int, NRQ_ELEMENT>::iterator it;
	it = nextReady.find(seq);		// Get iterator to seq from map object
	if (it == nextReady.end())
	{
		rc = ATS_C_NONEXIST;
	}
	else
	{
		elem = nextReady[seq];
		if (elem.count < NRQ_ELEMENT_LIMIT)
		{
			elem.count += 1;
		}
		nextReady.at(seq) = elem;
	}
	return rc;
}


ATS_CODE NextReadyQBusyHash::decr_count(unsigned int seq)
{
	ATS_CODE rc = ATS_C_FOUND;
	NRQ_ELEMENT elem;

	std::map<unsigned int, NRQ_ELEMENT>::iterator it;
	it = nextReady.find(seq);		// Get iterator to seq from map object
	if (it == nextReady.end())
	{
		rc = ATS_C_NONEXIST;
	}
	else
	{
		elem = nextReady[seq];
		if (elem.count > 0 && elem.count <= NRQ_ELEMENT_LIMIT)
		{
			elem.count -= 1;
		}
		nextReady.at(seq) = elem;
	}
	return rc;
}


///////////////////////////////////////////////////////////
//	ActiveAnalyticCtxBlock
///////////////////////////////////////////////////////////
ActiveAnalyticCtxBlock::ActiveAnalyticCtxBlock(void* Args)
{
	ATS_CODE rc;
	rc = initialize(Args);
}


ActiveAnalyticCtxBlock::~ActiveAnalyticCtxBlock()
{

}


ATS_CODE ActiveAnalyticCtxBlock::initialize(void* Args)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	pGlobals = (DS_Globals*)Args;

	//	Initialization of StructX common static global components
	//	before any object instances are created
	StructX m_StructX((void*)Args);

	return rc;
}


ATS_CODE ActiveAnalyticCtxBlock::createContainerRecord(WatchPoolItem* pCtxRecord)
{
	ATS_CODE rc=ATS_C_SUCCESS;
	ActiveAnalyticCtx* pActiveAnalyticCtx = NULL;
	AnalyticCtx_NOOP* noop = NULL;
	AnalyticCtx_LTLS* ltls = NULL;
	try
	{
		// Create Active Analytic Context
		pActiveAnalyticCtx = new ActiveAnalyticCtx();
		// Set-up Active Analytic Context
		pActiveAnalyticCtx->symbol = pCtxRecord->symbol;
		pActiveAnalyticCtx->activeAnalyticCtxList.clear();
		// Insert Active Analytic Context into MAP object (List needs to be loaded separately)
		activeAnalyticCtxContainer.insert(std::pair<unsigned int, ActiveAnalyticCtx*>(pCtxRecord->sequence, pActiveAnalyticCtx));

		// Load Analytics_NOOP & Analytics_LTLS into dispatcher list as defaults
		// In the future, this will be done based on assignments in the Analytic_Seq_Algo table				<----------------Temporary
		noop = new AnalyticCtx_NOOP();
		noop->desc = L"AnalyticCtx_NOOP";
		pActiveAnalyticCtx->activeAnalyticCtxList.push_back(noop);
		ltls = new AnalyticCtx_LTLS((void*)pGlobals);
		ltls->desc = L"AnalyticCtx_LTLS";
		pActiveAnalyticCtx->activeAnalyticCtxList.push_back(ltls);

		rc = ATS_C_SUCCESS;
	}
	catch (...)
	{
		if (pActiveAnalyticCtx != NULL)
			delete pActiveAnalyticCtx;
		if (noop != NULL)
			delete noop;
		if (ltls != NULL)
			delete ltls;

		// LOG ERROR
		rc = ATS_C_NOMEM;
	}
	return rc;
}


ATS_CODE ActiveAnalyticCtxBlock::removeContainerRecord(unsigned int seq)
{

	return ATS_C_SUCCESS;
}


///////////////////////////////////////////////////////////
//	ActiveAnalyticCtx
///////////////////////////////////////////////////////////
ActiveAnalyticCtx::ActiveAnalyticCtx()
{
	ATS_CODE rc;
	rc = initialize();
}


ActiveAnalyticCtx::~ActiveAnalyticCtx()
{

}


ATS_CODE ActiveAnalyticCtx::initialize(void)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	return rc;
}


///////////////////////////////////////////////////////////
//	Make a copy of the current AnalyticCtx list
ATS_CODE ActiveAnalyticCtx::copyListtoQ(std::list<CoreAnalyticCtx*> ctxList, std::queue< CoreAnalyticCtx*>& ctxQ)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	list<CoreAnalyticCtx*>::iterator it, it_end;
	for (it=ctxList.begin(), it_end=ctxList.end(); it != it_end; it++)
	{
		ctxQ.push(*it);
	}
	return rc;
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//	ANALYTICS IMPLEMENTATION CLASSES
//
//	Implement following virtual methods for each handler
//		- lookahead data handler
//		- outlier handler
//		- data smoothing handler
//		- super tick processing handler
//		- trend analysis handler
//		- reporting handler 
//		- notification handler
//
//	Note: 
//		- Each symbol under watch has its own handler
//		  instances
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
//	CoreAnalyticCtx (base class)
///////////////////////////////////////////////////////////
CoreAnalyticCtx::CoreAnalyticCtx() //: m_pGlobals(Args)
{
	ATS_CODE rc;
	rc = initialize();
}

CoreAnalyticCtx::~CoreAnalyticCtx()
{

}

ATS_CODE CoreAnalyticCtx::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		// Create all contexts required for this Analytic instance
		pAELib = new LTLS_Analysis((void*)pGlobals);	//	NULL
	}
	catch (...)
	{
		// Failed during context creation - delete residual
		if (pAELib != 0)
			delete pAELib;
		rc = ATS_C_FAIL;
	}

	return rc;
}

///////////////////////////////////////////////////////////
//	AnalyticCtx_NOOP
//	- Every symbol under watch has an instance of this
//	  handler by default
//	- This handler is for tracing purposes
///////////////////////////////////////////////////////////
AnalyticCtx_NOOP::AnalyticCtx_NOOP()
{
	ATS_CODE rc;
	rc = initialize();
}

AnalyticCtx_NOOP::~AnalyticCtx_NOOP()
{

}

ATS_CODE AnalyticCtx_NOOP::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	return rc;
}

ATS_CODE AnalyticCtx_NOOP::invokeHandlers(unsigned int db_seq, DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	

	return rc;
}

ATS_CODE AnalyticCtx_NOOP::lookaheadHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_NOOP::outlierHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_NOOP::dataSmoothingHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_NOOP::supertickHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_NOOP::trendAnalysisHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_NOOP::reportHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_NOOP::notificationHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

///////////////////////////////////////////////////////////
//	AnalyticCtx_LTLS
//	- Linear Trend Long / Short Analysis Analytic
///////////////////////////////////////////////////////////
AnalyticCtx_LTLS::AnalyticCtx_LTLS(void* Args)
{
	ATS_CODE rc;
	rc = initialize(Args);	//	Args is valid
}

AnalyticCtx_LTLS::~AnalyticCtx_LTLS()
{
	//	Clean-up
	if (pLookAhead != 0)
		delete pLookAhead;
	if (pSuperTick != 0)
		delete pSuperTick;
	if (pDAContainer != 0)
		delete pDAContainer;
}

ATS_CODE AnalyticCtx_LTLS::initialize(void* Args)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	try
	{
		//pGlobals = (DS_Globals*)Args; //	Args is valid
		//pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];

		// Create all contexts required for this Analytic instance
		pLookAhead = new AnalyticLookAheadBuffer();
		pSuperTick = new AnalyticSuperTickBuffer();
		pDAContainer = new DataAggregateContainer();
	}
	catch (...)
	{
		//	Failed during context creation
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
		{
			pLogger->log_error(L"Exception during object creation", L"AnalyticCtx_LTLS::initialize");
		}
		//	Delete residual
		if (pLookAhead != 0)
			delete pLookAhead;
		if (pSuperTick != 0)
			delete pSuperTick;
		if (pDAContainer != 0)
			delete pDAContainer;
		rc = ATS_C_FAIL;
	}

	return rc;
}

ATS_CODE AnalyticCtx_LTLS::invokeHandlers(unsigned int db_seq, DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	wstring symbol = pCtx->symbol;
	DWORD tid = GetCurrentThreadId();

	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
	{
		swprintf(logMsg, sizeof logMsg, L"Entered Handler - sym: %s thread: %d", symbol.c_str(), tid);
		pLogger->log_func_entry(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
	}

	///////////////////////////////////////////////////////
	//	Copy new trades from Collector DS to lookahead
	///////////////////////////////////////////////////////
	rc = lookaheadHandler(pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"lookaheadHandler failed with %d", rc);
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
		}
		goto invokeHandlers_exit;
	}
	///////////////////////////////////////////////////////
	//	Process outliers
	///////////////////////////////////////////////////////
	rc = outlierHandler(pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		if (rc == ATS_C_AE_INCOMPLETE)
		{
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
			{
				swprintf(logMsg, sizeof logMsg, L"outlierHandler exited do to incomplete processing");
				pLogger->log_debug(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
			}
			rc = ATS_C_SUCCESS;
		}
		else
		{
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			{
				swprintf(logMsg, sizeof logMsg, L"outlierHandler failed with %d", rc);
				pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
			}
			goto invokeHandlers_exit;
		}
	}
	///////////////////////////////////////////////////////
	//	Data smoothing
	///////////////////////////////////////////////////////
	rc = dataSmoothingHandler(pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"dataSmoothingHandler failed with %d", rc);
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
		}
	}
	///////////////////////////////////////////////////////
	//	Compose SuperTicks
	///////////////////////////////////////////////////////
	rc = supertickHandler(pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"supertickHandler failed with %d", rc);
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
		}
	}
	///////////////////////////////////////////////////////
	//	Perform Trend Analysis
	///////////////////////////////////////////////////////
	rc = trendAnalysisHandler(pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"trendAnalysisHandler failed with %d", rc);
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
		}
	}
#ifdef _UNUSED_

	///////////////////////////////////////////////////////
	//	Report Results
	///////////////////////////////////////////////////////
	rc = reportHandler(pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"reportHandler failed");
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
		}
	}
	///////////////////////////////////////////////////////
	//	Perform notifications
	///////////////////////////////////////////////////////
	rc = notificationHandler(pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"notificationHandler failed");
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
		}
	}

#endif _UNUSED_
	invokeHandlers_exit:

	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
	{
		swprintf(logMsg, sizeof logMsg, L"Exiting for sym: %s thread: %d", symbol.c_str(), tid);
		pLogger->log_func_entry(logMsg, L"AnalyticCtx_LTLS::invokeHandlers");
	}

	return rc;
}

ATS_CODE AnalyticCtx_LTLS::lookaheadHandler(DS_Active_CTX* pCtx)						//<------------------------------------------ LOOK_AHEAD HANDLER -------------------------------------------
{
	ATS_CODE rc = ATS_C_SUCCESS;
	wstring symbol = pCtx->symbol;

	Container_forward_iterator it, itSrcBegin, itSrcEnd;
	CircularBuffer<TICK_DATA>* pActiveTickBuffer = NULL;
	unsigned int activeCount=0;

	CONTAINER_CTX_BLOCK<TICK_DATA>* srcCtx = NULL;
	TICK_DATA newTick;
	double pivotAmt;

	unsigned int lastSlotUid = 0;
	unsigned int successorUid = 0;

	pCtx->DS_Active_sso.lock();
	try
	{
		//	Iterate through Active DS buffer from last processed uid until end of buffer
		//	Add new trade records to the lookahead buffer
		//	Trim active DS buffer - no need to keep more than 10k (approximately 30 minutes) worth of slots

		pActiveTickBuffer = &pCtx->pCtx->tickBuf;	//	Active DS buffer for symbol
		itSrcBegin = pActiveTickBuffer->begin();
		itSrcEnd = pActiveTickBuffer->end();
		lastSlotUid = pLookAhead->lastActiveDSslotUid;
		activeCount = pActiveTickBuffer->num_active_elements();	//	Always account for "embedded" empty slots

		//	Establish starting point
		if (itSrcBegin == itSrcEnd || activeCount < 2)
		{
			goto lookaheadHandler_exit;	//	Source buffer empty
		}

		if (lastSlotUid == 0)
		{
			//	Start from first valid slot uid in collection
			it = itSrcBegin;
		}
		else
		{
			//	Set starting iterator to next successive (unprocessed) uid
			rc = pActiveTickBuffer->find_successor_uid(lastSlotUid, successorUid);
			if (rc == ATS_C_SUCCESS)
			{
				rc = pActiveTickBuffer->find_element_by_uid(successorUid, it);
				if (rc != ATS_C_SUCCESS)
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
					{
						swprintf(logMsg, sizeof logMsg, L"Convert last slot uid [%d] to iterator inconsistency failure", lastSlotUid);
						pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::lookaheadHandler");
					}
					goto lookaheadHandler_exit;	//	iterator conversion call error - successor iterator inconsistency
				}
			}
			else if (rc == ATS_C_NONEXIST)
			{
				//	Buffer may have been trimmed, a higher valued uid will arrive
				rc = ATS_C_SUCCESS;
				goto lookaheadHandler_exit;	//	okay; no successor
			}
			else
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
				{
					swprintf(logMsg, sizeof logMsg, L"lookAhead buffer successor uid [%d] lookup failed", lastSlotUid);
					pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::lookaheadHandler");
				}
				goto lookaheadHandler_exit;	//	lookup call error - no successor
			}
		}

		//	Copy all newly added trades to lookahead
		pLookAhead->getPivotAmt(pivotAmt);
		while (it != itSrcEnd)
		{
			rc = pActiveTickBuffer->convert_forward_iterator_to_addr(it, srcCtx);
			if (rc != ATS_C_SUCCESS)
			{
				//	Not found - error; try next
				it = pActiveTickBuffer->container_iterator_incr(it);
				continue;
			}
			if (srcCtx->state == CIRCULAR_BUFFER_STATES::EMPTY)
			{
				it = pActiveTickBuffer->container_iterator_incr(it);
				continue;
			}

			//	Add Active DS buffer slot data to lookahead buffer
			memcpy((void*)&newTick, (void*)&srcCtx->element, sizeof(newTick));
			rc = pLookAhead->insertTickData(&newTick);
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
			{
				swprintf(logMsg, sizeof logMsg, L"lookAhead buffer data insertion [uid: %d, cb_active: %d],  sym:  %s", 
							srcCtx->uid, pLookAhead->getLookAheadCount(), symbol.c_str());
				pLogger->log_debug(logMsg, L"AnalyticCtx_LTLS::lookaheadHandler");
			}
			//	Set pivot amt if necessary
			if (pivotAmt == 0.0)
			{
				pivotAmt = newTick.trade_amt;
				pLookAhead->setPivotAmt(pivotAmt);
			}

			it = pActiveTickBuffer->container_iterator_incr(it);
			pLookAhead->lastActiveDSslotUid = srcCtx->uid;
		}	//	while-loop

		//	Trim slots from front of Active DS Buffer
		if (activeCount > param_ACTIVE_DS_TRIM_SIZE)
		{
			pActiveTickBuffer->trim_front(activeCount - param_ACTIVE_DS_TRIM_SIZE);
		}

		rc = ATS_C_SUCCESS;

	}
	catch (...)
	{
		//	Report error
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"lookaheadHandler exception");
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::lookaheadHandler");
		}
		rc = ATS_C_FAIL;
	}

lookaheadHandler_exit:
	pCtx->DS_Active_sso.release();

	return rc;
}

ATS_CODE AnalyticCtx_LTLS::outlierHandler(DS_Active_CTX* pCtx)						//<------------------------------------------ OUTLIER HANDLER -------------------------------------------
{
	ATS_CODE rc = ATS_C_SUCCESS;
	Container_element_uid uid;
	Container_element_uid lastUid;
	Container_element_uid savedLastUid;
	Container_forward_iterator itLA;
	Container_forward_iterator itLA_end;

	double actualDiff;
	double maxDelta;
	double pivotAmt = 0.0;
	Container_element_uid stackItemUid = 0;
	Container_element_uid itemUid = 0;
	TICK_DATA*	pTickData;
	boolean		foundDiff;
	CONTAINER_CTX_BLOCK<TICK_DATA>*	pElemContainer = NULL;	//	Element container
	Container_element_uid _successorUid;
	unsigned int tickCount;

	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
	{
		swprintf(logMsg, sizeof logMsg, L"Entered Handler");
		pLogger->log_func_entry(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
	}

	wstring symbol = pCtx->symbol;

	try
	{
		///////////////////////////////////////////////////////////////////////
		//	Determine starting iterator for LookAhead Buffer processing
		///////////////////////////////////////////////////////////////////////
		itLA = pLookAhead->lookAheadBegin();
		itLA_end = pLookAhead->lookAheadEnd();
		uid = 0;
		lastUid = 0;
		savedLastUid = 0;

		pLookAhead->getTickCount(tickCount);
		if (itLA == itLA_end)
		{
			//	Nothing to do
			return ATS_C_AE_INCOMPLETE;
		}

		///////////////////////////////////////////////////////////////////////
		//	Setting the start iterator based on the last uid processed
		///////////////////////////////////////////////////////////////////////
		pLookAhead->getLastUid(savedLastUid);
		lastUid = savedLastUid;	//	Saved for diagnostic purposes
		if (lastUid == 0)
		{
			//	Start from the beginning of buffer
			pLookAhead->lookupUid(itLA, uid);
			lastUid = uid;
			pLookAhead->setLastUidProcessed(lastUid);
		}
		else
		{
			//	lastUid was previously set, find its non-EMPTY successor and use its iterator
			rc = pLookAhead->findSuccessorUid(lastUid, _successorUid);
			if (rc == ATS_C_SUCCESS)
			{
				lastUid = _successorUid;
				pLookAhead->convertUidToIterator(lastUid, itLA);
				pLookAhead->setLastUidProcessed(lastUid);
			}
			else
			{
				//	No successor uid
				return ATS_C_AE_INCOMPLETE;
			}
		}

		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
		{
			swprintf(logMsg, sizeof logMsg, L"Begin iterator [%d],  end() [%d], saved lastUid: [%d],  successor uid: [%d],  symbol: %s", 
												itLA, itLA_end,  savedLastUid, lastUid, symbol.c_str());
			pLogger->log_debug(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
		}

		///////////////////////////////////////////////////////////////////////
		//	Iterate through LookAhead buffer and process outliers (blips)
		//
		//	- Returns:
		//		ATS_C_AE_INCOMPLETE		- Stack not empty
		//		ATS_C_SUCCESS			- otherwise
		///////////////////////////////////////////////////////////////////////

		while (itLA != itLA_end)
		{
			///////////////////////////////////////////////////////////////////
			//	Calculate difference between pivot and current amt
			///////////////////////////////////////////////////////////////////
			foundDiff = false;
			pLookAhead->getPivotAmt(pivotAmt);			
			rc = pLookAhead->getTickContainer(itLA, pElemContainer);
			if (rc == ATS_C_SUCCESS)
			{
				if ( pElemContainer->state == CIRCULAR_BUFFER_STATES::EMPTY)
				{
					// Ignore it
					itLA = pLookAhead->increment(itLA);
					continue;
				}
			}
			else
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
				{
					swprintf(logMsg, sizeof logMsg, L"Container lookup failed for iterator: [%d], symbol %s", itLA, symbol.c_str());
					pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
				}
				itLA = pLookAhead->increment(itLA);
				continue;
			}
			itemUid = pElemContainer->uid;								//	uid for current iterator
			pLookAhead->getTickContext(itLA, pTickData);					//	
			maxDelta = pivotAmt * (pLookAhead->percentDiff / 100.0);	//	Calculated max delta
			actualDiff = abs(pTickData->trade_amt - pivotAmt);			//	|Actual difference|

			////////////////////////////////////////////////////////////////////////////
			//	Check to see if an  OUTLIER (or blip) has been detected:
			//	- If so, push slot uid on to OL stack and set slot state to BUSY
			////////////////////////////////////////////////////////////////////////////
			if (actualDiff > maxDelta)
			{
				foundDiff = true;
				pLookAhead->outlierStack.push(itemUid);
				rc = pLookAhead->setSlotState(itemUid, CIRCULAR_BUFFER_STATES::BUSY);	//	Referenced by ST handler; reset when slot is cleared
				if (rc == ATS_C_SUCCESS)
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
					{
						swprintf(logMsg, sizeof logMsg, L"Differential check on pivot amt: [%f] and trade amt: [%f] produced possible outlier, stack size [%zd], symbol %s",
							pivotAmt, pTickData->trade_amt, pLookAhead->outlierStack.size(), symbol.c_str());
						pLogger->log_debug(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
					}
				}
				else
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
					{
						swprintf(logMsg, sizeof logMsg, L"setSlotState for uid (%d) failed with %d while resetting state on slot, symbol %s", itemUid, rc, symbol.c_str());
						pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
					}
				}
			}
			else
			{
				//	Update the pivot to reflect acceptable incremental changes
				pLookAhead->setPivotAmt(pTickData->trade_amt);
			}

			////////////////////////////////////////////////////////////////////////////
			//	Check result for GAP or qualified Outlier(s)
			//	- On GAP - accept stack entries
			//	- on qualified OUTLIER(s) clear stack entries and invalidate slots
			//	- Sets lastUid
			////////////////////////////////////////////////////////////////////////////
			if (pLookAhead->outlierStack.size() == pLookAhead->outlierLimit)
			{
				//	GAP
				//	- Accept slots as gap values - clear from stack
				while (!pLookAhead->outlierStack.empty())
				{
					stackItemUid = pLookAhead->outlierStack.top();
					pLookAhead->outlierStack.pop();
					//	Set STATE from BUSY to NEW for each uid on the stack
					rc = pLookAhead->getUidContainer(stackItemUid, pElemContainer);
					if (rc == ATS_C_SUCCESS)
					{
						pLookAhead->setSlotState(stackItemUid, CIRCULAR_BUFFER_STATES::NEW);
					}
					else
					{
						//	Sanity check - Not found; report and try next
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
						{
							swprintf(logMsg, sizeof logMsg, L"getUidContainer(%d) failed with %d while resetting state on GAP values, symbol %s", stackItemUid, rc, symbol.c_str());
							pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
						}
					}
				}	//	End-Inner-While
				pLookAhead->setPivotAmt(pTickData->trade_amt);
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
				{
					swprintf(logMsg, sizeof logMsg, L"lookAhead buffer values indicate GAP - Cleared stack and accepted values- New pivot [%f], symbol %s", pTickData->trade_amt, symbol.c_str());
					pLogger->log_debug(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
				}
			}
			else if (foundDiff == false && pLookAhead->outlierStack.size() > 0)
			{
				//	Qualified OUTLIER(s) detected
				//	- Remove items from stack and lookAhead buffer
				while (!pLookAhead->outlierStack.empty())
				{
					stackItemUid = pLookAhead->outlierStack.top();
					pLookAhead->outlierStack.pop();
					rc = pLookAhead->invalidateSlot(stackItemUid);
					if (rc == ATS_C_SUCCESS)
					{
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
						{
							swprintf(logMsg, sizeof logMsg, L"Cleared outlier uid: [%d] from lookAhead buffer, symbol %s", stackItemUid, symbol.c_str());
							pLogger->log_debug(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
						}
					}
					else if (rc != ATS_C_NONEXIST)
					{
						//	Sanity check - Not found; report and try next
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
						{
							swprintf(logMsg, sizeof logMsg, L"invalidateSlot(%d) failed with [%d], symbol %s", stackItemUid, rc, symbol.c_str());
							pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
						}
					}
				}	//	End-Inner-While
			}
			else
			{
				//	OUTLIER stack > 0 and GAP limit has not been met - continue processing
				;
			}

			pLookAhead->setLastUidProcessed(itemUid);	//	Current item uid
			itLA = pLookAhead->increment(itLA);

		}	//	End-Outer-While
		rc = ATS_C_SUCCESS;

		if (pLookAhead->outlierStack.size() > 0)
		{
			//	Unresolved OUTLIER(s) remain on stack - need more slots before next handler
			rc = ATS_C_AE_INCOMPLETE;
		}

	}
	catch (...)
	{
		//	Report error
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"Unspecified exception while processing %s", symbol.c_str());
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::outlierHandler");
		}
		rc = ATS_C_FAIL;
	}

	return rc;
}

ATS_CODE AnalyticCtx_LTLS::dataSmoothingHandler(DS_Active_CTX* pCtx)						//<------------------------------------------ DATA_SMOOTHING HANDLER --------------------------------
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}


///////////////////////////////////////////////////////////
//	supertickHandler
//	- Process Ticks from LookAhead buffer into SuperTicks
//	- Adjust pivot amount if necessary
//	- Collected tick slots are set to unused in buffer
///////////////////////////////////////////////////////////
ATS_CODE AnalyticCtx_LTLS::supertickHandler(DS_Active_CTX* pCtx)							//<------------------------------------------ SUPERTICK HANDLER -------------------------------------
{
	ATS_CODE rc = ATS_C_SUCCESS;

	unsigned int activeSlots;				//	Count of slots with state == NEW in LookAhead
	int barLimit = param_SUPERTICK_SIZE;	//	Number of ticks per Bar (A.K.A. SuperTick)
	int tickCount;							//	Keep track of counts for bar summations
	double tickAmtSum;
	int tickVolSum;
	int barsToProcess;

	CONTAINER_CTX_BLOCK<TICK_DATA>*		pElemContainer;	//	Tick element container
	Container_forward_iterator			itLA;			//	Lookahead Begin iterator
	Container_forward_iterator			it_endLA;		//	Lookahead End iterator
	S_TICK_DATA							superTick;
	Container_element_uid				slotUid;

	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
	{
		swprintf(logMsg, sizeof logMsg, L"Entered Handler");
		pLogger->log_func_entry(logMsg, L"AnalyticCtx_LTLS::superTickHandler");
	}

	wstring symbol = pCtx->symbol;

	try
	{
		tickCount = 0; 
		tickAmtSum = 0.0;
		tickVolSum = 0;
		slotUid = 0;

		itLA = pLookAhead->lookAheadBegin();
		it_endLA = pLookAhead->lookAheadEnd();
		activeSlots = 0;

		///////////////////////////////////////////////////////////////////////
		//	Calculate available LA slots for SuperTick processing
		///////////////////////////////////////////////////////////////////////
		while (itLA != it_endLA)
		{
			rc = pLookAhead->getTickContainer(itLA, pElemContainer);
			if (rc != ATS_C_SUCCESS)
			{
				// Report error
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
				{
					swprintf(logMsg, sizeof logMsg, L"getTickContainer() failed with code (%d) while calculating active slot count", rc);
					pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::supertickHandler");
				}
				return ATS_C_FAIL;
			}
			if (pElemContainer->state == CIRCULAR_BUFFER_STATES::EMPTY)
			{
				itLA = pLookAhead->increment(itLA);
				continue;
			}
			if (pElemContainer->state == CIRCULAR_BUFFER_STATES::BUSY)
			{
				//	Reached the end of eligible slots for processing in LA
				break;
			}
			itLA = pLookAhead->increment(itLA);
			++activeSlots;
		}
		//	Do we have enough slots to process
		if (activeSlots < (unsigned int)barLimit)
		{
			return ATS_C_SUCCESS;
		}

		///////////////////////////////////////////////////
		//	Process available bars
		///////////////////////////////////////////////////
		itLA = pLookAhead->lookAheadBegin();
		barsToProcess = activeSlots / barLimit;

		while (itLA != it_endLA && barsToProcess > 0)
		{
			rc = pLookAhead->getTickContainer(itLA, pElemContainer);
			if (rc == ATS_C_SUCCESS)
			{
				if (pElemContainer->state == CIRCULAR_BUFFER_STATES::EMPTY)
				{
					itLA = pLookAhead->increment(itLA);	//	Next one
					continue;
				}
			}
			else
			{
				// Report error
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
				{
					swprintf(logMsg, sizeof logMsg, L"getTickContainer() failed with %d", rc);
					pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::supertickHandler");
				}
				return ATS_C_FAIL;
			}
			++tickCount;

			// Aggregate data
			tickAmtSum += pElemContainer->element.trade_amt;
			tickVolSum += pElemContainer->element.trade_vol;
			if (tickCount == 1)
			{
				//	Use time of earliest in collection
				superTick.time_of_trade = pElemContainer->element.time_of_trade;
				superTick.time_added_to_buffer = pElemContainer->element.time_added_to_buffer;
				superTick.last_trade_amt = 0.0;
			}
			// Invalidate LA buffer slot
			slotUid = pElemContainer->uid;
			rc = pLookAhead->invalidateSlot(slotUid);
			if (rc != ATS_C_SUCCESS)
			{
				// Report error
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
				{
					swprintf(logMsg, sizeof logMsg, L"invalidateLASlot(%d) failed with %d", slotUid, rc);
					pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::supertickHandler");
				}
				rc = ATS_C_SUCCESS; // for now
			}
			else
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
				{
					swprintf(logMsg, sizeof logMsg, L"invalidateLASlot(%d),  symbol:  %s", slotUid, symbol.c_str());
					pLogger->log_debug(logMsg, L"AnalyticCtx_LTLS::supertickHandler");
				}
			}

			//	Check to see if we've accumulated enough ticks to construct a bar
			if (tickCount == barLimit)
			{
				// Perform final calculations and insert SuperTick into buffer 
				superTick.trade_amt = tickAmtSum / barLimit;
				superTick.trade_vol = tickVolSum / barLimit;
				superTick.last_trade_amt = pElemContainer->element.trade_amt;	//	Last trade amount in collection for reporting
				pSuperTick->insertTickData(&superTick);
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG))
				{
					swprintf(logMsg, sizeof logMsg, L"Insert into SuperTick buffer: [%s, cb_active %d]", 
								symbol.c_str(), pSuperTick->getActiveSlotCount());
					pLogger->log_debug(logMsg, L"AnalyticCtx_LTLS::supertickHandler");
				}
				// Set for next bar calculation
				tickCount = 0;
				tickAmtSum = 0.0;
				tickVolSum = 0;
				barsToProcess--;
			}

			// Get next buffer iterator
			itLA = pLookAhead->increment(itLA);
		}	//	End-While
	}
	catch (...)
	{
		//	Report error
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
		{
			swprintf(logMsg, sizeof logMsg, L"Unspecified exception");
			pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::supertickHandler");
		}
		rc = ATS_C_FAIL;
	}

	return rc;
}

ATS_CODE AnalyticCtx_LTLS::trendAnalysisHandler(DS_Active_CTX* pCtx)							//<------------------------------------------ TREND_ANALYSIS HANDLER ---------------------------------
{
	ATS_CODE rc = ATS_C_SUCCESS;

	///////////////////////////////////////////////////////
	//	Trend Analysis FSM
	//	- Process newly established SUPERTICK data
	//	- All relevant data is located in the trendData 
	//	  object
	///////////////////////////////////////////////////////
	Container_forward_iterator			itST;
	Container_forward_iterator			it_endST;
	CONTAINER_CTX_BLOCK<S_TICK_DATA>*		pSlot=NULL;
	DataAggregateXYValue	dataValue;

	double			tempLastAmt=0.0;
	double			tempDiff=0.0;
	unsigned int	dataValuesToTrim=0;
	unsigned int	activeSlots = 0;
	int				dvYCount = 0;

	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_FUNC_ENTRY))
	{
		swprintf(logMsg, sizeof logMsg, L"Entered Handler");
		pLogger->log_func_entry(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
	}

	for (;;)
	{
		rc = ATS_C_SUCCESS;
		switch (pSuperTick->trendData.patternState)
		{



		case TREND_PATTERN_STATE::INITIAL:
			///////////////////////////////////////////////////////////////////
			// Starts and initializes from fresh pivot  
			//	- Handler sets initial state & creates analytic along with 
			//	  numeric objects
			//	- For now, this is a NOOP state
			///////////////////////////////////////////////////////////////////
			pSuperTick->trendData.patternState = TREND_PATTERN_STATE::START;
			break;



		case TREND_PATTERN_STATE::START:
		case TREND_PATTERN_STATE::RESTART:
			///////////////////////////////////////////////////////////////////
			// Preliminary trend processing set-up
			//	- Clear elements of Data Aggregation Container
			//	- The intention is to start searching for a new trend
			//	  with previous aggregation data fully cleared
			//	- Collection DS, SuperTick, outlier and lookahead buffers
			//	  will not be affected by this reset and will continue
			//	  to be the source of new tick data
			///////////////////////////////////////////////////////////////////
			rc = pDAContainer->recycle();
			pSuperTick->trendData.patternState = TREND_PATTERN_STATE::ESTABLISH;
			break;



		case TREND_PATTERN_STATE::ESTABLISH:																		//<------------------------------------------ TREND_ANALYSIS HANDLER :: ESTABLISH
			/////////////////////////////////////////////////////////////////////////////////////////////////
			//	(Reentrant State)
			//	Establish the LTLS Linear Trend Pattern
			//	- ST Processing occurs on each element of ST Buffer (window is between 1 and 
			//	  NUM_SLOTS_TO_ESTABLISH_TREND). This is required in order to calculate data compression.
			//	- A minimum of MIN_ST_FOR_TREND_CALCS is required to call library routines
			//	- The earliest slot(s) is(are) invalidated in the SuperTick Buffer if no trend is 
			//	  established or broken before NUM_SLOTS_TO_ESTABLISH_TREND is met
			//	- Compression and Gaps are assessed for each new ST value - CB<> is adjusted accordingly
			//	  on each iteration
			/////////////////////////////////////////////////////////////////////////////////////////////////
			itST = pSuperTick->superTickBegin();
			it_endST = pSuperTick->superTickEnd();
			pDAContainer->state = TREND_PATTERN_STATE::ESTABLISH;

			///////////////////////////////////////////////////////////////////
			//	Fill Data Aggregate with Y-Value data from SuperTick buffer
			//	- Compression is allowed, Gaps are not
			//////////////////////////////////////////////////////////////////
			while (itST != it_endST)
			{
				pSuperTick->getTickContainer(itST, pSlot);
				if (pSuperTick->isValidSlot(itST) == FALSE)
				{
					//	Found empty slot
					itST = pSuperTick->increment(itST);
					continue;	//	while-loop
				}

				//	Collect slot data and do trend processing
				dataValue.sTickValue = pSlot->element.trade_amt;
				dataValue.lastTickValueInSet = pSlot->element.last_trade_amt;
				pDAContainer->Y_DataValues.pushBack(dataValue);	//	<-----------------Only trade amount is required from slot
				rc = pSuperTick->invalidateSlot(pSlot->uid);
				if (rc != ATS_C_SUCCESS)
				{
					//	Report error - "Establishing" trend has issue
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
					{
						swprintf(logMsg, sizeof logMsg, L"invalidateSlot(%d) failed with %d during ESTABLISH state", pSlot->uid, rc);
						pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
					}
					pSuperTick->trendData.patternState = TREND_PATTERN_STATE::RESTART;
					break;	//	for-loop - next state
				}

				dvYCount = pDAContainer->Y_DataValues.getCount();
				if (dvYCount >= param_MIN_SLOTS_FOR_TREND_CALCS)
				{
					//	Call lib function to perform trend analysis
					rc = pAELib->processTrend(pDAContainer);

					/////////////////////////////
					//	Assess result - BEGIN
					/////////////////////////////
					if (rc == ATS_C_AE_NOTREND)
					{
						if (dvYCount > param_MIN_SLOTS_FOR_TREND_CALCS)
							dataValuesToTrim = dvYCount - param_MIN_SLOTS_FOR_TREND_CALCS;
						//	Trim all above MIN_SLOTS_FOR_TREND_CALCS
						pDAContainer->Y_DataValues.popFront(dataValuesToTrim);
					}
					else if (rc == ATS_C_AE_TRENDING)
					{
						if (dvYCount >= param_NUM_SLOTS_TO_ESTABLISH_TREND)
						{
							//	Strong trend established
							pSuperTick->trendData.patternState = TREND_PATTERN_STATE::ENTER;
							break;	//	for-loop - next state
						}
					}
					else if (rc == ATS_C_AE_DATA_COMPRESS)
					{
						//	Will be trending
						//	Remove most recent value from Y-Value array	
												//	Trend broken while in establishing state
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG_2))
						{
							swprintf(logMsg, sizeof logMsg, L"TREND COMPRESS occurred while in ESTABLISH state for (%s)", pCtx->symbol.data());
							pLogger->log_debug_2(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
						}
						pDAContainer->Y_DataValues.pop();
					}
					else if (rc == ATS_C_AE_DATA_GAP)
					{
						//	Gaps are not allowed while in ESTABLISH state
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG_2))
						{
							swprintf(logMsg, sizeof logMsg, L"TREND GAP occurred while processing trend data for (%s) while in ESTABLISH state - not allowed", pCtx->symbol.data());
							pLogger->log_debug_2(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
						}
						pSuperTick->trendData.patternState = TREND_PATTERN_STATE::RESTART;
						break;
					}
					else if (rc == ATS_C_AE_TRENDCHANGE)
					{
						//	Trend broken while in establishing state
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG_2))
						{
							swprintf(logMsg, sizeof logMsg, L"TREND CHANGE occurred while in ESTABLISH state for (%s)", pCtx->symbol.data());
							pLogger->log_debug_2(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
						}
						pSuperTick->trendData.patternState = TREND_PATTERN_STATE::RESTART;
						break;
					}
					else if (rc == ATS_C_AE_TRENDLIMITOUT)
					{
						//	Trend broken due to limit-out while in establishing state
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG_2))
						{
							swprintf(logMsg, sizeof logMsg, L"TREND LIMITOUT occurred while in ESTABLISH state for (%s)", pCtx->symbol.data());
							pLogger->log_debug_2(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
						}
						pSuperTick->trendData.patternState = TREND_PATTERN_STATE::RESTART;
						break;
					}
					else
					{
						//	Trend error occurred allowed while in ESTABLISH state
						if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
						{
							swprintf(logMsg, sizeof logMsg, L"TREND processing unexpected error (%d) occurred while in ESTABLISH state for (%s)", rc, pCtx->symbol.data());
							pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
						}
						pSuperTick->trendData.patternState = TREND_PATTERN_STATE::RESTART;
						break;
					}
					/////////////////////////////
					//	Assess result - END
					/////////////////////////////
				}

				itST = pSuperTick->increment(itST);

			}	//	while-loop

			if (pSuperTick->trendData.patternState == TREND_PATTERN_STATE::ESTABLISH)
			{
				//	Need more data for continued trend processing
				rc = ATS_C_SUCCESS;
				goto trendAnalysisHandler_exit;
			}
			break;



		case TREND_PATTERN_STATE::ENTER:
			///////////////////////////////////////////////////////////////////
			// Report and notify trend data indicates a pattern entry point
			//	- Snapshot entry position data
			///////////////////////////////////////////////////////////////////
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRADE))
			{
				std::wstring trend;
				if (pDAContainer->bias == 1)
				{
					trend = L"Long";
				}
				else
				{
					trend = L"Short";
				}
				swprintf(logMsg, sizeof logMsg, L">>>>> OPEN  (%s) position:    %s  Amt %4.4f <<<<<  last_tick: %4.4f",
					trend.data(),
					pCtx->symbol.data(),
					pDAContainer->Y_DataValues.yValueArray[pDAContainer->Y_DataValues.index - 1].sTickValue,
					pDAContainer->Y_DataValues.yValueArray[pDAContainer->Y_DataValues.index - 1].lastTickValueInSet);
						//pSlot->element.last_trade_amt); 
				pLogger->log_trade(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");

				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_SPEAK))
				{
					swprintf(logMsg, sizeof logMsg, L"Open %s in %s at %4.2f",
								trend.data(), 
								pCtx->symbol.data(), 
								pDAContainer->Y_DataValues.yValueArray[pDAContainer->Y_DataValues.index - 1].sTickValue);	// was -2
					pLogger->log_speak(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
				}
			}
			pSuperTick->trendData.patternState = TREND_PATTERN_STATE::FOLLOW;
			break;



		case TREND_PATTERN_STATE::FOLLOW:																			//<------------------------------------------ TREND_ANALYSIS HANDLER :: FOLLOW
			///////////////////////////////////////////////////////////////////
			//	(Reentrant State)
			//	Process new SuperTick data - Correlate with established trend
			//	- Eliminate compressible values (slots)
			//	- Adjust prior Y_Value aggregate data when Gaps occur
			//	- Process ST buffer data until the established trend changes
			///////////////////////////////////////////////////////////////////
			itST = pSuperTick->superTickBegin();
			it_endST = pSuperTick->superTickEnd();
			pDAContainer->state = TREND_PATTERN_STATE::FOLLOW;

			///////////////////////////////////////////////////////////////////
			//	Fill Data Aggregate with Y-Value data from SuperTick buffer
			//	- Compression and Gaps are allowed
			//////////////////////////////////////////////////////////////////

			while (itST != it_endST)
			{
				pSuperTick->getTickContainer(itST, pSlot);
				if (pSuperTick->isValidSlot(itST) == FALSE)
				{
					//	Found empty slot
					itST = pSuperTick->increment(itST);
					continue;	//	while-loop - next iteration
				}

				//	Collect slot data and continue trend processing
				dataValue.sTickValue = pSlot->element.trade_amt;
				dataValue.lastTickValueInSet = pSlot->element.last_trade_amt;
				pDAContainer->Y_DataValues.pushBack(dataValue);	//	<----------------- Only trade amount is required from slot
				rc = pSuperTick->invalidateSlot(pSlot->uid);
				if (rc != ATS_C_SUCCESS)
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
					{
						swprintf(logMsg, sizeof logMsg, L"invalidateSlot(%d) failed with %d during FOLLOW state for (%s)", pSlot->uid, rc, pCtx->symbol.data());
						pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
					}
					pSuperTick->trendData.patternState = TREND_PATTERN_STATE::CLOSE;
					break;	//	for-loop - next state
				}

				dvYCount = pDAContainer->Y_DataValues.getCount();
				
				//	Call lib function to perform trend analysis
				rc = pAELib->processTrend(pDAContainer);

				/////////////////////////////
				//	Assess result - BEGIN
				/////////////////////////////
				if (rc == ATS_C_AE_NOTREND)
				{
					//	Trend is broken - Close trade, record change and notify
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
					{
						swprintf(logMsg, sizeof logMsg, L"NOTREND occurred while in FOLLOW state for (%s)", pCtx->symbol.data());
						pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
					}
					pSuperTick->trendData.patternState = TREND_PATTERN_STATE::CLOSE;
					break;	//	for-loop - next state
				}
				else if (rc == ATS_C_AE_TRENDING)
				{
					//	Check trend time duration	-	(future)
					//	- Adjust trend strength metric
					//	- Assess position continued eligibility
					;

					//	Trim trend window and data aggregates if necessary
					if (dvYCount > param_MAX_SLOTS_ON_FOLLOW)
					{
						dataValuesToTrim = dvYCount - param_MAX_SLOTS_ON_FOLLOW;
						pDAContainer->Y_DataValues.popFront(dataValuesToTrim);
					}
				}
				else if (rc == ATS_C_AE_TRENDCHANGE)
				{
					//	Trend is broken - Close trade, record change and notify
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					{
						swprintf(logMsg, sizeof logMsg, L"TREND CHANGE occurred while in FOLLOW state for (%s)", pCtx->symbol.data());
						pLogger->log_info(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
					}
					pSuperTick->trendData.patternState = TREND_PATTERN_STATE::CLOSE;
					break;	//	for-loop - next state
				}
				else if (rc == ATS_C_AE_TRENDLIMITOUT)
				{
					//	Trend is broken due to limit-out - Close trade, record change and notify
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					{
						swprintf(logMsg, sizeof logMsg, L"TREND LIMITOUT occurred while in FOLLOW state for (%s)", pCtx->symbol.data());
						pLogger->log_info(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
					}
					pSuperTick->trendData.patternState = TREND_PATTERN_STATE::CLOSE;
					break;	//	for-loop - next state
				}

				else if (rc == ATS_C_AE_DATA_COMPRESS)
				{
					//	Will be trending
					//	Remove most recent value from Y-Value array	
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_DEBUG_2))
					{
						swprintf(logMsg, sizeof logMsg, L"TREND COMPRESS occurred while in FOLLOW state for (%s)", pCtx->symbol.data());
						pLogger->log_debug_2(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
					}
					pDAContainer->Y_DataValues.pop();
				}
				else if (rc == ATS_C_AE_DATA_GAP)
				{
					//	Check trend time duration	-	(future)
					//	- Adjust trend strength metric
					//	- Assess position continued eligibility
					;

					//	Trim trend window and data aggregates if necessary
					if (dvYCount > param_MAX_SLOTS_ON_FOLLOW)
					{
						dataValuesToTrim = dvYCount - param_MAX_SLOTS_ON_FOLLOW;
						pDAContainer->Y_DataValues.popFront(dataValuesToTrim);
					}
					//	Removes latest Y Data Value and adjusts high and low values
					pDAContainer->Y_DataValues.pop();
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					{
						swprintf(logMsg, sizeof logMsg, L"TREND GAP occurred while in FOLLOW state for (%s)", pCtx->symbol.data());
						pLogger->log_info(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
					}
				}
				else
				{
					//	Processing Error
					//	Close open position and restart
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
					{
						swprintf(logMsg, sizeof logMsg, L"Processing error while in FOLLOW state for (%s)", pCtx->symbol.data());
						pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
					}
					pSuperTick->trendData.patternState = TREND_PATTERN_STATE::CLOSE;
					break;	//	next for-loop state
				}
				/////////////////////////////
				//	Assess result - END
				/////////////////////////////

				rc = ATS_C_SUCCESS;
				itST = pSuperTick->increment(itST);
			}	//	while-loop

			//	Check state
			if (pSuperTick->trendData.patternState == TREND_PATTERN_STATE::FOLLOW)
			{
				//	Need more data
				rc = ATS_C_SUCCESS;
				goto trendAnalysisHandler_exit;
			}
			break;



		case TREND_PATTERN_STATE::CLOSE:
			///////////////////////////////////////////////////////////////////
			// Trend following has terminated
			//	- Trend data indicates a pattern exit point - finalization
			//	- Log event
			///////////////////////////////////////////////////////////////////
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRADE))
			{
				std::wstring trend;
				if (pDAContainer->bias == 1)
				{
					trend = L"Long";
				}
				else
				{
					trend = L"Short";
				}
				//	Close trade using last value before trend break
				swprintf(logMsg, sizeof logMsg, L">>>>> CLOSE (%s) position:    %s  Amt %4.4f <<<<<  last_tick: %4.4f",
					trend.data(),
					pCtx->symbol.data(),
					pDAContainer->Y_DataValues.yValueArray[pDAContainer->Y_DataValues.index - 1].sTickValue,
					pDAContainer->Y_DataValues.yValueArray[pDAContainer->Y_DataValues.index - 1].lastTickValueInSet);	//pSlot->element.last_trade_amt);	// was -2
				pLogger->log_trade(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");

				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_SPEAK))
				{
					swprintf(logMsg, sizeof logMsg, L"Close %s in %s at %4.2f",
							trend.data(), 
							pCtx->symbol.data(), 
							pDAContainer->Y_DataValues.yValueArray[pDAContainer->Y_DataValues.index - 1].sTickValue);	// was -2
					pLogger->log_speak(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
				}
			}
			pSuperTick->trendData.patternState = TREND_PATTERN_STATE::EXIT;
			break;



		case TREND_PATTERN_STATE::EXIT:
			///////////////////////////////////////////////////////////////////
			// Reporting and notification
			//	- Create a DB audit Snapshot of position data
			//	- Send event notification
			//	- Cycle to RESTART and start over
			///////////////////////////////////////////////////////////////////

			//	Add steps mentioned in comments

			pSuperTick->trendData.patternState = TREND_PATTERN_STATE::RESTART;
			rc = ATS_C_SUCCESS;
			goto trendAnalysisHandler_exit;
			break;



		default:
			///////////////////////////////////////////////////////////////////
			// Report error
			///////////////////////////////////////////////////////////////////
			if (pSuperTick->trendData.patternState == TREND_PATTERN_STATE::FOLLOW)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
				{
					swprintf(logMsg, sizeof logMsg, L"Processing error - Entered FSM default state. Attempting to close open position");
					pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
				}
				pSuperTick->trendData.patternState = TREND_PATTERN_STATE::CLOSE;
			}
			else
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
				{
					swprintf(logMsg, sizeof logMsg, L"Processing error - Entered FSM default state. No open position reported.");
					pLogger->log_warn(logMsg, L"AnalyticCtx_LTLS::trendAnalysisHandler");
				}
				pSuperTick->trendData.patternState = TREND_PATTERN_STATE::RESTART;
			}
			pSuperTick->emptyBuffer();
			rc = ATS_C_SUCCESS;
			goto trendAnalysisHandler_exit;
			break;

		}	//	End Switch


	}	//	End for-loop

	trendAnalysisHandler_exit:
	return rc;
}


ATS_CODE AnalyticCtx_LTLS::reportHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_LTLS::notificationHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}
///////////////////////////////////////////////////////////
//	AnalyticCtx_ETLS
//	- Exponential Trend Long / Short analysis
///////////////////////////////////////////////////////////
AnalyticCtx_ETLS::AnalyticCtx_ETLS()
{
	ATS_CODE rc;
	rc = initialize();
}

AnalyticCtx_ETLS::~AnalyticCtx_ETLS()
{

}

ATS_CODE AnalyticCtx_ETLS::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	return rc;
}

ATS_CODE AnalyticCtx_ETLS::invokeHandlers(unsigned int db_seq, DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;


	return rc;
}

ATS_CODE AnalyticCtx_ETLS::lookaheadHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_ETLS::outlierHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_ETLS::dataSmoothingHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_ETLS::supertickHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_ETLS::trendAnalysisHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_ETLS::reportHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}

ATS_CODE AnalyticCtx_ETLS::notificationHandler(DS_Active_CTX* pCtx)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Add test code and trace logging

	return rc;
}


///////////////////////////////////////////////////////////
//	AnalyticLookAheadBuffer
///////////////////////////////////////////////////////////
AnalyticLookAheadBuffer::AnalyticLookAheadBuffer()
{
	ATS_CODE rc;
	rc = initialize();
}


AnalyticLookAheadBuffer::~AnalyticLookAheadBuffer()
{

}

ATS_CODE AnalyticLookAheadBuffer::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	lastActiveDSslotUid = 0;
	pivotAmt = 0;
	percentDiff = param_MAX_OUTLIER_DELTA;	//	Percentage above last trade value to identify outlier blip
	outlierStack.empty();
	outlierLimit = param_OUTLIER_COLLECTION_SIZE;	//	This count or beyond is identified as a Gap
	return rc;
}

Container_forward_iterator AnalyticLookAheadBuffer::lookAheadBegin()
{
	return(lookAheadBuffer.begin());
}

Container_forward_iterator AnalyticLookAheadBuffer::lookAheadEnd()
{
	return(lookAheadBuffer.end());
}

ATS_CODE AnalyticLookAheadBuffer::getPivotAmt(double& amt)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	amt = pivotAmt;
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::setPivotAmt(double& amt)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	pivotAmt = amt;
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::getLastUid(Container_element_uid& uid)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	uid = lastUid;
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::setLastUidProcessed(Container_element_uid& uid)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	lastUid = uid;
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::setUidUnused(Container_element_uid& uid)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	rc = lookAheadBuffer.invalidate_entry(uid);
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::lookupUid(Container_forward_iterator& it, unsigned int& uid)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	CONTAINER_CTX_BLOCK<TICK_DATA>* pCtx = NULL;
	rc = lookAheadBuffer.convert_forward_iterator_to_addr(it, pCtx);
	if (rc == ATS_C_SUCCESS)
	{
		// Found it
		uid = pCtx->uid;
	}
	else
	{
		uid = 0;
	}
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::invalidateSlot(Container_element_uid& uid)
{
	ATS_CODE rc;
	rc = lookAheadBuffer.invalidate_entry(uid);
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::setSlotState(Container_element_uid& uid, CIRCULAR_BUFFER_STATES state)
{
	ATS_CODE rc;
	rc = lookAheadBuffer.set_element_state_by_uid(uid, state);
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::convertUidToIterator(Container_element_uid& uid, Container_forward_iterator& it)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	rc = lookAheadBuffer.find_element_by_uid(uid, it);
	return rc;
}

int AnalyticLookAheadBuffer::getLookAheadCount()
{
	return (lookAheadBuffer.num_active_elements());
}

ATS_CODE AnalyticLookAheadBuffer::getTickCount(unsigned int& count)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	count = lookAheadBuffer.num_active_elements();
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::getTickContainer(Container_forward_iterator& it, CONTAINER_CTX_BLOCK<TICK_DATA>*& pElemContainer)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	CONTAINER_CTX_BLOCK<TICK_DATA>* pCtx = NULL;
	rc = lookAheadBuffer.convert_forward_iterator_to_addr(it, pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		pCtx = NULL;
		return rc;
	}
	pElemContainer = pCtx;
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::getUidContainer(Container_element_uid& uid, CONTAINER_CTX_BLOCK<TICK_DATA>*& pElemContainer)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	CONTAINER_CTX_BLOCK<TICK_DATA>* pCtx = NULL;
	rc = lookAheadBuffer.convert_uid_to_addr(uid, pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		pCtx = NULL;
		return rc;
	}
	pElemContainer = pCtx;
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::getTickContext(Container_forward_iterator& it, TICK_DATA*& pTickData)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	CONTAINER_CTX_BLOCK<TICK_DATA>* pCtx = NULL;
	rc = lookAheadBuffer.convert_forward_iterator_to_addr(it, pCtx);
	if (rc != ATS_C_SUCCESS)
	{
		pCtx = NULL;
		return rc;
	}
	pTickData = &pCtx->element;
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::insertTickData(TICK_DATA* tickData)
{
	ATS_CODE rc;
	rc = lookAheadBuffer.insert_back(tickData);
	return rc;
}

ATS_CODE AnalyticLookAheadBuffer::findSuccessorUid(Container_element_uid& uid, Container_element_uid& successorUid)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	CONTAINER_CTX_BLOCK<TICK_DATA>* pCtx = NULL;
	rc = lookAheadBuffer.find_successor_uid(uid, successorUid);
	return rc;
}

Container_forward_iterator AnalyticLookAheadBuffer::increment(Container_forward_iterator& it)
{
	return (lookAheadBuffer.container_iterator_incr(it));
}

///////////////////////////////////////////////////////////
//	SuperTickBuffer
///////////////////////////////////////////////////////////
AnalyticSuperTickBuffer::AnalyticSuperTickBuffer()
{
	ATS_CODE rc;
	rc = initialize();
}


AnalyticSuperTickBuffer::~AnalyticSuperTickBuffer()
{

}

ATS_CODE AnalyticSuperTickBuffer::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	return rc;
}

Container_forward_iterator AnalyticSuperTickBuffer::superTickBegin()
{
	return(superTickBuffer.begin());
}

Container_forward_iterator AnalyticSuperTickBuffer::superTickEnd()
{
	return(superTickBuffer.end());
}

int AnalyticSuperTickBuffer::getActiveSlotCount(void)
{
	return (superTickBuffer.num_active_elements());
}

ATS_CODE AnalyticSuperTickBuffer::getActiveSlotCount(unsigned int& activeSlots)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	activeSlots = superTickBuffer.num_active_elements();
	return rc;
}

ATS_CODE AnalyticSuperTickBuffer::getTickContainer(Container_forward_iterator& it, CONTAINER_CTX_BLOCK<S_TICK_DATA>*& pElemContainer)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	CONTAINER_CTX_BLOCK<S_TICK_DATA>* pCtx = NULL;
	rc = superTickBuffer.convert_forward_iterator_to_addr(it, pCtx); 
	if (rc != ATS_C_SUCCESS)
	{
		pCtx = NULL;
		return rc;
	}
	pElemContainer = pCtx;
	return rc;
}

ATS_CODE AnalyticSuperTickBuffer::insertTickData(S_TICK_DATA* tickData)
{
	ATS_CODE rc;
	rc = superTickBuffer.insert_back(tickData);
	return rc;
}

ATS_CODE AnalyticSuperTickBuffer::invalidateSlot(Container_element_uid& uid)
{
	ATS_CODE rc;
	rc = superTickBuffer.invalidate_entry(uid);
	return rc;
}

bool AnalyticSuperTickBuffer::isValidSlot(Container_forward_iterator& it)	//	<-----------------Validate this routine
{
	ATS_CODE rc;
	bool bRet;
	rc = superTickBuffer.isActiveElement(it, bRet);
	if (rc == ATS_C_SUCCESS)
		return bRet;
	else
		return FALSE;
}

ATS_CODE AnalyticSuperTickBuffer::trimFront(unsigned int slots)
{
	ATS_CODE rc;
	rc = superTickBuffer.trim_front(slots);
	return rc;
}

ATS_CODE AnalyticSuperTickBuffer::emptyBuffer(void)
{
	ATS_CODE rc;
	rc = superTickBuffer.empty();
	return rc;
}

Container_forward_iterator AnalyticSuperTickBuffer::increment(Container_forward_iterator& it)
{
	return (superTickBuffer.container_iterator_incr(it));
}

ATS_CODE AnalyticSuperTickBuffer::convertUID(Container_element_uid& uid, Container_forward_iterator& it)
{
	return (superTickBuffer.find_element_by_uid(uid, it));
}


///////////////////////////////////////////////////////////
//	TrendProperties
//	- LifeCycle managed by SuperTickBuffer
///////////////////////////////////////////////////////////
TrendProperties::TrendProperties()
{
	ATS_CODE rc;
	rc = initialize();
}


TrendProperties::~TrendProperties()
{

}

ATS_CODE TrendProperties::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	patternState = TREND_PATTERN_STATE::INITIAL;

	return rc;
}





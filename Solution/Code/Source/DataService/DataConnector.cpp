////////////////////////////////////////////////////////////////////////////////
//
//	Data Connector
//	- Currently supports ActiveTick provider
//
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DataConnector.h"
#include <string>
#include <vector>


using namespace ATS_DATASERVICE;


//	(Extern) Initialize and make visible in Module
bool DataConnector::bConnectorEnabled = false;
bool DataConnector::bConnected=false;
bool DataConnector::bLoggedin = false;
wstring last_ATLoginResponseCallback_msg = L"";
wstring last_AT_SessionStatusChangeCallback_msg = L"";


////////////////////////////////////////////////////////////////////////////////
//	Runnable thread
//	- Manages connection to data provider
//	- Performs housekeeping operations on the raw data queue 
////////////////////////////////////////////////////////////////////////////////
unsigned DataConnectorThreads::ConnectorManager(void* Args)
{
	//
	//	Loop on raw data processing thread and system status
	//
	DS_Globals* pGlobals = (DS_Globals*)Args;
	DS_Shared* pSDShare = (DS_Shared*)pGlobals->Args[ARG_SDS];
	LoggerX* pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	DataConnector* pDataConn = (DataConnector*)pGlobals->Args[ARG_CONN];

	HRESULT hRC = S_OK;
	int sysState;
	int subSysState;
	int	sleepTimeMS = 1000;
	
	
	for (;;)
	{
		//	ConnectorManager
		Sleep(sleepTimeMS);

		sysState = g_pStates->DS_SYSTEM_ST;
		subSysState = g_pStates->DS_CONNECTOR_ST;

		switch (subSysState)
		{
		case ATS_DATASERVICE::RUN_ST:
			///////////////////////////////////////////////////////////////////
			//	Perform necessary housekeeping
			///////////////////////////////////////////////////////////////////
			if (pDataConn->isConnectorEnabled() == true) {

				//	Keep track of connection state (provider callback will set this value)
				//if (pDataConn->isbATProvConnected() == false)
				if (pDataConn->isbATProvConnected() == false)
				{
					//	Recover connection to provider
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
						pLogger->log_trace(L"Connection to AT service provider lost - restart connection", L"DataConnectorThreads::ConnectorManager");
					//FIX: g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::RESTART_ST;
				}
			}
			else
			{
				g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::DORMANT_ST;
			}
			break;

		case ATS_DATASERVICE::START_ST:
			//	
			if (pDataConn->isConnectorEnabled() == true)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
					pLogger->log_trace(L"AT Provider Connector subsystem is starting", L"DataConnectorThreads::ConnectorManager");
				g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::INPROGESS_UP1_ST;
			}
			else
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
					pLogger->log_trace(L"AT Provider Connector subsystem is ***disabled***", L"DataConnectorThreads::ConnectorManager");
				g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::RUN_ST;
			}
			break;

		case ATS_DATASERVICE::RESTART_ST:
			//	Reconnects in case provider connection drops
			if (true)
			{

			}

			break;

		case ATS_DATASERVICE::INPROGESS_UP1_ST:
			//	Perform preliminary connection step
			if (pDataConn->isbATProvConnected() == false)
			{
				if (pDataConn->connectToService() == ATS_C_SUCCESS)
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
						pLogger->log_trace(L"Connecting to AT provider service", L"DataConnectorThreads::ConnectorManager");
					g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::INPROGESS_UP2_ST;
				}
				else
				{
					//	Recycles if failure ??
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
						pLogger->log_warn(L"Unable to connect to AT provider service", L"DataConnectorThreads::ConnectorManager");
					g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::SHUTDOWN_ST;
				}
			}
			break;

		case ATS_DATASERVICE::INPROGESS_UP2_ST:
			//	Wait for connection callback and issue service login
			if (pDataConn->isbATProvConnected() == true)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
				{
					pLogger->log_trace(L"Connected to AT provider service", L"DataConnectorThreads::ConnectorManager");
					pLogger->log_trace(L"Login to AT provider service", L"DataConnectorThreads::ConnectorManager");
				}
				pDataConn->loginToService();
				g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::INPROGESS_UP3_ST;
				continue;
			}
			//	FIX: query connection timeout
			break;

		case ATS_DATASERVICE::INPROGESS_UP3_ST:
			//	Create and initialize the Streamer object
			if (pDataConn->isbATProvLoggedin() == true)
			{
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					pLogger->log_info(L"Logged into AT provider service", L"DataConnectorThreads::ConnectorManager");
				//	Create the streamer object
				try {
					pDataConn->pStreamer = new AT_Streamer(Args, pDataConn->connCtx.hSession);
					g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::INPROGESS_UP4_ST;
				}
				catch (...)
				{
					if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
						pLogger->log_error(L"Unable to create AT_Streamer object", L"DataConnectorThreads::ConnectorManager");
					g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::SHUTDOWN_ST;
				}
				continue;
			}
			//	FIX: how long should we wait until login failure?
			break;

		case ATS_DATASERVICE::INPROGESS_UP4_ST:
			{
				// Setup/send streaming trade subscription
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					pLogger->log_info(L"Initiating AT provider Quote Stream request", L"DataConnectorThreads::ConnectorManager");


				// Build the symbol list from WatchPoolItems
				wstring symbolListW;// = L"IBM,AMZN,AAPL,MSFT"; // just set to one symbol for now
				DataConnector::getSymbolList(pSDShare, symbolListW);
				
				// Convert to string
				string symbolList;
				symbolList = AT_Helper::ws2s(symbolListW);
				// Convert the list to vector
				vector<ATSYMBOL> at_symbols;
				at_symbols = AT_Helper::StringToATSymbols(symbolList);

				// Issue SendATQuoteStreamRequest
				ATStreamRequestType requestType = StreamRequestSubscribeTradesOnly;
				unsigned int tmo = 5000; // Timeout in milliseconds
				Sleep(5000);
				uint64_t request = pDataConn->pStreamer->SendATQuoteStreamRequest(at_symbols.data(), (uint16_t)at_symbols.size(), requestType, tmo);

				// Save connector start time and log event
				pSDShare->providerConnectionTime = std::chrono::system_clock::now();
				if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
					pLogger->log_info(L"Connector subsystem has started", L"DataConnectorThreads::ConnectorManager");
				g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::RUN_ST;
			}
			break;

		case ATS_DATASERVICE::SHUTDOWN_ST:
			// Set state for dependent subsystem(s)
			
			break;

		case ATS_DATASERVICE::INPROGRESS_DOWN1_ST:
			//	if the subsystem is not down then go into shutdown mode
			if (subSysState != ATS_DATASERVICE::DOWN_ST)
			{
				//	Unbind from provider
				pDataConn->disconnectFromService(); // FIX: check rc

				// if all went well then subsystem state gets set to DOWN_ST
				g_pStates->DS_CONNECTOR_ST = ATS_DATASERVICE::DOWN_ST;
			}
			break;

		case ATS_DATASERVICE::DOWN_ST:
			//	Log shutdown message && exit the thread
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
				pLogger->log_info(L"Connector subsystem is stopped", L"DataConnectorThreads::ConnectorManager"); 
			_endthreadex(hRC);
			break;

		case ATS_DATASERVICE::IDLE_ST:
			//	Idempotent
			break;

		case ATS_DATASERVICE::DORMANT_ST:
			//	Idempotent
			break;

		default:
			//	NOOP
			break;

		}
	}


}



///////////////////////////////////////////////////////////////////////////////
//	Data Connector Definitions
///////////////////////////////////////////////////////////////////////////////
DataConnector::DataConnector(void* Args) : ActiveTickServerRequestor(connCtx.hSession)
{
	int rc = initialize(Args);
}


DataConnector::~DataConnector()
{
}


ATS_CODE DataConnector::initialize(void* Args)
{
	//	Process Args input parameters
	//	- Objects are declared in PropsX::DS_globals
	//	- DS_Globals is created in PropsX
	//	- LoggerX is created in DataManager
	try
	{
		pGlobals = (DS_Globals*)Args;
		pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];


		///////////////////////////////////////////////////////////////////////
		//	Setup the Connection Block
		///////////////////////////////////////////////////////////////////////
		initConnCtxBlock();

	}
	catch (...)
	{
		setATProvConnected(false);
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
			pLogger->log_info(L"Failed to initialize DataConnector", L"DataConnector::initialize");
	}

	return (ATS_C_SUCCESS);
}


//
//	Check Connector Enabled State
//
bool DataConnector::isConnectorEnabled()
{
	return (connCtx.enabled);
}


//
//	Set service connection
//	Returns previous value
//
bool DataConnector::setATProvConnected(bool bValue)
{
	bool previous = bValue;
	bConnected = bValue;
	return (previous);
}


//
//	Check for service connection
//
bool DataConnector::isbATProvConnected()
{
	return (bConnected);
}


//	Set service login state
//	Returns previous value
//
bool DataConnector::setATProvLoggedin(bool bValue)
{
	bool previous = bValue;
	bLoggedin = bValue;
	return (previous);
}


//
//	Check for service login state
//
bool DataConnector::isbATProvLoggedin()
{
	return (bLoggedin);
}

// 
//	Initializes Provider Service Connection and Login information
//
ATS_CODE DataConnector::initConnCtxBlock()
{
	(_wcsicmp(pGlobals->pProps->prop_ATenabled.c_str(), L"true") == 0) ? connCtx.enabled = true : connCtx.enabled = false;
	(_wcsicmp(pGlobals->pProps->prop_ATDisableRDQInsertion.c_str(), L"true") == 0) ? connCtx.disableRDQInsertion = true : connCtx.disableRDQInsertion = false;

	connCtx.GUID_KEY = AT_Helper::ws2s(pGlobals->pProps->prop_ATguid_key);
	connCtx.primaryServerHostname = AT_Helper::ws2s(pGlobals->pProps->prop_ATprimary_server);
	connCtx.secondaryServerHostname = AT_Helper::ws2s(pGlobals->pProps->prop_ATsecondary_server);
	connCtx.serverPort = _wtoi(pGlobals->pProps->prop_ATserver_port.c_str());
	(_wcsicmp(pGlobals->pProps->prop_ATuse_internal_queue.c_str(), L"true") == 0) ? connCtx.isUsingInternalQueue = true : connCtx.isUsingInternalQueue = false;
	connCtx.userid = pGlobals->pProps->prop_ATuserid.c_str();
	connCtx.password = pGlobals->pProps->prop_ATpassword.c_str();
	connCtx.callbackCycleDelay = _wtoi(pGlobals->pProps->prop_ATCallbackCycleDelay.c_str());
	connCtx.trackStatsForSymbol = pGlobals->pProps->prop_ATTrackStatsForSymbol.c_str();

	return ATS_C_SUCCESS;
}


// 
//	Provider Service Bind Routine
//
ATS_CODE DataConnector::connectToService()
{
	bool b_rc;

	//	Create the AT session
	connCtx.hSession = NULL;
	connCtx.hLoginRequest = 0;

	//	Initialize the AT Provider API
	b_rc = ATInitAPI();
	connCtx.apiVersion = ATGetAPIVersion();

	//	Create and initialize an AT Provider session
	connCtx.hSession = ATCreateSession();

	//	Set API user ID based on GUID
	ATGUID at_guid = AT_Helper::StringToATGuid(connCtx.GUID_KEY);
	b_rc = ATSetAPIUserId(connCtx.hSession, &at_guid);

	//	Set the stream update callback
	//	Note: This callback is not currently required for stream trades - may be used in the future
	//b_rc = ATSetStreamUpdateCallback(connCtx.hSession, ATStreamUpdateCallback);

	//	Initialize an AT Provider session
	b_rc = ATInitSession(
				connCtx.hSession, 
				connCtx.primaryServerHostname.c_str(), 
				connCtx.secondaryServerHostname.c_str(), 
				connCtx.serverPort, 
				AT_SessionStatusChangeCallback, 
				connCtx.isUsingInternalQueue);

	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
		pLogger->log_trace(L"Initiate API Session connect", L"AT_DataConnector_Routines::connectToService");

	return ATS_C_SUCCESS;
}


// 
//	Provider Service Bind Routine
//	- Session initialization must complete successfully prior to this call
//
ATS_CODE DataConnector::loginToService()
{
	bool b_rc;

	//	Create and send Login request - check callbacks for status
	connCtx.hLoginRequest = ATCreateLoginRequest(connCtx.hSession, connCtx.userid, connCtx.password, ATLoginResponseCallback);
	b_rc = ATSendRequest(connCtx.hSession, connCtx.hLoginRequest, /*DEFAULT_REQUEST_TIMEOUT*/ 30000, ATRequestTimeoutCallback);
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
		pLogger->log_trace(L"Initiate API Session Login", L"AT_DataConnector_Routines::loginToService");

	return ATS_C_SUCCESS;
}


// 
//	Service Unbind Routine
//
ATS_CODE DataConnector::disconnectFromService()
{
	bool b_rc;

	//	Shutdown provider
	b_rc = ATShutdownAPI();
	ATShutdownSession(connCtx.hSession);
	setATProvConnected(false);
	setATProvLoggedin(false);

	//	Disconnect from provider
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
		pLogger->log_info(L"ActiveTick provider service is disconnected", L"DataConnector::disconnectFromService");
	return(ATS_C_SUCCESS);
}


// 
//	Subscription Symbol List Build Routine
//
/*static*/ ATS_CODE DataConnector::getSymbolList(DS_Shared* pSDShare, wstring& symbolList)
{
	//	Get full list of symbols from WatchPool
	for (map<std::wstring, WatchPoolItem*>::iterator it=pSDShare->pWatchPool->watchPool.begin(); it!=pSDShare->pWatchPool->watchPool.end(); it++)
	{
		symbolList += it->first + L",";
	}
	if (!symbolList.empty())
		symbolList.back() = 0;
	return ATS_C_SUCCESS;
}
////////////////////////
////////////////////////
////////////////////////





///////////////////////////////////////////////////////////////////////////////
//	AT Request Timeout Callback Routine
//	Returns original request handle
///////////////////////////////////////////////////////////////////////////////
void DataConnector::ATRequestTimeoutCallback(
	uint64_t hOrigRequest)
{

	return;
}


///////////////////////////////////////////////////////////////////////////////
//	AT Session Status Change Callback Routine
//	Statuses:
//		Connected
//		Disconnected
//		??
///////////////////////////////////////////////////////////////////////////////
void DataConnector::AT_SessionStatusChangeCallback(
	uint64_t session,
	ATSessionStatusType statusType)
{
	wstring strConnectionResponseType;
	//DataConnector::bConnected = false;
	//	Check session status and change the connector state if necessary

	switch (statusType)
	{
	case SessionStatusConnected: strConnectionResponseType = L"SessionStatusConnected"; DataConnector::bConnected = true; break;
	case SessionStatusDisconnected: strConnectionResponseType = L"SessionStatusDisconnected"; break;
	case SessionStatusDisconnectedDuplicateLogin: strConnectionResponseType = L"SessionStatusDisconnectedDuplicateLogin"; break;
	default: strConnectionResponseType = L"Unknown response type"; break;
	}
	//last_AT_SessionStatusChangeCallback_msg = strConnectionResponseType;

	return;
}


///////////////////////////////////////////////////////////////////////////////
//	AT Login Callback Routine
//	Statuses:
//		Connected
//		Disconnected
//		??
///////////////////////////////////////////////////////////////////////////////
void DataConnector::ATLoginResponseCallback(
	uint64_t session,
	uint64_t request,
	LPATLOGIN_RESPONSE pResponse)
{
	//	Check Login status and change the connector state if necessary

	wstring strLoginResponseType;
	switch (pResponse->loginResponse)
	{
	case LoginResponseSuccess: strLoginResponseType = L"LoginResponseSuccess"; DataConnector::bLoggedin = true;  break;
	case LoginResponseInvalidUserid: strLoginResponseType = L"LoginResponseInvalidUserid"; break;
	case LoginResponseInvalidPassword: strLoginResponseType = L"LoginResponseInvalidPassword"; break;
	case LoginResponseInvalidRequest: strLoginResponseType = L"LoginResponseInvalidRequest"; break;
	case LoginResponseLoginDenied: strLoginResponseType = L"LoginResponseLoginDenied"; break;
	case LoginResponseServerError: strLoginResponseType = L"LoginResponseServerError"; break;
	default: strLoginResponseType = L"Unknown response type"; break;
	}
	//last_ATLoginResponseCallback_msg = strLoginResponseType;
	return;
}


///////////////////////////////////////////////////////////////////////////////
//	AT Streaming Update Callback Routine
//	- This function is called for each stream update
///////////////////////////////////////////////////////////////////////////////
void DataConnector::ATStreamUpdateCallback(
	LPATSTREAM_UPDATE pStreamUpdate)
{
	//	Defined - not currently used
	

	return;
}







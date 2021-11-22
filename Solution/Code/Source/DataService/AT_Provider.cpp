///////////////////////////////////////////////////////////////////////////////
//	Contains all ActiveTick (AT) Provider communications and management routines
//	- Initialize the Provider
//	- Setup the Session and register Callback routine
//	- Register the Streaming Quote Callback routine
//	- Communicate commands to the Provider
//	- Manage state of Provider session/connection
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AT_Provider.h"


//	Initialize statics
bool AT_DataConnector_Routines::bConnected=false;



/* Private */
AT_DataConnector_Routines::AT_DataConnector_Routines() : ActiveTickServerRequestor(hSession)
{

}


AT_DataConnector_Routines::AT_DataConnector_Routines(void* Args) : ActiveTickServerRequestor(hSession)
{
	initialize(Args);
}


AT_DataConnector_Routines::~AT_DataConnector_Routines()
{
	ATShutdownAPI();
}


///////////////////////////////////////////////////////////////////////////////
//	Initialize the AT Provider, Create a session and register the callbacks
///////////////////////////////////////////////////////////////////////////////
ATS_CODE AT_DataConnector_Routines::initialize(void* Args)
{
	//	Initialize the callback block
	pGlobals = (DS_Globals*)Args;
	pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];

//	bool b_rc;

#ifdef _UNUSED_CODE_
	//	Create the AT session
	//bConnected = false;
	hSession = NULL;
	const char* primaryServerHostname = "activetick1.activetick.com";
	const char* backupServerHostname = "activetick2.activetick.com";
	uint32_t 	serverPort = 443;
	bool isUsingInternalQueue = false;
	uint64_t hRequest;

	string GUID_KEY = "b3872d61dcc345a28f2b9aee5e9753d6"; 
	ATGUID guidApiUserid;
	const wchar16_t * 	userid = L"aris.athens@athelon.com";
	const wchar16_t * 	password = L"!Athe6572";

	//	Initialize the AT Provider API
	b_rc = ATInitAPI();


	//	Create and initialize an AT Provider session
	hSession = ATCreateSession();

	//	Set API user ID based on GUID
	at_guid = AT_Helper::StringToATGuid(GUID_KEY);
	b_rc = ATSetAPIUserId(hSession, &guidApiUserid);

	//	Set the stream update callback
	b_rc = ATSetStreamUpdateCallback(hSession, ATStreamUpdateCallback);

	//	Initialize an AT Provider session
	b_rc = ATInitSession(hSession, primaryServerHostname, backupServerHostname, serverPort, AT_SessionStatusChangeCallback, isUsingInternalQueue);

	//	Create Login request
	hRequest = ATCreateLoginRequest(hSession, userid, password, ATLoginResponseCallback);

	//	Setup and send Login Request
	b_rc = ATSendRequest(hSession, hRequest, DEFAULT_REQUEST_TIMEOUT, NULL);

	pLogger->trace(L"Session Connected", L"AT_DataConnector_Routines::initialize");

#endif _UNUSED_CODE_
	return ATS_C_SUCCESS;
}

#ifdef _UNUSED_CODE_
///////////////////////////////////////////////////////////////////////////////
//	AT Session Status Change Callback Routine
//	Statuses:
//		Connected
//		Disconnected
//		??
///////////////////////////////////////////////////////////////////////////////
void AT_DataConnector_Routines::AT_SessionStatusChangeCallback(
	uint64_t session,
	ATSessionStatusType statusType)
{
	//	Check session status and change the connector state if necessary
	switch (statusType)
	{
	case SessionStatusConnected: 
		//bATProviderConnectionState = true;
		//pLogger->info(L"Session Connected", L"AT_DataConnector_Routines::AT_SessionStatusChangeCallback");
		AT_DataConnector_Routines::bConnected = true;
		break;
	case SessionStatusDisconnected: 
		//bATProviderConnectionState = false;
		//pLogger->warn(L"Session Disconnected", L"AT_DataConnector_Routines::AT_SessionStatusChangeCallback");
		AT_DataConnector_Routines::bConnected = false;
		break;
	case SessionStatusDisconnectedDuplicateLogin: 
		//bATProviderConnectionState = false;
		//pLogger->warn(L"Duplicate Login", L"AT_DataConnector_Routines::AT_SessionStatusChangeCallback");
		AT_DataConnector_Routines::bConnected = false;
		break;
	default: 
		//bATProviderConnectionState = false;
		//pLogger->warn(L"Session Login - Unknown State", L"AT_DataConnector_Routines::AT_SessionStatusChangeCallback");
		AT_DataConnector_Routines::bConnected = false;
		break;
	}

	// FIX: bConnected must be conveyed to connector so housekeeping can change the state if necessary
	//		Connector manager must be able to reference a static to set the current state

	return;
}


///////////////////////////////////////////////////////////////////////////////
//	AT Login Callback Routine
//	Statuses:
//		Connected
//		Disconnected
//		??
///////////////////////////////////////////////////////////////////////////////
void AT_DataConnector_Routines::ATLoginResponseCallback(
	uint64_t session,
	uint64_t request,
	LPATLOGIN_RESPONSE pResponse)
{
	//	Check Login status and change the connector state if necessary

	return;
}


///////////////////////////////////////////////////////////////////////////////
//	AT Streaming Update Callback Routine
//	- This function is called for each stream update
///////////////////////////////////////////////////////////////////////////////
void AT_DataConnector_Routines::ATStreamUpdateCallback(
	LPATSTREAM_UPDATE pStreamUpdate)
{


	return;
}


///////////////////////////////////////////////////////////////////////////////
//	AT Streaming Trade Callback Routine
//	- This function is called for each trade AT provider trade event updates
///////////////////////////////////////////////////////////////////////////////
/* virtual */ void AT_DataConnector_Routines::OnATStreamTradeUpdate(LPATQUOTESTREAM_TRADE_UPDATE pUpdate)
{
	//	Unpack trade data and convert to trade event


	//	Add trade event to the circular buffer


	//	Perform buffer cleanup


	return;
}

#endif _UNUSED_CODE_



///////////////////////////////////////////////////////////////////////////////
//	Data Service - Data Connector
//	- Support for provider services
//	- Implements ActiveTick data provider
//	- May include others in the future
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../Utils/Includes/CommonX.h"
#include "../Utils/Includes/PropsX.h"
#include "../Utils/Includes/StructX.h"


#include "DS_Shared.h"

#include "AT_Helper.h"
#include "AT_Streamer.h"
#include "../AT/Shared/ATServerAPIDefines.h"
#include "../AT/ActiveTickServerAPI/ActiveTickServerAPI.h"
#include "../AT/ActiveTickServerAPI/ActiveTickStreamListener.h"
#include "../AT/ActiveTickServerAPI/ActiveTickServerRequestor.h"


using namespace ATS_DS;


//	Trade Event States
//		Trades may stay in the buffer until processing is complete
enum TRADE_EVENT_STATE {
	NEW = 0,
	IN_PROGRESS,
	DONE
};

//	Trade Event
struct REALTIME_TRADE_EVENT {
	int	state;
	// Add provider structure here
};


//	Real-Time Trade Event Buffer container
//		Contains trade event, event state, provider trade data
struct REALTIME_TRADE_EVENT_BUFFER {
	//	Vector or events
	std::vector<REALTIME_TRADE_EVENT> trades;
};


struct ConnectionContextBlock
{
	uint32_t apiVersion;
	uint32_t serverPort;
	const wchar16_t* userid;
	const wchar16_t* password;
	const wchar16_t* trackStatsForSymbol;
	uint64_t hSession;
	uint64_t hLoginRequest;
	int	callbackCycleDelay;

	bool enabled;
	bool disableRDQInsertion;
	bool isUsingInternalQueue;

	string GUID_KEY;
	string primaryServerHostname;
	string secondaryServerHostname;
};


class DataConnector : public ActiveTickServerRequestor
{
public:
	DataConnector(void*);
	~DataConnector();
	const wchar_t*	moduleName = L"DataConnector";

	ATS_CODE initialize(void*);
	ATS_CODE initConnCtxBlock();
	ATS_CODE connectToService();
	ATS_CODE disconnectFromService();
	ATS_CODE loginToService();
	bool isConnectorEnabled();
	bool isbATProvConnected();
	bool setATProvConnected(bool);
	bool isbATProvLoggedin();
	bool setATProvLoggedin(bool);
	static ATS_CODE getSymbolList(DS_Shared*, wstring&);

	DS_Globals* pGlobals;
	LoggerX* pLogger;
	AT_Streamer* pStreamer;

	///////////////BEGIN///////////////
	//  Connector Context Data
	///////////////////////////////////
	ConnectionContextBlock connCtx;
	////////////////END////////////////



	
	///////////////BEGIN///////////////
	//	AT Provider Callbacks
	///////////////////////////////////
	static void ATRequestTimeoutCallback(
		uint64_t
	);

	static void AT_SessionStatusChangeCallback(
		uint64_t,
		ATSessionStatusType
	);

	static void ATLoginResponseCallback(
		uint64_t, uint64_t, LPATLOGIN_RESPONSE
	);

	static void ATStreamUpdateCallback(
		LPATSTREAM_UPDATE
	);
	////////////////END////////////////


	///////////////BEGIN///////////////
	//	AT Provider Callback Messages
	///////////////////////////////////
	static wstring last_ATLoginResponseCallback_msg;
	static wstring last_AT_SessionStatusChangeCallback_msg;
	////////////////END////////////////


private:
	// Declare private methods and objects
	DataConnector();
	DataConnector(const DataConnector &);
	DataConnector& operator=(const DataConnector&);

	static bool bConnectorEnabled;
	static bool bConnected;
	static bool bLoggedin;
	
};

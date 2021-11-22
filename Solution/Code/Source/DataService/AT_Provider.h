///////////////////////////////////////////////////////////////////////////////
//	Contains all ActiveTick (AT) Provider communications and management routines
//	- Initialize the Provider
//	- Setup the Session and register Callback routine
//	- Register the Streaming Quote Callback routine
//	- Communicate commands to the Provider
//	- Manage state of Provider session/connection
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Includes/CommonX.h"
#include "../Includes/PropsX.h"
#include "../Includes/StructX.h"
#include "AT_Helper.h"
#include "../AT/Shared/ATServerAPIDefines.h"
#include "../AT/ActiveTickServerAPI/ActiveTickServerAPI.h"
#include "../AT/ActiveTickServerAPI/ActiveTickStreamListener.h"
#include "../AT/ActiveTickServerAPI/ActiveTickServerRequestor.h"





///////////////////////////////////////////////////////////////////////////////
//	This is a wrapper for AT Provider Streaming functions
///////////////////////////////////////////////////////////////////////////////
class AT_DataConnector_Routines : public ActiveTickServerRequestor
{
public:
	AT_DataConnector_Routines();
	AT_DataConnector_Routines(void*);
	~AT_DataConnector_Routines();

	uint64_t hSession;
	ATGUID at_guid;
	static bool bConnected;

	DS_Globals* pGlobals;
	LoggerX* pLogger;


private:
	AT_DataConnector_Routines(const AT_DataConnector_Routines &);
	AT_DataConnector_Routines& operator=(const AT_DataConnector_Routines&);

	ATS_CODE initialize(void*);

#ifdef _UNUSED_CODE_
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

	virtual void OnATStreamTradeUpdate(
		LPATQUOTESTREAM_TRADE_UPDATE
	);
#endif _UNUSED_CODE_
	// Connect to Service


};









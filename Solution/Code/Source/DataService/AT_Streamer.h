#pragma once
#include "../Utils/Includes/CommonX.h"
#include "../Utils/Includes/PropsX.h"
#include "../Includes/StructX.h"	//	<--------------ADDED
#include "CircularBuffer.h"
#include "ActiveTickServerAPI.h"
#include "ActiveTickStreamListener.h"
#include "ActiveTickServerRequestor.h"
#include "DataSecurityStructs.h"
#include "AT_Helper.h"
#include "DS_Shared.h" // FIX



class AT_Streamer : public ActiveTickServerRequestor, public ActiveTickStreamListener
{
public:
	AT_Streamer(void*, uint64_t&);
	~AT_Streamer();
	ATS_CODE initialize(void*);


	DS_Globals* pGlobals;
	void* pArgs;
	LoggerX* pLogger;
	DataLoggerX* pDataLogger;
	DS_Shared* pDS_Shared; // TODO: FIX?
	uint64_t& m_hSession;

	ATS_CODE unpackTradeUpdate(LPATQUOTESTREAM_TRADE_UPDATE, TRADE&);
	ATS_CODE convertTradeMsgToWString(TRADE&, wchar_t*, int);

	AutoCriticalSection streamer_sso;

private:
	AT_Streamer();
	AT_Streamer(const AT_Streamer &);
	AT_Streamer& operator=(const AT_Streamer&);


	virtual void OnATStreamTradeUpdate(LPATQUOTESTREAM_TRADE_UPDATE pUpdate);




};


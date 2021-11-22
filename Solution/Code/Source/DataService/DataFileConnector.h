#pragma once

///////////////////////////////////////////////////////////////////////////////
//	Data Service - Data File-Based Connector
//	- Support for provider services
//	- Implements Snapshot File-Based data provider
//	- The file-based provider is used for testing purposes
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../Utils/Includes/CommonX.h"
#include "../Utils/Includes/PropsX.h"
#include "../Utils/Includes/StructX.h"


#include "DS_Shared.h"



using namespace ATS_DS;


#define DFC_MASTER_TUID		100000 // uid (unique TRADE identifier) to simulate trade IDs


struct DataFileConnectionContextBlock
{
	//	Configuration data
	const wchar16_t* snapshot_name;
	int frequency;
	int tradeRecordsPerCycle;
	bool bDisableRDQInsertion;
	bool bEnabled;
	bool insert_timestamp;

	//	File processing data
	std::wfstream snapshot_fs;
	std::streampos curr_pos;
	bool bConnected;
	wstring snapshot_path;

	//	Snapshot msg management components
	int queueThreshold;
	int	queueFillLimit;
	int tradeRecordCount;
	std::queue<TRADE> tradeMsg;
	bool bSnapshotFileEmpty;

};


class DataFileConnector
{
public:
	DataFileConnector(void*);
	~DataFileConnector();
	const wchar_t*	moduleName = L"DataFileConnector";

	ATS_CODE initialize(void*);
	ATS_CODE initConnCtxBlock();
	ATS_CODE connectToService();
	bool isConnectorEnabled();
	bool isDataFileProvConnected();

	DS_Globals* pGlobals;
	LoggerX* pLogger;
	DS_Shared* pDS_Shared;
	

	///////////////BEGIN///////////////
	//  Connector Context Data
	///////////////////////////////////
	DataFileConnectionContextBlock dataFileConnCtx;
	////////////////END////////////////

	ATS_CODE processSnapshotFile(void*);
	ATS_CODE convertToTradeRec(wchar_t*, TRADE&);


private:
	// ++ Not Implemented
	DataFileConnector();
	DataFileConnector(const DataFileConnector &);
	DataFileConnector& operator=(const DataFileConnector&);
	// -- Not Implemented
	unsigned int master_tuid;

};

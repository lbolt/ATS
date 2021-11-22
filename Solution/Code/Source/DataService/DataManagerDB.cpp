#include "stdafx.h"
#include "DataManagerDB.h"


DataManagerDB::DataManagerDB(void* Args)
{
	//	Perform basic inits
	bConnected = false;
	bWatchInitialized = false;
	hStmt_loadWatchRecs = NULL;
	initialize(Args);
}


DataManagerDB::~DataManagerDB()
{

}


ATS_CODE DataManagerDB::initialize(void* Args)
{
	//	Set DS_Globals pointer
	pGlobals = (DS_Globals*)Args;
	pLogger = (LoggerX*)pGlobals->Args[ARG_UTIL];
	pDSShared = (DS_Shared*)pGlobals->Args[ARG_SDS];

	//////////////////////////////////////////////////////////////////////////////
	//	Perform all setup steps here
	//	- Add database connection detail here
	//////////////////////////////////////////////////////////////////////////////



	wchar_t _inConnStr[256];
	wsprintf(	_inConnStr, L"Driver=%s;server=%s;database=%s;uid=%s;pwd=%s;", 
				pGlobals->pProps->prop_dbDriver.c_str(), 
				pGlobals->pProps->prop_dbServer.c_str(),
				pGlobals->pProps->prop_dbName.c_str(), 
				pGlobals->pProps->prop_dbUser.c_str(), 
				pGlobals->pProps->prop_dbCred.c_str());
	inConnStr.assign(_inConnStr);

	return ATS_OK;
}


///////////////////////////////////////////////////////////////////////////////
//	Connect to the database
ATS_CODE DataManagerDB::connectDB()
{

	SQLRETURN	sqlRc;
	wchar_t		logMsg[512];
	bConnected = false;

	//	For testing purposes - get window handle for desktop
	/*
	HWND desktopHandle = GetDesktopWindow();
	if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
		//	Report error
		error = L"connectDB: Failed on GetDesktopWindow";
		goto connectDBFail;
	}
	*/

	//	Set attributes on environment handle -- SQL_ATTR_CONNECTION_POOLING
	/*
	sqlRc = SQLSetEnvAttr(SQL_NULL_HENV, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER*)SQL_CP_ONE_PER_DRIVER, 0);
	if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
		//	Set logMsg
		swprintf(logMsg, sizeof logMsg, L"Failed on SQLSetEnvAttr (SQL_ATTR_CONNECTION_POOLING) - with code (%d)", sqlRc);
		goto connectDBFail;
	}
	*/
	//	ENVIRONMENT
	//	Allocate environment handle
	sqlRc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
		//	Set logMsg
		swprintf (logMsg, sizeof logMsg, L"Failed on GetDesktopWindow - with code (%d)", sqlRc);
		goto connectDBFail;
	}
	//	Set attributes on environment handle
	sqlRc = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
		//	Set logMsg
		swprintf(logMsg, sizeof logMsg, L"Failed on SQLSetEnvAttr (SQL_ATTR_ODBC_VERSION) - with code (%d)", sqlRc);
		goto connectDBFail;
	}

	//	CONNECTION
	//	Allocate DB connection handle
	sqlRc = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
		//	Set logMsg
		swprintf(logMsg, sizeof logMsg, L"Failed on SQLAllocHandle - with code (%d)", sqlRc);
		goto connectDBFail;
	}
	//	Set attributes on connection handle
	sqlRc = SQLSetConnectAttr(hDBC, SQL_LOGIN_TIMEOUT, (SQLPOINTER)7, 0);
	if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
		//	Set logMsg
		swprintf(logMsg, sizeof logMsg, L"Failed on SQLSetConnectAttr (SQL_LOGIN_TIMEOUT) - with code (%d)", sqlRc);
		goto connectDBFail;
	}

	//	CONNECT
	//	Make the connection to the DB
	sqlRc = SQLDriverConnect(
		hDBC,
		NULL,	//	Or - desktopHandle
		(SQLWCHAR*)inConnStr.c_str(),
		SQL_NTS,
		NULL,
		0,
		NULL,
		SQL_DRIVER_COMPLETE);

	if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
		//	Set logMsg
		swprintf(logMsg, sizeof logMsg, L"Failed on SQLDriverConnect - with code (%d)", sqlRc);
		goto connectDBFail;
	}
	
	//	Success
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
		pLogger->log_trace(L"Database connection successful", L"DataManagerDB::connectDB");
	bConnected = true;
	return ATS_OK;

connectDBFail:
	//	Log error
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
		pLogger->log_error(logMsg, L"DataManagerDB::connectDB");
	return ATS_FALSE;

}

bool DataManagerDB::isConnected()
{
	return (bConnected);
}

bool DataManagerDB::isWatchInitialized()
{
	return (bWatchInitialized);
}

///////////////////////////////////////////////////////////////////////////////
//	Load eligible watch records from DB to WatchPool item list
//	- Param: (out) WatchPoolItemList*
//
//int DataManagerDB::loadWatchRecs(WatchPoolItemList* pWatchPool)
int DataManagerDB::loadWatchRecs()
{

	SQLRETURN	sqlRc;
	wchar_t		logMsg[512];
	wchar_t		sqlMsg[256];

	try {
		if (bConnected == false)
		{
			swprintf(logMsg, sizeof logMsg, L"Not connected to database");
			goto loadWatchRecsFail;
		}

		//	Allocate a new statement handle for the connected DB
		if (hStmt_loadWatchRecs != NULL)
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt_loadWatchRecs);
		sqlRc = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt_loadWatchRecs);
		if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
			//	Set logMsg
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			{
				buildLoadWatchRecsMsg(SQL_HANDLE_STMT, hStmt_loadWatchRecs, sqlMsg, sizeof sqlMsg);
				swprintf(logMsg, sizeof logMsg, L"Failed on SQLAllocHandle for loadRecs Stored proccedure - %s", &sqlMsg);
			}
			goto loadWatchRecsFail;
		}
		//	Prepare the statement
		SQLWCHAR * strCallSP = L"{? = CALL sp_get_position_recs ()}";
		wchar_t    * strProcName = L"sp_get_position_recs";
		SQLHDESC hIpd = NULL;
		sqlRc = SQLPrepare(hStmt_loadWatchRecs, (SQLWCHAR*)L"{call sp_get_position_recs}", SQL_NTS);
		if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
			//	Set logMsg
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			{
				buildLoadWatchRecsMsg(SQL_HANDLE_STMT, hStmt_loadWatchRecs, sqlMsg, sizeof sqlMsg);
				swprintf(logMsg, sizeof logMsg, L"Failed on SQLPrepare for loadRecs Stored proccedure - %s", &sqlMsg);
			}
			goto loadWatchRecsFail;
		}
#ifdef UNUSED_CODE
		sqlRc = SQLGetStmtAttr(hStmt_loadWatchRecs, SQL_ATTR_IMP_PARAM_DESC, &hIpd, 0, 0);
		if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
			//	Set logMsg
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			{
				swprintf(logMsg, sizeof logMsg, L"Failed on SQLGetStmtAttr (SQL_ATTR_IMP_PARAM_DESC) - with code (%d)", sqlRc);
				pLogger->log_error(logMsg, L"DataManagerDB::loadWatchRecs");
			}
			goto loadWatchRecsFail;
		}

		sqlRc = SQLSetDescField(hIpd, 1, SQL_DESC_NAME, L"@quote", SQL_NTS);
		if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
			//	Set logMsg
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			{
				swprintf(logMsg, sizeof logMsg, L"Failed on SQLSetDescField (@quote) - with code (%d)", sqlRc);
				pLogger->log_error(logMsg, L"DataManagerDB::loadWatchRecs");
			}
			goto loadWatchRecsFail;
		}
#endif UNUSED_CODE
		sqlRc = SQLExecute(hStmt_loadWatchRecs);
		if (sqlRc != SQL_SUCCESS && sqlRc != SQL_SUCCESS_WITH_INFO) {
			//	Set logMsg
			if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
			{
				buildLoadWatchRecsMsg(SQL_HANDLE_STMT, hStmt_loadWatchRecs, sqlMsg, sizeof sqlMsg);
				swprintf(logMsg, sizeof logMsg, L"Failed on SQLExecute for loadRecs Stored proccedure - %s", &sqlMsg);
			}
			goto loadWatchRecsFail;
		}

		/**/
		///////////////////////////////////////////////////////////////////////	
		//	Convert DB resultset to operational format and add to item list 
		//	data object
		SQLSMALLINT		columns;
		SQLSMALLINT		colPosition;
		SQLLEN			indicator;
		int				count = 0;
		wchar_t			buf[MAXATDESCFIELDSIZE];	//	Match at least largest char field
		int				intBuf;
		unsigned char	bitBuf;
		SQL_TIMESTAMP_STRUCT timeStamp;

		sqlRc = SQLNumResultCols(hStmt_loadWatchRecs, &columns);	//	Num columns returned
		while (SQLFetch(hStmt_loadWatchRecs) != SQL_NO_DATA) {		//	Next row
			count++;	//	Keep track of rows returned
			WatchPoolItem* pWPRec = new WatchPoolItem();	//	Create WatchPool item
			//	Iterate through the columns in this record and assign a value to each of the data object fields
			for (colPosition = 1; colPosition <= columns; colPosition++) {
				//	Retrieves column data based on select statement ordering
				switch (colPosition)
				{
				case 1:	//	Sequence
					intBuf = 0;
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_LONG, &intBuf, sizeof intBuf, &indicator);
					pWPRec->sequence = intBuf;
					break;
				case 2:	//	Symbol
					memset(buf, L' ', sizeof buf);
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_WCHAR, (wchar_t*)&buf, sizeof buf, &indicator);
					pWPRec->symbol.assign(buf, indicator / 2);
					while (!pWPRec->symbol.empty() && iswspace(pWPRec->symbol.back())) pWPRec->symbol.pop_back();
					break;
				case 3:	//	Average volume
					intBuf = 0;
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_LONG, &intBuf, sizeof intBuf, &indicator);
					pWPRec->avgVolume = intBuf;
					break;
				case 4:	//	Watch eligible
					bitBuf = 0;
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_BIT, &bitBuf, sizeof bitBuf, &indicator);
					pWPRec->watchEligible = bitBuf;
					break;
				case 5:	//	Trade eligible
					bitBuf = 0;
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_BIT, &bitBuf, sizeof bitBuf, &indicator);
					pWPRec->tradeEligible = bitBuf;
					break;
				case 6:	//	Create Date/Time
					memset(&timeStamp, 0, sizeof timeStamp);
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_TYPE_TIMESTAMP, &timeStamp, sizeof timeStamp, &indicator);
					break;
				case 7:	//	Update Date/Time
					memset(&timeStamp, 0, sizeof timeStamp);
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_TYPE_TIMESTAMP, &timeStamp, sizeof timeStamp, &indicator);
					break;
				case 8:	//	Exchange
					memset(buf, L' ', sizeof buf);
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_WCHAR, (wchar_t*)&buf, sizeof buf, &indicator);
					pWPRec->exchange.assign(buf, indicator / 2);
					while (!pWPRec->exchange.empty() && iswspace(pWPRec->exchange.back())) pWPRec->exchange.pop_back();
					break;
				case 9:	//	Description
					memset(buf, L' ', sizeof buf);
					sqlRc = SQLGetData(hStmt_loadWatchRecs, colPosition, SQL_C_WCHAR, (wchar_t*)&buf, sizeof buf, &indicator);
					pWPRec->description.assign(buf, indicator / 2);
					while (!pWPRec->description.empty() && iswspace(pWPRec->description.back())) pWPRec->description.pop_back();
					break;
				default:
					break;
				}
			}
			if (pWPRec->watchEligible == true)
			{
				//	Place the data object in the Watch Pool Item map
				pDSShared->pWatchPool->watchPool.insert(std::pair<wstring, WatchPoolItem*>(pWPRec->symbol, pWPRec));
				//	Add sequence number for item to Predecessor Busy Hash
				pDSShared->pPredBusyHash->add(pWPRec->sequence);
				//	Add item to lookup table
				pDSShared->pSequenceLookupMap->symToseqNumLookupHash.insert(std::pair<wstring, unsigned int>(pWPRec->symbol, pWPRec->sequence));
				//	Add sequence number for item to Active Watch Table
				pDSShared->pActiveCTXBlock->createActiveCTXRecord(pWPRec);
				//	Add item to Active Analytic Context Container
				pDSShared->pActiveAnalyticCtxBlock->createContainerRecord(pWPRec);
				//	Add sequence number for item to Next Ready Analytic Context Busy Hash
				pDSShared->pNRQBusyHash->add(pWPRec->sequence);
			}
		}
		// Report counts
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_INFO))
		{
			swprintf(logMsg, sizeof logMsg, L"Total number of watch records read from DB (%d) - Total Actively watched (%d)", count, (int)pDSShared->pActiveCTXBlock->active_CTX_Container.size());
			pLogger->log_info(logMsg, L"DataManagerDB::loadWatchRecs");
		}

	}
	catch (...)
	{
		//
		if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_WARN))
			pLogger->log_warn(L"Unexpected exception caught", L"DataManagerDB::loadWatchRecs");
	}

	//	Success - object created/updated 
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_TRACE))
		pLogger->log_trace(L"Watch record load complete", L"DataManagerDB::loadWatchRecs");
	return S_OK;

loadWatchRecsFail:
	//	Log error
	if (pLogger->assertLogLevel(DS_LOG_LEVEL::DSL_ERROR))
		pLogger->log_error(logMsg, L"DataManagerDB::loadWatchRecs");
	return S_FALSE;
}


void DataManagerDB::buildLoadWatchRecsMsg(SQLSMALLINT hdlType, SQLHSTMT hStmt, wchar_t* sqlMsg, size_t size)
{

	SQLSMALLINT handletype = hdlType;
	SQLHANDLE handle = hStmt;
	SQLWCHAR state[6];
	SQLINTEGER errnr;
	SQLWCHAR msg[256];
	SQLSMALLINT msglen;
	SQLGetDiagRecW(handletype, handle, 1, state, &errnr, msg, sizeof(msg), &msglen);
	swprintf(sqlMsg, size, L"state: [%6s] message [%s]", state, msg);
}

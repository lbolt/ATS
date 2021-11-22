/////////////////////////////////////////////////////////////////////////////////////
//	DataManagerDB
//	- Manages ODBC calls to MSSQL database system
//	- The lifecycle of this object is owned / controlled by the DataManager service
//
//
#pragma once
#include "../Utils/Includes/CommonX.h"
#include "../Utils/Includes/PropsX.h"
#include "../Utils/Includes/RdbX.h"
#include "DS_Shared.h"


//	Forward decls
class DS_Globals;



class DataManagerDB
{
public:
	DataManagerDB(void*);
	~DataManagerDB();
	const wchar_t*	moduleName = L"DataManagerDB";

	ATS_CODE initialize(void*);
	ATS_CODE connectDB();
	ATS_CODE disconnectDB();
	//int loadWatchRecs(WatchPoolItemList*); // need this?
	int loadWatchRecs();
	bool isConnected();
	bool isWatchInitialized();

	//	Environment data from configuration file
	DS_Globals*	pGlobals;
	LoggerX*	pLogger;
	DS_Shared*	pDSShared;
	wstring		dbDSN;		//	future
	wstring		dbDriver;
	wstring		dbServer;
	wstring		dbName;
	wstring		dbUser;
	wstring		dbCred;

	bool bConnected;		// State currently controlled in the DataManager object
	bool bWatchInitialized;	// Start-up has run & initial DB read has been completed

	//	Connection string detail (uses DSN)
	SQLHENV		hEnv;
	SQLHDBC		hDBC = NULL;
	//SQLWCHAR*	inConnStr;
	wstring inConnStr;

	//	Prep for static Statements
	SQLHSTMT	hStmt_loadWatchRecs;


	//	SPI Calls



private:
	// Declare private methods and objects
	DataManagerDB();
	DataManagerDB(const DataManagerDB &);
	DataManagerDB& operator=(const DataManagerDB&);

	void buildLoadWatchRecsMsg(SQLSMALLINT, SQLHSTMT, wchar_t*, size_t);

};


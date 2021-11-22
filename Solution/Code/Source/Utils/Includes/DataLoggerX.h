#pragma once

#include "../Includes/CommonX.h"
#include "../Includes/PropsX.h"
#include "../Includes/SafeSyncX.h"


//	Forward decls
class DS_Globals;


enum DS_DATALOGGER_LEVEL {
	SNAP_OUTPUT=0,
	SNAP_VERBOSE
};

#define	DATALOGGER_MSG_SMALLBUF		128
#define	DATALOGGER_MSG_MEDIUMBUF	256
#define	DATALOGGER_MSG_LARGEBUF		1024
#define	DATALOGGER_MSG_XLARGEBUF	2048



///////////////////////////////////////////////////////////////////////////////
//	ATS Data Message File-system LAST TRADE Logger
//	- Data:
//		Last Trade Data
//
///////////////////////////////////////////////////////////////////////////////
struct DataLoggerMessage
{
public:
	wstring msg;
	wstring level;
	wstring ident;
	wstring timestamp;
};

class DataLoggerX
{
public:
	DataLoggerX(void*);
	~DataLoggerX();
	const wchar_t*	moduleName = L"DataLoggerX";

	bool isInitialized();
	bool isEnabled(void);
	int safeMsgCount(void);
	int getLoggerLevel(void);
	void output(const wchar_t*);
	void verbose(const wchar_t*);
	int flushQueue(int);

	static AutoCriticalSection dataLogger_cs; 

	//	Reference pointers to global objects
	DS_Globals* pGlobals;
	
private:
	// Declare private methods and objects
	DataLoggerX();
	DataLoggerX(const DataLoggerX &);
	DataLoggerX& operator=(const DataLoggerX&);
	//
	int initialize(void*);
	bool isRolloverEnabled();
	int safeInsertMsg(const wchar_t*, int);
	int safeRemoveMsg();


	bool initState = false;
	bool loggerEnabled = false;
	bool rolloverEnabled = true;
	int dataLoggerLevel = DS_DATALOGGER_LEVEL::SNAP_OUTPUT;

	//	Callers write messages to queue
	queue<DataLoggerMessage*> dataLogMsgQ;
	wstring snapShotFile;

};



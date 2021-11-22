#pragma once

#include <sapi.h>

#include "../Includes/CommonX.h"
#include "../Includes/PropsX.h"
#include "../Includes/SafeSyncX.h"

//	Forward decls

//	Forward decls
class DS_Globals;



enum DS_LOGGER_LEVEL 
{
	DS_ERROR=0,
	DS_WARN,
	DS_INFO,
	DS_TRACE,
	DS_DEBUG,
	DS_DEBUG_2,
	DS_FUNC_ENTRY,
	DS_TRADE,
	DS_SPEAK,
	DS_STATS
};

enum DS_LOG_LEVEL 
{
	DSL_ERROR		= 0x00000001,
	DSL_WARN		= 0x00000002,
	DSL_INFO		= 0x00000004,
	DSL_TRACE		= 0x00000008,
	DSL_DEBUG		= 0x00000010,
	DSL_DEBUG_2		= 0x00000020,
	DSL_FUNC_ENTRY	= 0x00000040,
	DSL_TRADE		= 0x00000080,
	DSL_SPEAK		= 0x00000100,
	DSL_STATS		= 0x00000200
};



#define	LOGGER_MSG_SMALLBUF		128
#define	LOGGER_MSG_MEDIUMBUF	256
#define	LOGGER_MSG_LARGEBUF		1024
#define	LOGGER_MSG_XLARGEBUF	2048



//	Structured log message
//	- timestamp caller:  message
//	- where:
//		timestamp: yyyy-mm-dd mmmm
//		caller: modulename
struct LoggerMessage
{
public:
	wstring msg;
	wstring level;
	wstring ident;
	wstring timestamp;
};


//	Logger Object
//	- Implements a message queue object
//	- All access to the queue object is synchronized via critical section
//	- Queue data is written to the logfile via management thread
//
class LoggerX
{
public:
	LoggerX(void*);
	~LoggerX();
	const wchar_t*	moduleName = L"LoggerX";

	bool isInitialized();
	bool isEnabled();
	int safeMsgCount();
	int setState(int);
	bool assertLogLevel(UINT);

	void log_error(const wchar_t*, const wchar_t*);
	void log_warn(const wchar_t*, const wchar_t*);
	void log_info(const wchar_t*, const wchar_t*);
	void log_trace(const wchar_t*, const wchar_t*);
	void log_debug(const wchar_t*, const wchar_t*);
	void log_debug_2(const wchar_t*, const wchar_t*);
	void log_func_entry(const wchar_t*, const wchar_t*);
	void log_trade(const wchar_t*, const wchar_t*);
	void log_speak(const wchar_t*, const wchar_t*);
	void log_stats(const wchar_t*, const wchar_t*);
	int flushQueue(int);

	static AutoCriticalSection logger_cs;

	//	Reference pointers to global objects
	DS_Globals* pGlobals;


private:
	// Declare private methods and objects
	LoggerX();
	LoggerX(const LoggerX &);
	LoggerX& operator=(const LoggerX&);
	//
	ATS_CODE initialize(void*);
	int safeInsertMsg(const wchar_t*, const wchar_t*, int);
	int safeRemoveMsg();
	bool isRolloverEnabled();

	//
	bool initState = false;;
	bool loggerEnabled=true;
	bool rolloverEnabled = true;
	UINT log_level = DS_LOG_LEVEL::DSL_ERROR;
	map<unsigned int, wstring> log_level_desc;

	wstring getDateStamp(void);	//	Timestamp is added to message when queued

	//	Callers write messages to queue
	queue<LoggerMessage*> logMsgQ;
	wstring logFile;
	bool file_log_desc(void);


};




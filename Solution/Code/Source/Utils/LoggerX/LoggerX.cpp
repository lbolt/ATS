#include "stdafx.h"
#include "../Includes/LoggerX.h"
#include "../Includes/ThreadMgrX.h"
#include "time.h"


//	Statics
AutoCriticalSection LoggerX::logger_cs;





LoggerX::LoggerX(void* Args)
{
	int rc = initialize(Args);
}


LoggerX::~LoggerX()
{

}

int LoggerX::initialize(void* Args)
{
	AutoCriticalSectionLock LoggerX_sso(logger_cs);

	if (isInitialized() == true)
		return S_OK;

	pGlobals = (DS_Globals*)Args;
	logFile = pGlobals->logFilePath;

	///////////////////////////////////////////
	//	Perform all setup steps here
	///////////////////////////////////////////
	log_level = pGlobals->pProps->prop_log_level;
	file_log_desc();
	(_wcsicmp(pGlobals->pProps->prop_logging_enabled.c_str(), L"true") == 0) ? loggerEnabled = true: loggerEnabled = false;
	(_wcsicmp(pGlobals->pProps->prop_logging_rollover.c_str(), L"true") == 0) ? loggerEnabled = true: rolloverEnabled = false;
	initState = true;



	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////
//	Call Methods
////////////////////////////////////////////////////////////////////////////////////////////

void LoggerX::log_error(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_ERROR)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_ERROR);
		}
	}
}


void LoggerX::log_warn(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_WARN)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_WARN);
		}
	}
}


void LoggerX::log_info(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_INFO)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_INFO);
		}
	}
}


void LoggerX::log_trace(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_TRACE)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_TRACE);
		}
	}
}


void LoggerX::log_debug(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_DEBUG)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_DEBUG);
		}
	}
}


void LoggerX::log_debug_2(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_DEBUG_2)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_DEBUG_2);
		}
	}
}


void LoggerX::log_func_entry(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_FUNC_ENTRY)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_FUNC_ENTRY);
		}
	}
}


void LoggerX::log_trade(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_TRADE)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_TRADE);
		}
	}
}

void LoggerX::log_speak(const wchar_t* msg, const wchar_t* ident)
{
	AutoCriticalSectionLock LoggerX_sso(logger_cs);

	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_SPEAK)
		{
			//	Speak message
			//	Initialize COM for SAPI
			ISpVoice * pVoice;
			pVoice = NULL;
			if (FAILED(::CoInitialize(NULL)))
			{
				return;
			}
			HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
			if (FAILED(hr))
			{
				pVoice->Release();
				pVoice = NULL;
			}
			else
			{
				//SpCreateBestObject(SPCAT_VOICES, L”Gender = Female”, NULL, &pVoice);
				hr = pVoice->Speak(msg, 0, NULL);
				if (FAILED(hr))
				{
					hr = hr;
				}
				if (pVoice->Release() != NULL)
				{
					pVoice->Release();
				}
			}
			//::CoUninitialize();
		}
	}
}

void LoggerX::log_stats(const wchar_t* msg, const wchar_t* ident)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (log_level & DS_LOG_LEVEL::DSL_STATS)
		{
			safeInsertMsg(msg, ident, DS_LOG_LEVEL::DSL_STATS);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

int LoggerX::safeInsertMsg(const wchar_t* msg, const wchar_t* caller, int level)
{
	// Acquire safesync queue insertion object
	AutoCriticalSectionLock LoggerX_sso(logger_cs);

	LoggerMessage* logMsg = new LoggerMessage();
	logMsg->timestamp = getDateStamp();
	logMsg->msg = msg;
	logMsg->level = log_level_desc[level];
	logMsg->ident.assign(caller);
	//	Let the management thread build the msg and write it to the file - less overhead
	logMsgQ.push(logMsg);
	return S_OK;
}


int LoggerX::safeRemoveMsg()
{
	// Acquire safesync queue extraction object
	AutoCriticalSectionLock LoggerX_sso(logger_cs);


	return S_OK;
}


int LoggerX::safeMsgCount()
{
	//	Create safesync object
	AutoCriticalSectionLock LoggerX_sso(logger_cs);

	//	Get the msg count via safe sync
	return (int) logMsgQ.size();
}


bool LoggerX::isInitialized()
{
	//	Return initialization state
	return (initState);
}


bool LoggerX::isEnabled()
{
	//	Return initialization state
	return (loggerEnabled);
}

bool LoggerX::isRolloverEnabled()
{
	//	Return logfile rollover state
	return (rolloverEnabled);
}

int LoggerX::setState(int newState)
{
	//	Return initialization state
	return S_OK;
}

bool LoggerX::assertLogLevel(UINT assert_level)
{
	return (assert_level & log_level);
}

wstring LoggerX::getDateStamp(void)
{
	//	create a yyyymmddhhmm date stamp
	char timeStamp[26];
	struct tm localTM;
	time_t long_time;
	wstring dateTime;

	memset(timeStamp, 0, sizeof timeStamp);
	time(&long_time);					//	Get time as long integer
	localtime_s(&localTM, &long_time);	//	Convert to local time
	sprintf_s(timeStamp, sizeof timeStamp, "%4d-%02d-%02d %02d%02d.%02d"
		, localTM.tm_year + 1900
		, localTM.tm_mon + 1
		, localTM.tm_mday
		, localTM.tm_hour
		, localTM.tm_min
		, localTM.tm_sec
	);
	wstring ws(&timeStamp[0], &timeStamp[strlen(timeStamp)]);	//	convert to wide
	return (ws);
}


int LoggerX::flushQueue(int msgCount)
{
	//	TODO: future
	//	- (Roll the logfile separately - leverage the timestamp method for date-time of roll)
	//	- Need to figure-out when to perform the roll (should be at 00:00 hours)

	//	Iterate through the msg queue and write the log messages to the file
	LoggerMessage* msg;
	wchar_t msgBuf[LOGGER_MSG_MEDIUMBUF];
	wfstream os;

	try
	{
		os.open(logFile.data(), std::ios_base::app);
		if (!os.fail())
		{
			for (int count = msgCount; count > 0; count--)
			{
				{
					AutoCriticalSectionLock LoggerX_sso(logger_cs);
					if (logMsgQ.empty())
					{
						break;
					}

					msg = NULL;
					memset(msgBuf, 0, sizeof msgBuf);
					msg = logMsgQ.front();
					if (msg != NULL)
					{
						logMsgQ.pop();
						wsprintf(msgBuf, L"%s (%s) %s: %s", msg->timestamp.data(), msg->level.data(), msg->ident.data(), msg->msg.data());
						delete msg;
					}
					os << msgBuf << L"\n";
				}
			}

			//	Flush & close
			os.flush();
			os.close();
		}
	}
	catch (...)
	{

	}

	return S_OK;
}

bool LoggerX::file_log_desc()
{
	log_level_desc[DS_LOG_LEVEL::DSL_ERROR] = L"Error";
	log_level_desc[DS_LOG_LEVEL::DSL_WARN] = L"Warn";
	log_level_desc[DS_LOG_LEVEL::DSL_INFO] = L"Info";
	log_level_desc[DS_LOG_LEVEL::DSL_TRACE] = L"Trace";
	log_level_desc[DS_LOG_LEVEL::DSL_DEBUG] = L"Debug";
	log_level_desc[DS_LOG_LEVEL::DSL_DEBUG_2] = L"Debug_2";
	log_level_desc[DS_LOG_LEVEL::DSL_FUNC_ENTRY] = L"Func_Entry";
	log_level_desc[DS_LOG_LEVEL::DSL_TRADE] = L"Trade";
	log_level_desc[DS_LOG_LEVEL::DSL_SPEAK] = L"Speak";
	log_level_desc[DS_LOG_LEVEL::DSL_STATS] = L"Stats";
	return true;
}



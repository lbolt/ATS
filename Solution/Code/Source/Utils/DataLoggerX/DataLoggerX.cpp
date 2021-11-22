// DataLoggerX.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"
#include "../Includes/DataLoggerX.h"
#include "../Includes/ThreadMgrX.h"
#include "time.h"



//	Statics
AutoCriticalSection DataLoggerX::dataLogger_cs;


wchar_t* DATALOGGER_LEVEL_DESC[] = { (wchar_t*)L"output", (wchar_t*)L"verbose", (wchar_t*)L"null" };





///////////////////////////////////////////////////////////////////////////////
//	ATS Data Message File-system Logger
//	
//	- This data logger serializes last trade received by Real-TIme Data Provider
//	  to a snapshot file.
//
//	- See configuration file for set-up detail.
//
///////////////////////////////////////////////////////////////////////////////

DataLoggerX::DataLoggerX(void* Args)
{
	ATS_CODE rc = initialize(Args);
}

DataLoggerX::~DataLoggerX()
{
}

ATS_CODE DataLoggerX::initialize(void* Args)
{
	AutoCriticalSectionLock DataLogger_sso(dataLogger_cs);

	if (isInitialized() == true)
		return ATS_C_SUCCESS;

	pGlobals = (DS_Globals*)Args;
	snapShotFile = pGlobals->snapshotFilePath;

	///////////////////////////////////////////
	//	Perform all setup steps here
	///////////////////////////////////////////
	//TODO: FIX These
	dataLoggerLevel = _wtoi(pGlobals->pProps->prop_snapshot_level.c_str());
	(_wcsicmp(pGlobals->pProps->prop_snapshot_enabled.c_str(), L"true") == 0) ? loggerEnabled = true : loggerEnabled = false;
	(_wcsicmp(pGlobals->pProps->prop_snapshot_rollover.c_str(), L"true") == 0) ? rolloverEnabled = true : rolloverEnabled = false;
	initState = true;

	return ATS_C_SUCCESS;
}


bool DataLoggerX::isInitialized()
{
	//	Return initialization state
	return (initState);
}


bool DataLoggerX::isEnabled(void)
{
	//	Return initialization state
	return (loggerEnabled);
}

bool DataLoggerX::isRolloverEnabled()
{
	//	Return logfile rollover state
	return (rolloverEnabled);
}


void DataLoggerX::output(const wchar_t* msg)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (dataLoggerLevel >= DS_DATALOGGER_LEVEL::SNAP_OUTPUT)
		{
			safeInsertMsg(msg, DS_DATALOGGER_LEVEL::SNAP_OUTPUT);
		}
	}
}

void DataLoggerX::verbose(const wchar_t* msg)
{
	if (isEnabled() && isInitialized())
	{
		// check logger level
		if (dataLoggerLevel >= DS_DATALOGGER_LEVEL::SNAP_VERBOSE)
		{
			// Add code to modify message
			safeInsertMsg(msg, DS_DATALOGGER_LEVEL::SNAP_VERBOSE);
		}
	}
}


/////////////////////////////////
///////////////////////////////////

int DataLoggerX::safeInsertMsg(const wchar_t* msg, int level)
{
	AutoCriticalSectionLock DataLogger_sso(dataLogger_cs);

	DataLoggerMessage* dataLogMsg = NULL;

	// Acquire safesync queue insertion object
	//dataLogger_cs dataLogger_sso.lock();
	dataLogMsg = new DataLoggerMessage();
//	logMsg->timestamp = getDateStamp();
	dataLogMsg->msg = msg;
	dataLogMsg->level = DATALOGGER_LEVEL_DESC[level];
	dataLogMsgQ.push(dataLogMsg);

	//dataLogger_sso.release();
	return S_OK;
}


int DataLoggerX::safeRemoveMsg()
{
	AutoCriticalSectionLock DataLogger_sso(dataLogger_cs);

	return S_OK;
}


int DataLoggerX::safeMsgCount()
{
	AutoCriticalSectionLock DataLogger_sso(dataLogger_cs);

	return (int)dataLogMsgQ.size();
}

int DataLoggerX::getLoggerLevel(void)
{
	return dataLoggerLevel;
}
//////////////////////////////////
/////////////////////////////////

int DataLoggerX::flushQueue(int msgCount)
{
	//	TODO: future
	//	- (Roll the logfile separately - leverage the timestamp method for date-time of roll)
	//	- Need to figure-out when to perform the roll (should be at 00:00 hours)

	//	Iterate through the msg queue and write the log messages to the file
	DataLoggerMessage* dataLogMsg;
	wchar_t msgBuf[DATALOGGER_MSG_MEDIUMBUF]; 
	wfstream os;

	try 
	{
		os.open(snapShotFile.data(), ios::out | ios::app);
		if (!os.fail())
		{
			for (int count = msgCount; count > 0; count--)
			{
				{
					AutoCriticalSectionLock DataLogger_sso(dataLogger_cs);
					if (dataLogMsgQ.empty())
					{
						break;
					}
					dataLogMsg = NULL;
					memset(msgBuf, 0, sizeof msgBuf);
					dataLogMsg = dataLogMsgQ.front();
					if (dataLogMsg != NULL)
					{
						dataLogMsgQ.pop();
						wsprintf(msgBuf, L"%s", dataLogMsg->msg.data());
						delete dataLogMsg;
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




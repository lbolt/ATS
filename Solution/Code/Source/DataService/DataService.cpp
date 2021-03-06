///////////////////////////////////////////////////////////////////////////////
//	DataService.cpp : Defines the entry point for the console application
//
//
//	This will eventually be re-written as a service
//
#include "stdafx.h"
#include "DataService.h"


int CONTROL_SIG_RAISED = 0;


//	Forward decls
BOOL WINAPI CtrlHandler(DWORD);



///////////////////////////////////////////////////////////////////////////////
//	service Main
///////////////////////////////////////////////////////////////////////////////
int _tmain(int argc, wchar_t* argv[])
{
	DataManager* pDataMgr=NULL;
	int rcDM = 0;

	try {
		//	Set control handlers
		SetConsoleCtrlHandler(CtrlHandler, TRUE);
		//	Create an initialize the DataManager object  
		pDataMgr = new DataManager(CONTROL_SIG_RAISED);
		//	Start the collection and analytic subsystem threads
		pDataMgr->initialize(argc, argv);
		//	Service run loop - manages the overall system status and operational subsystems
		rcDM = pDataMgr->run();
		//	Perform an orderly shutdown and cleanup
		pDataMgr->shutdown();
	}
	catch (...) {
	}
	//	Release data manager object

	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//	Control Handler
//
//	Control^C and Control^Break will set the system DS_SYSTEM_ST
//	to SHUTDOWN
///////////////////////////////////////////////////////////////////////////////
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		//	Stop Signal Raised
		CONTROL_SIG_RAISED = 1;
		return TRUE;

		// CTRL-CLOSE: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		//	NOOP
		return TRUE;

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:
		//	Stop Signal Raised
		CONTROL_SIG_RAISED = 1;
		return TRUE;

	case CTRL_LOGOFF_EVENT:
		//	NOOP
		return FALSE;

	case CTRL_SHUTDOWN_EVENT:
		//	NOOP
		return FALSE;

	default:
		return FALSE;
	}
}


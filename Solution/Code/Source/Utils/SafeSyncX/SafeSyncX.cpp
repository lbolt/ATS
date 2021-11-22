#include "stdafx.h"
#include "../Includes/SafeSyncX.h"



///////////////////////////////////////
//	class CS_Sync_External 
////////////////////////////////////////
CS_Sync_External::CS_Sync_External()
{
	initialize();
}

CS_Sync_External::~CS_Sync_External()
{
	DeleteCriticalSection(_sync_obj);
	if (_sync_obj != NULL)
		delete _sync_obj;
}

void CS_Sync_External::initialize()
{
	_sync_obj = new CRITICAL_SECTION;
	InitializeCriticalSectionAndSpinCount(_sync_obj, 0x00001770);
	//InitializeCriticalSection(_sync_obj);
	initState = true;
}

void CS_Sync_External::lock()
{
	EnterCriticalSection(_sync_obj);
}

void CS_Sync_External::release()
{
	LeaveCriticalSection(_sync_obj);
}




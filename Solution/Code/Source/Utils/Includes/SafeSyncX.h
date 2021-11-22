#pragma once

//#include "shlobj.h"
//#include "concrt.h"
#include "Windows.h"



//
// useful class to ensure that resources would be deleted on exception
//
class AutoCriticalSection
{
public:
	AutoCriticalSection() 
	{ 
		InitializeCriticalSectionAndSpinCount(&m_cs, 0x00001770);
		//InitializeCriticalSection(&m_cs);
	}
	~AutoCriticalSection() { Leave(); DeleteCriticalSection(&m_cs); }

	void Enter() { EnterCriticalSection(&m_cs); }
	void Leave() { LeaveCriticalSection(&m_cs); }

private:
	// ++ Not implemented
	AutoCriticalSection& operator=(const AutoCriticalSection&);
	// -- Not implemented

protected:
	CRITICAL_SECTION m_cs;

};

//
// useful class to ensure that resources would be deleted on exception
//
class AutoCriticalSectionLock
{
public:
	AutoCriticalSectionLock(AutoCriticalSection& cs)
		: m_cs(cs)
	{
		m_cs.Enter();
	}

	~AutoCriticalSectionLock()
	{
		m_cs.Leave();
	}

private:
	// ++ Not implemented
	AutoCriticalSectionLock& operator=(const AutoCriticalSectionLock&);
	// -- Not implemented

protected:
	AutoCriticalSection&	m_cs;
};



/////////////////////////////////////////////////////////////////////
//	Implements MANUAL Critical Section locking management
//	- Each instance of this object is unique and persistent
/////////////////////////////////////////////////////////////////////
class CS_Sync_External
{
public:
	CS_Sync_External();
	~CS_Sync_External();

	void initialize();
	void lock();
	void release();
	
private:
	// ++ Not implemented
	CS_Sync_External(CS_Sync_External&);
	CS_Sync_External& operator=(CS_Sync_External&);
	// -- Not implemented

	CRITICAL_SECTION* _sync_obj;
	bool	initState = false;
	bool isInitialized() { return initState; }
};






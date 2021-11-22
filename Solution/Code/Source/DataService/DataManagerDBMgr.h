#pragma once
//
#include "../Utils/Includes/CommonX.h"
#include "../Utils/Includes/PropsX.h"

class DataManagerDBMgr
{
public:
	DataManagerDBMgr();
	~DataManagerDBMgr();
	const wchar_t*	moduleName = L"DataManagerDBMgr";

private:
	// Declare private methods and objects
	DataManagerDBMgr(const DataManagerDBMgr &);
	DataManagerDBMgr& operator=(const DataManagerDBMgr&);
};



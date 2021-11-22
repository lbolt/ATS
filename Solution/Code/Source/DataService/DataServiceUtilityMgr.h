#pragma once
//
#include "CommonX.h"
#include "PropsX.h"



class DataServiceUtilityMgr
{
public:
	DataServiceUtilityMgr();
	~DataServiceUtilityMgr();
	const wchar_t*	moduleName = L"DataServiceUtilityMgr";


private:
	// Declare private methods and objects
	DataServiceUtilityMgr(const DataServiceUtilityMgr &);
	DataServiceUtilityMgr& operator=(const DataServiceUtilityMgr&);
};

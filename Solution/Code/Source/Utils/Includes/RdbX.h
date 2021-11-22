///////////////////////////////////////////////////////////////////////////////
//	Contains all RDB related Classes
//	- All calls are made through stored procedures
//	- SPs may return one or more rows in their result sets; some return a single row
//	- All stored SPs are mapped to C++ objects
//
//	The DataRecords file contains all Objects (classes) that map to a stored procedure
//	for either input or output. All SQL is executed via transact SQL in the database.
//
//	Recordsets for input or output will be placed in a std::list. The list acts as a
//	collection object of type "specialized record".
//
//	Stored procedures that return a result set (reads from DB) of one or more records 
//	of type "specialized record" will be placed in an std::list object
//	
//	Stored procedures that that are called iteratively for input (writes to the database) 
//	containing one or more records will be acquired from the std::list of 
//	"specialized record" objects.
//
//	The following is a list of all "specialized record" types and their containers:
//		
//		WatchPoolRecord - Record identifying a security of interest
//	
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Includes/CommonX.h"
#include "../Includes/PropsX.h"
#include "../Includes/StructX.h"
#include <windows.h>  
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sqlucode.h>


///////////////////////////////////////////////////////////////////////////////
//	WorkPoolRec
//	- Record identifying security eligible for position calculation and trading
//	- These records are acquired from the DB via "sp_get_position_rec.sql"
///////////////////////////////////////////////////////////////////////////////
class WatchPoolRec
{
public:
	WatchPoolRec();
	~WatchPoolRec();

	//	TODO: 
	//	- build-out the record based on WorkPool data record
	//	- ...



private:
	// Declare private methods and objects
	WatchPoolRec(const WatchPoolRec &);
	WatchPoolRec& operator=(const WatchPoolRec&);
};


class WatchPoolRecList
{
public:
	WatchPoolRecList();
	~WatchPoolRecList();


	std::list<WatchPoolRec*> watchList;


private:
	// Declare private methods and objects
	WatchPoolRecList(const WatchPoolRecList &);
	WatchPoolRecList& operator=(const WatchPoolRecList&);
};



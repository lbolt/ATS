#pragma once

/////////////////////////////////////////////////////////////////////////////////////
//	Contains all data structures used by DataManager
//
//	The following is a list of all "specialized record" types and their containers:
//		
//		SOIRecord - Security of interest record
//	
//	

#include "../Includes/CommonX.h"
#include "../Includes/PropsX.h"
#include "../Includes/SafeSyncX.h"
#include "../Includes/RdbX.h"
#include "../Includes/CircularBuffer.h"
#include "../../DataService/DataSecurityStructs.h"
#include "../Includes/AnalyticX.h"

#include <stack>







namespace ATS_STRUCT_DATA_DEFAULTS
{
	////////////////////////////////////
	//	Default Values
	////////////////////////////////////
	//	StructX
	static int			SYMBOL_MAXSIZE					= 8;		//	Set to 8
	static bool			DATA_COMPRESSION_ENABLED		= true;		//
	static bool			DATA_GAP_ENABLED				= true;		//
	static int			OUTLIER_COLLECTION_SIZE			= 3;		//	Default number of ticks required for collection to be considered an out-lier
	static double		MAX_OUTLIER_DELTA				= 1.0;		//	Percentage above last trade value to identify value as a potential outlier (must be larger than BASIS_POINT_ADJ)
	static int			SUPERTICK_SIZE					= 5;		//	Default number of ticks in a SuperTick
	static int			MIN_SLOTS_FOR_TREND_CALCS		= 50;		//	Minimum slots required to perform trend calculations
	static int			NUM_SLOTS_TO_ESTABLISH_TREND	= 200;		//	Number of slots required in window during ESTABLISH phase to identify a trade worthy trend
	static int			MAX_SLOTS_ON_FOLLOW				= 200;		//	Maximum slots allowed in processing window during trend FOLLOW phase 
	static unsigned int	ACTIVE_DS_TRIM_SIZE				= 1000;		//	Default buffer window size for Active DS container based on 25m daily tick average; yields ~5 minute window
	//	AnalyticX
	static double		TREND_UPPER_GRADIENT			= 1.25;		//	Trend slope range upper bound
	static double		TREND_LOWER_GRADIENT			= 0.75;		//	Trend slope range upper bound
	static double		CHANNEL_COMPRESS_RANGE_LIMIT	= 0.10;		//	Compress range limit multiplier as a percentage in decimal form (.01 is 1 %)
	static double		CHANNEL_GAP_RANGE_LIMIT			= 0.10;		//	Gap range limit multiplier as a percentage in decimal form (.01 is 1 %)
	static double		BASIS_POINT_ADJ					= 0.0025;	//	Basis point adjustment value (.0001 equals 1 BP)

	////////////////////////////////////
	//	Initial Values
	////////////////////////////////////
	// StructX
	static	int			param_SYMBOL_MAXSIZE			= SYMBOL_MAXSIZE;
	static	bool		param_DATA_COMPRESSION_ENABLED	= DATA_COMPRESSION_ENABLED;
	static	bool		param_DATA_GAP_ENABLED			= DATA_GAP_ENABLED;
	static	int			param_OUTLIER_COLLECTION_SIZE	= OUTLIER_COLLECTION_SIZE;
	static	double		param_MAX_OUTLIER_DELTA			= MAX_OUTLIER_DELTA;
	static	int			param_SUPERTICK_SIZE			= SUPERTICK_SIZE;
	static	int			param_MIN_SLOTS_FOR_TREND_CALCS = MIN_SLOTS_FOR_TREND_CALCS;
	static	int			param_NUM_SLOTS_TO_ESTABLISH_TREND	= NUM_SLOTS_TO_ESTABLISH_TREND;
	static	int			param_MAX_SLOTS_ON_FOLLOW		= MAX_SLOTS_ON_FOLLOW;
	static unsigned int	param_ACTIVE_DS_TRIM_SIZE		= ACTIVE_DS_TRIM_SIZE;
	// AnalyticX
	static double		param_TREND_UPPER_GRADIENT		= TREND_UPPER_GRADIENT;
	static double		param_TREND_LOWER_GRADIENT		= TREND_LOWER_GRADIENT;
	static double		param_CHANNEL_COMPRESS_RANGE_LIMIT = CHANNEL_COMPRESS_RANGE_LIMIT;
	static double		param_CHANNEL_GAP_RANGE_LIMIT	= CHANNEL_GAP_RANGE_LIMIT;
	static double		param_BASIS_POINT_ADJ			= BASIS_POINT_ADJ;

}

using namespace ATS_STRUCT_DATA_DEFAULTS;






// Forward DECLS for tracking
class WatchPool;				//	WatchPool header block 
class WatchPoolItem;			//	WatchPool items on ready queue
class WatchPoolItemSequence;	//	?

class ProviderTradeMsg;			//	RDQ item
class DS_Active_CTX_Block;		//	Aggregate container for active contexts
class DS_Active_CTX;			//	Active collection context reference object
class DS_Context;				//	Holds collection context active components (ticks, s_ticks, etc..)

class ActiveAnalyticCtxBlock;	//	Active Analytics header block holds Map of active symbols for processing
class ActiveAnalyticCtx;		//	Active Analytic context map entry; holds active context list and safe sync obj
class CoreAnalyticCtx;			//	
class AnalyticCtx;				//	Active context holds analytic data collection and algorithmic functions
class ATS_Analytic;				//	TBD

class AnalyticLookAheadBuffer;	//	Look ahead buffer processing
class AnalyticSuperTickBuffer;	//	SuperTick collection processing




///////////////////////////////////////////////////////////////////////////////
//	Class StructX
//	- Multiple instance base class for global component access
//	- Globals are efficiently initialized for each instance
///////////////////////////////////////////////////////////////////////////////

class StructX
{
public:
	StructX();
	StructX(void*);
	~StructX();

	static DS_Globals*	pGlobals;
	static LoggerX*		pLogger;


private:
	// Declare private methods and objects
	StructX(const StructX &);
	StructX& operator=(const StructX&);

	AutoCriticalSection	StructX_cs;

	static int isInitialized;
	static int getObjectState();
	static int setObjectState();

	ATS_CODE initialize(void*);
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	Data Collection Structures
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
//	class WatchPool
//	- Header block containing the pool of securities potentially eligible for 
//	  position calculation and trading
//	- List is acquired from the ATS database during initial load
///////////////////////////////////////////////////////////////////////////////
class WatchPool
{
public:
	WatchPool();
	~WatchPool();

	//	TODO: 
	//	- build SPI calls
	//	- ...
	std::map<std::wstring, WatchPoolItem*> watchPool;



private:
	// Declare private methods and objects
	WatchPool(const WatchPool &);
	WatchPool& operator=(const WatchPool&);
};



///////////////////////////////////////////////////////////////////////////////
//	class WatchPoolItem
//	- Contains individual security eligible for position calculation and trading
//	- Item is acquired from the ATS database WatchPoolRecList
///////////////////////////////////////////////////////////////////////////////
class WatchPoolItem
{
public:
	WatchPoolItem();
	~WatchPoolItem();

	//	TODO: 
	//	- build item out
	//	- ...
	unsigned int	sequence;
	std::wstring	symbol;
	int				avgVolume;
	bool			watchEligible;
	bool			tradeEligible;
	//SQL_TIMESTAMP_STRUCT createDT;
	//SQL_TIMESTAMP_STRUCT updateDT;
	std::wstring	exchange;
	std::wstring	description;

private:
	// Declare private methods and objects
	WatchPoolItem(const WatchPoolItem &);
	WatchPoolItem& operator=(const WatchPoolItem&);
};


///////////////////////////////////////////////////////////////////////////////
//	class WatchPoolItemSequence (Sequence lookups)
//	- Contains MAP of security sequence numbers for 'symbol to sequence' conversion
///////////////////////////////////////////////////////////////////////////////
class WatchPoolItemSequence
{
public:
	WatchPoolItemSequence();
	~WatchPoolItemSequence();

	std::map<wstring, unsigned int> symToseqNumLookupHash;

	CRITICAL_SECTION sso;	//	All operations on the MAP are synchronized through this object

private:
	// Declare private methods and objects
	WatchPoolItemSequence(const WatchPoolItemSequence &);
	WatchPoolItemSequence& operator=(const WatchPoolItemSequence&);

};



///////////////////////////////////////////////////////////////////////////////
//	class ProviderTradeMsg
//	- These trade records are placed in the data collection dequeues
//	- The records are formatted RDQ message derivatives
///////////////////////////////////////////////////////////////////////////////
class ProviderTradeMsg
{
public:
	ProviderTradeMsg();
	~ProviderTradeMsg();
	ProviderTradeMsg(const ProviderTradeMsg &);

	//	TODO: 
	//	- amount (this trade amount)
	//	- volume (this trade volume)
	//	- timestamp (date and time of this trade)
	//	- other...
	//
	//	- Add an object for analytics processing to the end of this record

private:
	// Declare private methods and objects
	ProviderTradeMsg & operator=(const ProviderTradeMsg&);


};


///////////////////////////////////////////////////////////////////////////////
//	class DS_Active_CTX_Block
//	- Header block containing the pool of DS structures Actively being watched for 
//	  position calculation and trading
//	- List is acquired from the ATS database during initial load
///////////////////////////////////////////////////////////////////////////////
class DS_Active_CTX_Block
{
public:
	DS_Active_CTX_Block();
	~DS_Active_CTX_Block();

	//	TODO: 
	//	- build SPI calls
	//	- ...
	std::map<unsigned int, DS_Active_CTX*> active_CTX_Container;

	ATS_CODE createActiveCTXRecord(WatchPoolItem*);
	ATS_CODE removeActiveCTXRecord(unsigned int);


private:
	// Declare private methods and objects
	DS_Active_CTX_Block(const DS_Active_CTX_Block &);
	DS_Active_CTX_Block& operator=(const DS_Active_CTX_Block&);
};


///////////////////////////////////////////////////////////////////////////////
//	class DS_Active_CTX
//	- Contains header block for individual security active collection context
//	
///////////////////////////////////////////////////////////////////////////////
class DS_Active_CTX
{
public:
	DS_Active_CTX();
	~DS_Active_CTX();

	//	TODO: 
	//	- build item out
	//	- ...
	CS_Sync_External	DS_Active_sso;	// Context synchronization lock
	DS_Context*			pCtx;			// Operational context object 
	std::wstring		symbol;			// Backward symbol lookup


private:
	// Declare private methods and objects
	DS_Active_CTX(const DS_Active_CTX &);
	DS_Active_CTX& operator=(const DS_Active_CTX&);

	ATS_CODE initialize(void);
};


///////////////////////////////////////////////////////////////////////////////
//	class DS_Context
//	- Holds collection context active components (active flag, ticks, etc..)
//	
///////////////////////////////////////////////////////////////////////////////
class DS_Context
{
public:
	DS_Context();
	~DS_Context();
	ATS_CODE initialize();

	unsigned int activeFlag;
	CircularBuffer<TICK_DATA> tickBuf;


private:
	// Declare private methods and objects
	DS_Context(const DS_Context &);
	DS_Context& operator=(const DS_Context&);
};


///////////////////////////////////////////////////////////////////////////////
//	class PredecessorBusyHash
//	- Keeps track of busy event "symbols" in RDQ
///////////////////////////////////////////////////////////////////////////////
class PredecessorBusyHash
{
public:
	PredecessorBusyHash();
	~PredecessorBusyHash();

	// Data
	CS_Sync_External predBusyHash_sso;

	// Methods
	ATS_CODE	initialize(void);
	ATS_CODE	add(unsigned int);			// Adds symbol to hash - used upon initial load
	ATS_CODE	remove(unsigned int);
	ATS_CODE	value(unsigned int, bool&);
	ATS_CODE	set(unsigned int);			// Set busy
	ATS_CODE	reset(unsigned int);		// Reset not busy

private:
	// Declare private methods and objects
	PredecessorBusyHash(const PredecessorBusyHash&);
	PredecessorBusyHash& operator=(const PredecessorBusyHash&);

	// Data
	std::map<unsigned int, bool> predecessor;

};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	Data Analytic Structures
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//	class NextReadyQBusyHash (NRQ Hash)
//	- Keeps track of busy event "symbols" in Next Ready Queue
///////////////////////////////////////////////////////////////////////////////
#define NRQ_ELEMENT_LIMIT	2

enum NRQ_ELEMENT_BUSY_FLAG
{
	NOT_BUSY = 0,
	BUSY = 1
};

enum TREND_PATTERN_STATE
{
	INITIAL = 0,
	START,
	RESTART,
	ENTER,
	ESTABLISH,
	FOLLOW,
	CLOSE,
	EXIT
};

typedef struct NRQ_ELEMENT
{
	NRQ_ELEMENT_BUSY_FLAG flag;
	int count;
} NRQ_ELEMENT;


class NextReadyQBusyHash
{
public:
	NextReadyQBusyHash();
	~NextReadyQBusyHash();

	// Data
	CS_Sync_External nextReadyQBusyHash_sso;

	// Methods
	ATS_CODE	initialize(void);
	ATS_CODE	add(unsigned int);			// Adds symbol to hash - used upon initial load
	ATS_CODE	remove(unsigned int);
	ATS_CODE	get_value(unsigned int, NRQ_ELEMENT&);
	ATS_CODE	modify_count(unsigned int, NRQ_ELEMENT&);
	ATS_CODE	set_state(unsigned int, NRQ_ELEMENT_BUSY_FLAG);
	ATS_CODE	incr_count(unsigned int);
	ATS_CODE	decr_count(unsigned int);

private:
	// Declare private methods and objects
	NextReadyQBusyHash(const NextReadyQBusyHash&);
	NextReadyQBusyHash& operator=(const NextReadyQBusyHash&);

	// Data
	std::map<unsigned int, NRQ_ELEMENT> nextReady;


};


///////////////////////////////////////////////////////////////////////////////
//	class ActiveAnalyticCtxBlock
//	- Header block containing the pool of securities potentially eligible for 
//	  position calculation and trading
//	- List is acquired from the ATS database during initial load
///////////////////////////////////////////////////////////////////////////////
class ActiveAnalyticCtxBlock
{
public:
	ActiveAnalyticCtxBlock(void*);
	~ActiveAnalyticCtxBlock();

	DS_Globals* pGlobals;

	// Map holds active contexts indexed by seq number
	std::map<unsigned int, ActiveAnalyticCtx*> activeAnalyticCtxContainer;
	ATS_CODE createContainerRecord(WatchPoolItem*);
	ATS_CODE removeContainerRecord(unsigned int);


private:
	// Declare private methods and objects
	ActiveAnalyticCtxBlock(const ActiveAnalyticCtxBlock &);
	ActiveAnalyticCtxBlock& operator=(const ActiveAnalyticCtxBlock&);

	ATS_CODE initialize(void*);

};


///////////////////////////////////////////////////////////////////////////////
//	class ActiveAnalyticCtx
//	- Each context is a unique Map item
//	- This object identifies a trade eligible security
//	- Analytics list contains all registered handlers used for position 
//	  calculations uniquely identified by its symbol
//	- Item symbol is acquired from the ATS database WatchPoolRecList
///////////////////////////////////////////////////////////////////////////////
class ActiveAnalyticCtx
{
public:
	ActiveAnalyticCtx();
	~ActiveAnalyticCtx();

	CS_Sync_External ActveAnalyticCtx_sso;					//	Synchronizes execution to all registered analytic handlers for context
	std::list<CoreAnalyticCtx*> activeAnalyticCtxList;		//	List of registered analytic handlers for context
	std::wstring	symbol;	//	For lookup purposes

	//	Helpers
	ATS_CODE copyListtoQ(std::list<CoreAnalyticCtx*>, std::queue< CoreAnalyticCtx*>&);


private:
	// Declare private methods and objects
	ActiveAnalyticCtx(const ActiveAnalyticCtx &);
	ActiveAnalyticCtx& operator=(const ActiveAnalyticCtx&);

	ATS_CODE initialize(void);	//	Initialize analytics list
};


///////////////////////////////////////////////////////////////////////////////
//	class AnalyticLookAheadBuffer
//	- Contains Lookahead Tick buffer data
//	- Used for Outlier filtering and ST composition processing
//	- Holds data acquired from DS collection only until the data is processed 
//	  into super ticks
///////////////////////////////////////////////////////////////////////////////
class AnalyticLookAheadBuffer : public StructX
{
public:
	AnalyticLookAheadBuffer();
	~AnalyticLookAheadBuffer();

	int		outlierLimit;
	double	percentDiff;
	Container_forward_iterator lookAheadBegin();
	Container_forward_iterator lookAheadEnd();
	int getLookAheadCount(void);
	ATS_CODE getTickCount(unsigned int&);
	ATS_CODE getPivotAmt(double&);
	ATS_CODE setPivotAmt(double&);
	ATS_CODE getLastUid(Container_element_uid&);
	ATS_CODE setLastUidProcessed(Container_element_uid&);
	ATS_CODE setUidUnused(Container_element_uid&);
	ATS_CODE lookupUid(Container_forward_iterator&, Container_element_uid&);
	ATS_CODE invalidateSlot(Container_element_uid&);
	ATS_CODE setSlotState(Container_element_uid&, CIRCULAR_BUFFER_STATES);
	ATS_CODE convertUidToIterator(Container_element_uid&, Container_forward_iterator&);
	ATS_CODE getTickContext(Container_forward_iterator&, TICK_DATA*&);
	ATS_CODE getTickContainer(Container_forward_iterator&, CONTAINER_CTX_BLOCK<TICK_DATA>*&);
	ATS_CODE getUidContainer(Container_element_uid&, CONTAINER_CTX_BLOCK<TICK_DATA>*&);
	ATS_CODE insertTickData(TICK_DATA*);
	ATS_CODE findSuccessorUid(Container_element_uid&, Container_element_uid&);
	Container_forward_iterator		increment(Container_forward_iterator&);
	stack<Container_element_uid>	outlierStack;

	unsigned int	lastActiveDSslotUid;

private:
	AnalyticLookAheadBuffer(const AnalyticLookAheadBuffer &);
	AnalyticLookAheadBuffer& operator=(const AnalyticLookAheadBuffer&);

	ATS_CODE initialize();

	CircularBuffer<TICK_DATA>	lookAheadBuffer;

	unsigned int	lastUid;			// Where we left off while processing the collection
	double			pivotAmt;			// Last valid tick amount used for out-lier calculation


};


///////////////////////////////////////////////////////////////////////////////
//	class TrendProperties
//	- Contains ST Buffer properties for trend analysis
///////////////////////////////////////////////////////////////////////////////
class TrendProperties : public StructX
{
public:
	TrendProperties();
	~TrendProperties();

	unsigned int	patternState;	// Current state of trend analysis
	unsigned int	lastUID;		// Last UID processed in trend data


private:
	TrendProperties(const TrendProperties &);
	TrendProperties& operator=(const TrendProperties&);

	ATS_CODE initialize();

};


///////////////////////////////////////////////////////////////////////////////
//	class AnalyticSuperTickBuffer
//	- Contains Super Tick buffer and processing properties
///////////////////////////////////////////////////////////////////////////////
class AnalyticSuperTickBuffer : public StructX
{
public:
	AnalyticSuperTickBuffer();
	~AnalyticSuperTickBuffer();

	Container_forward_iterator superTickBegin();
	Container_forward_iterator superTickEnd();

	TrendProperties trendData;

	int getActiveSlotCount(void); 
	ATS_CODE getActiveSlotCount(unsigned int&);
	ATS_CODE getSlotState();
	ATS_CODE getTickContainer(Container_forward_iterator&, CONTAINER_CTX_BLOCK<S_TICK_DATA>*&);	// Get address of element container
	ATS_CODE insertTickData(S_TICK_DATA*);
	ATS_CODE invalidateSlot(Container_element_uid&);
	bool  isValidSlot(Container_forward_iterator&);
	ATS_CODE trimFront(unsigned int);
	ATS_CODE emptyBuffer(void);
	ATS_CODE convertUID(Container_element_uid&, Container_forward_iterator&);
	Container_forward_iterator increment(Container_forward_iterator&);


private:
	AnalyticSuperTickBuffer(const AnalyticSuperTickBuffer &);
	AnalyticSuperTickBuffer& operator=(const AnalyticSuperTickBuffer&);

	ATS_CODE initialize();
	CircularBuffer<S_TICK_DATA>	superTickBuffer;	//	Default sizing

};


///////////////////////////////////////////////////////////////////////////////
//	class CoreAnalyticCtx - Core Components Base Class
///////////////////////////////////////////////////////////////////////////////
class CoreAnalyticCtx : public StructX
{
public:
	CoreAnalyticCtx();
	~CoreAnalyticCtx();

	virtual ATS_CODE invokeHandlers(unsigned int, DS_Active_CTX*) { return ATS_C_SUCCESS; };

	std::wstring	desc;
	wchar_t			logMsg[256];

	//	++Add Analytics and Math libraries
	LTLS_Analysis*	pAELib;	// Analytic engine lib (contains all analytic calls)
	//	--Add Analytics and Math libraries

private:
	CoreAnalyticCtx(const CoreAnalyticCtx &);
	CoreAnalyticCtx& operator=(const CoreAnalyticCtx&);

	virtual ATS_CODE initialize();
	virtual ATS_CODE lookaheadHandler() { return ATS_C_SUCCESS; };
	virtual ATS_CODE outlierHandler() { return ATS_C_SUCCESS; };
	virtual ATS_CODE dataSmoothingHandler() { return ATS_C_SUCCESS; };
	virtual ATS_CODE supertickHandler() { return ATS_C_SUCCESS; };
	virtual ATS_CODE trendAnalysisHandler() { return ATS_C_SUCCESS; };
	virtual ATS_CODE reportHandler() { return ATS_C_SUCCESS; };
	virtual ATS_CODE notificationHandler() { return ATS_C_SUCCESS; }

};


///////////////////////////////////////////////////////////////////////////////
//	class Analytic_NOOP
//	- Dummy Analytic - Implementation Class
//		- Every symbol is assigned this Analytic
//		- Used for tracing and debugging (does nothing)
//
///////////////////////////////////////////////////////////////////////////////
class AnalyticCtx_NOOP : public CoreAnalyticCtx
{
public:
	AnalyticCtx_NOOP();
	~AnalyticCtx_NOOP();

	//	++Implement Virtual Methods
	ATS_CODE invokeHandlers(unsigned int, DS_Active_CTX*);
	//	--Implement Virtual Methods

	std::wstring	desc;


private:
	// Declare private methods and objects
	AnalyticCtx_NOOP(const AnalyticCtx_NOOP &);
	AnalyticCtx_NOOP& operator=(const AnalyticCtx_NOOP&);

	ATS_CODE initialize(void);
	//	++Implement Virtual Methods
	ATS_CODE lookaheadHandler(DS_Active_CTX*);
	ATS_CODE outlierHandler(DS_Active_CTX*);
	ATS_CODE dataSmoothingHandler(DS_Active_CTX*);
	ATS_CODE supertickHandler(DS_Active_CTX*);
	ATS_CODE trendAnalysisHandler(DS_Active_CTX*);
	ATS_CODE reportHandler(DS_Active_CTX*);
	ATS_CODE notificationHandler(DS_Active_CTX*);
	//	--Implement Virtual Methods

};


///////////////////////////////////////////////////////
//	class Analytics_LTLS
//	- Linear Trend Long Short
//
///////////////////////////////////////////////////////
class AnalyticCtx_LTLS : public CoreAnalyticCtx
{
public:
	AnalyticCtx_LTLS(void*);
	~AnalyticCtx_LTLS();

	//	++Implement Virtual Methods
	ATS_CODE invokeHandlers(unsigned int, DS_Active_CTX*);
	//	--Implement Virtual Methods

	std::wstring	desc;
	//DS_Globals*		pGlobals;
	//LoggerX*		pLogger;

	AnalyticLookAheadBuffer* pLookAhead;
	AnalyticSuperTickBuffer* pSuperTick;

	//	++Add trend management data and routines
	DataAggregateContainer*	pDAContainer;
	//	--Add trend management data and routines



private:
	// Declare private methods and objects
	AnalyticCtx_LTLS();
	AnalyticCtx_LTLS(const AnalyticCtx_LTLS &);
	AnalyticCtx_LTLS& operator=(const AnalyticCtx_LTLS&);

	ATS_CODE initialize(void*);
	//	++Implement Virtual Methods
	ATS_CODE lookaheadHandler(DS_Active_CTX*);
	ATS_CODE outlierHandler(DS_Active_CTX*);
	ATS_CODE dataSmoothingHandler(DS_Active_CTX*);
	ATS_CODE supertickHandler(DS_Active_CTX*);
	ATS_CODE trendAnalysisHandler(DS_Active_CTX*);
	ATS_CODE reportHandler(DS_Active_CTX*);
	ATS_CODE notificationHandler(DS_Active_CTX*);
	//	--Implement Virtual Methods


};


///////////////////////////////////////////////////////
//	class Analytics_ETLS
//	- Exponential Trend Long Short
//
///////////////////////////////////////////////////////
class AnalyticCtx_ETLS : public CoreAnalyticCtx
{
public:
	AnalyticCtx_ETLS();
	~AnalyticCtx_ETLS();

	//	++Implement Virtual Methods
	ATS_CODE invokeHandlers(unsigned int, DS_Active_CTX*);
	//	--Implement Virtual Methods


	std::wstring	desc;

private:
	// Declare private methods and objects
	AnalyticCtx_ETLS(const AnalyticCtx_ETLS &);
	AnalyticCtx_ETLS& operator=(const AnalyticCtx_ETLS&);

	ATS_CODE initialize(void);
	//	++Implement Virtual Methods
	ATS_CODE lookaheadHandler(DS_Active_CTX*);
	ATS_CODE outlierHandler(DS_Active_CTX*);
	ATS_CODE dataSmoothingHandler(DS_Active_CTX*);
	ATS_CODE supertickHandler(DS_Active_CTX*);
	ATS_CODE trendAnalysisHandler(DS_Active_CTX*);
	ATS_CODE reportHandler(DS_Active_CTX*);
	ATS_CODE notificationHandler(DS_Active_CTX*);
	//	--Implement Virtual Methods

};



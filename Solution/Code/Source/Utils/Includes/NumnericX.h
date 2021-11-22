#pragma once

#include "../Includes/CommonX.h"
#include <cmath>




#define	TREND_AGGR_SIZE_LIMIT		200


///////////////////////////////////////////////////////////
//	DataAggregateXYValue
//	- Contains SuperTIck value along with last tick 
//	  value in set
///////////////////////////////////////////////////////////
struct DataAggregateXYValue
{
	double	sTickValue;
	double	lastTickValueInSet;
	//	Add lastTickTimeInSet

	DataAggregateXYValue();
};


///////////////////////////////////////////////////////////
//	DataAggregate
//	- Contains an array for a specific data aggregate
//	  instance
//	- The lifecycle for this object only lasts for as long 
//	  as the trend is active
///////////////////////////////////////////////////////////
class DataAggregate
{
public:
	DataAggregate();
	~DataAggregate();

//	double* arr;
	DataAggregateXYValue* yValueArray;	//	Holds X- or Y-Value objects
	double lowValue;
	double highValue;
	int sizeLimit;
	int currentSize;
	int index;							//	Points to the next unoccupied slot; index==0 implies arr is empty


//	ATS_CODE	pushBack(double);					//	Adds to the current slot pointed to by index
	ATS_CODE	pushBack(DataAggregateXYValue&);	//	Adds to the current aggregate slot pointed to by index
	ATS_CODE	popFront(void);						//	Remove lowest index item
	ATS_CODE	popFront(int);						//	Remove (n) lowest index items 
	ATS_CODE	pop(void);							//	Remove highest index item
	int			getCount(void);						//	Returns number of slots currently occupied
	ATS_CODE	empty(void);						//	Resets aggregate with storage reallocation
	ATS_CODE	clear(void);						//	Resets Aggregate
	ATS_CODE	findLowerBound(double&);			//	Finds value
	ATS_CODE	findUpperBound(double&);			//	Finds value


private:
	// Declare private methods and objects
	DataAggregate(const DataAggregate&);
	DataAggregate& operator=(const DataAggregate&);

	ATS_CODE initialize(void);
	ATS_CODE expand(void);

};


///////////////////////////////////////////////////////////////////////////////
//	class DataAggregateContainer
//	- Contains all DataAggregates required for trend analysis
///////////////////////////////////////////////////////////////////////////////
class DataAggregateContainer
{
public:
	DataAggregateContainer();
	~DataAggregateContainer();

	DataAggregate Y_DataValues;		//	Y-Value Aggregate
	DataAggregate Y_DeltaValues;	//	Y-Delta Value Aggregate
	DataAggregate X_DataValues;		//	X-Value Aggregate
	double	sumX;					//	Sum of X-Values
	double	sumY;					//	Sum of Y-Values
	double	sumXY;					//	Sum of X*Y-Values
	double	sumXsq;					//	Sum of X^2
	double	ahld;					//	AHLD
	double	sumMSqD;				//	Sum of X-values Mean Sq Diffs

	double	m;						//	Slope in gradients for Y_DataValues and X_DataValues
	double	sD;						//	Standard deviation for Y_Delta Values
	double	a;						//	Mean of Y_DataValues
	double	aD;						//	Mean of Y_DeltaValues
	double	b;						//	Y-Intercept used to predict next point on trend line's X-Value
	int		bias;					//	Trend direction (1 is positive and 0 is negative)

	double	currMidpoint;			//	Last calculated midpoint
	double	currUpperBound;			//	Last calculated upper bound
	double	currLowerBound;			//	Last calculated lower bound
	double	midPointGapAmt;			//	Gap between current and last mid-points

	unsigned int	state;			//	Processing state (TREND_PATTERN_STATE::ESTABLISH | TREND_PATTERN_STATE::FOLLOW)

	bool	recycledDAContainer;	//	Signifies the container has been recycled

	//	++Public Methods
	ATS_CODE recycle(void);			// Recycle container objects and scalar values
	//	--Public Methods


private:
	DataAggregateContainer(const DataAggregateContainer &);
	DataAggregateContainer& operator=(const DataAggregateContainer&);

	//	++Private Methods
	ATS_CODE initialize();
	//	--Private Methods

};


///////////////////////////////////////////////////////
//	Math Routines
///////////////////////////////////////////////////////
class NumnericX
{
public:
	NumnericX();
	~NumnericX();
	ATS_CODE sumArrayVars(DataAggregate&, double&);								// SUM    ( ARR[X] )
	ATS_CODE sumArraySqVars(DataAggregate&, double&);							// SUM    ( ARR[X**2] )

	ATS_CODE sumProdArrayVars(DataAggregate&, DataAggregate&, double&);			// SUM    ( PROD( ARR[X], ARR[Y] ) )
	ATS_CODE sumProdArraySqVars(DataAggregate&, DataAggregate&, double&);		// SUM    ( PROD( ARR[X**2], ARR[Y**2] ) )
	ATS_CODE sumSqProdArrayVars(DataAggregate&, DataAggregate&, double&);		// SUM    ( SQ ( PROD( ARR[X], ARR[Y]) ) )

	ATS_CODE calcMean(DataAggregate&, double&);									// MEAN   ( ARR[X] )
	ATS_CODE calcAvgHighLowDiff(DataAggregate&, double&);						// AHLD   ( ABS( ARR[X].MAX - ARR[X].MIN ) / SIZEOF ARR[X] )
	ATS_CODE calcBarValues(DataAggregate&, DataAggregate&, double&);			// BARS   ( MIN(Y) + (INDEX * AHLD) )
	ATS_CODE calcBarValues(DataAggregate&, DataAggregate&);						// BARS   ( MIN(Y) + CALC(IncValue) )

	ATS_CODE calcDeltaValues(DataAggregate&, DataAggregate&);					// DELTA  ( DELTA (ARR[X] )
	ATS_CODE calcStdDevS(DataAggregate&, double&, double&);						// STD.S  ( ARR[X] )

	//	Complex calculations require DataAggregateContainer
	//	and prerequisite calculations
	ATS_CODE calcTrend(DataAggregate&, DataAggregate&, double&, double&, double&);	// TREND  ( ARR[X], ARR[Y] )
	ATS_CODE calcY_Intercept(DataAggregate&, double&, double&, double&, double&);	// Y_INT  ( ARR[X], ARR[Y] )

	//	Upper / Lower Band and  Mid-Point calculations
	ATS_CODE calcMidPoint(DataAggregate&, double&, double&, double&, double&);



private:
	// Declare private methods and objects
	NumnericX(const NumnericX &);
	NumnericX& operator=(const NumnericX&);

	ATS_CODE initialize();

};



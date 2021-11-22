///////////////////////////////////////////////////////////////////////////////
//	NumnericX
//
//	Contains all abstract numeric objects
//	Includes:
//		- Decimal Math
//		- Matrix Math
//	
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../Includes/NumnericX.h"





///////////////////////////////////////////////////////
//	Class DataAggregateXYValue
//	- Value Array Aggregate
///////////////////////////////////////////////////////
DataAggregateXYValue::DataAggregateXYValue()
{
	sTickValue = 0.0;
	lastTickValueInSet = 0.0;
}


///////////////////////////////////////////////////////
//	Class DataAggregate
//	- Trend Aggregate Container and Routines
///////////////////////////////////////////////////////
DataAggregate::DataAggregate()
{
	ATS_CODE rc;
	rc = initialize();
}


DataAggregate::~DataAggregate()
{

}


ATS_CODE DataAggregate::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		yValueArray = NULL;
		sizeLimit = TREND_AGGR_SIZE_LIMIT;
		currentSize = sizeLimit;
		index = 0;
		lowValue = 0.0;
		highValue = 0.0;
		yValueArray = new DataAggregateXYValue[sizeLimit];

	}
	catch (...)
	{
		//	Process exception
	}

	return rc;
}


ATS_CODE DataAggregate::expand()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	//	Dynamically expands arr by sizeLimit
	try
	{
		int newSize = currentSize + sizeLimit;
		DataAggregateXYValue* newArr = new DataAggregateXYValue[newSize];

		for (int i = 0; i < currentSize; i++)
		{
			newArr[i].sTickValue = yValueArray[i].sTickValue;
			newArr[i].lastTickValueInSet = yValueArray[i].lastTickValueInSet;
		}
		delete[] yValueArray;
		yValueArray = newArr;
		currentSize = newSize;
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


ATS_CODE DataAggregate::empty()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		index = 0;
		lowValue = 0.0;
		highValue = 0.0;
		// Reallocate storage if size exceeds TREND_AGGR_SIZE_LIMIT
		if (currentSize > sizeLimit)
		{
			delete[] yValueArray;
			currentSize = sizeLimit;
			yValueArray = new DataAggregateXYValue[currentSize];
		}
	}
	catch (...)
	{

	}
	return rc;
}


ATS_CODE DataAggregate::clear()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		index = 0;
		lowValue = 0.0;
		highValue = 0.0;
	}
	catch (...)
	{

	}
	return rc;
}


ATS_CODE DataAggregate::findLowerBound(double& val)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	//	Validate operation
	if (index == 0)
	{
		val = 0.0;
		rc = ATS_C_EMPTY;
	}
	else
	{
		//	Finds lowest value
		double	cmpVal = yValueArray[0].sTickValue;
		for (int i = 1; i < index; i++)
		{
			if (yValueArray[i].sTickValue < cmpVal)
			{
				cmpVal = yValueArray[i].sTickValue;
			}
		}
		val = cmpVal;
	}
	return rc;
}


ATS_CODE DataAggregate::findUpperBound(double& val)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	//	Validate operation
	if (index == 0)
	{
		val = 0.0;
		rc = ATS_C_EMPTY;
	}
	else
	{
		//	Finds lowest value
		double	cmpVal = yValueArray[0].sTickValue;
		for (int i = 1; i < index; i++)
		{
			if (yValueArray[i].sTickValue > cmpVal)
			{
				cmpVal = yValueArray[i].sTickValue;
			}
		}
		val = cmpVal;
	}
	return rc;
}


ATS_CODE DataAggregate::pushBack(DataAggregateXYValue& yValueAggr)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		//	Validate operation
		if (index == currentSize)
		{
			rc = expand();
			//	Check status
		}
		//arr[index] = val;	//	Adds new entry
		yValueArray[index].sTickValue = yValueAggr.sTickValue;
		yValueArray[index].lastTickValueInSet = yValueAggr.lastTickValueInSet;
		if (index == 0)
		{
			lowValue = highValue = yValueAggr.sTickValue;
		}
		else
		{
			if (yValueAggr.sTickValue > highValue)
			{
				highValue = yValueAggr.sTickValue;
			}
			else if (yValueAggr.sTickValue < lowValue)
			{
				lowValue = yValueAggr.sTickValue;
			}
		}
		index++;
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


ATS_CODE DataAggregate::popFront()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	rc = popFront(1);
	return rc;
}


ATS_CODE DataAggregate::popFront(int num_items)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	//	Validate operation
	if (index == 0)
	{
		return ATS_C_EMPTY;
	}

	if (num_items > index)
	{
		return ATS_C_RANGE;
	}

	//	Remove item(s) beginning at lowest index 
	DataAggregateXYValue* srcArr = &yValueArray[num_items];
	DataAggregateXYValue* dstArr = &yValueArray[0];

	//	Shifts everything in Array down num_items (gets rid of older entries)
	for (int i = index - num_items; i > 0; i--, dstArr++, srcArr++)
	{
		*dstArr = *srcArr;	//	Validate this copy works !!!
	}

	index = index - num_items;

	//	Adjust high / low value
	if (index == 0)
	{
		clear();
	}
	else
	{
		findLowerBound(lowValue);
		findUpperBound(highValue);
	}
	return rc;
}


ATS_CODE DataAggregate::pop()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	//	Validate operation
	if (index == 0)
	{
		return ATS_C_EMPTY;	//	Empty
	}

	//	Removes highest index element
	index--;

	//	Adjust high / low value
	if (index == 0)
	{
		clear();
	}
	else
	{
		findLowerBound(lowValue);
		findUpperBound(highValue);
	}
	return rc;
}


int DataAggregate::getCount()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	return index;
}


///////////////////////////////////////////////////////
//	Class NumericX 
//	- Math library Routines Container
///////////////////////////////////////////////////////
NumnericX::NumnericX()
{
	ATS_CODE rc;
	rc = initialize();
}


NumnericX::~NumnericX()
{
}


ATS_CODE NumnericX::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;
	return rc;
}


//	SUM ( ARR[X] )
ATS_CODE NumnericX::sumArrayVars(DataAggregate& aggr, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggr.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			double sum = 0.0;
			for (int i = 0; i < aggr.index; i++)
			{
				sum += aggr.yValueArray[i].sTickValue;
			}
			retVal = sum;
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	SUM ( ARR[X**2] )
ATS_CODE NumnericX::sumArraySqVars(DataAggregate& aggr, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggr.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			double sum = 0.0;
			double sq = 0.0;
			for (int i = 0; i < aggr.index; i++)
			{
				sq = aggr.yValueArray[i].sTickValue * aggr.yValueArray[i].sTickValue;
				sum += sq;
			}
			retVal = sum;
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	SUM ( PROD ( ARR[X], ARR[Y] ) )
ATS_CODE NumnericX::sumProdArrayVars(DataAggregate& aggrX, DataAggregate& aggrY, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggrX.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			if (aggrX.index != aggrY.index)
			{
				retVal = 0.0;
				rc = ATS_C_RANGE;
			}
			else
			{
				double sum = 0.0;
				for (int i = 0; i < aggrX.index; i++)
				{
					sum += (aggrX.yValueArray[i].sTickValue * aggrY.yValueArray[i].sTickValue);
				}
				retVal = sum;
			}
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	SUM ( PROD( ARR[X**2], ARR[Y**2] ) )
ATS_CODE NumnericX::sumProdArraySqVars(DataAggregate& aggrX, DataAggregate& aggrY, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggrX.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			if (aggrX.index != aggrY.index)
			{
				retVal = 0.0;
				rc = ATS_C_RANGE;
			}
			else
			{
				double sum = 0.0;
				for (int i = 0; i < aggrX.index; i++)
				{
					sum += ((aggrX.yValueArray[i].sTickValue * aggrX.yValueArray[i].sTickValue) * (aggrY.yValueArray[i].sTickValue * aggrY.yValueArray[i].sTickValue));
				}
				retVal = sum;
			}
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	SUM ( SQ ( PROD( ARR[X], ARR[Y]) ) )
ATS_CODE NumnericX::sumSqProdArrayVars(DataAggregate& aggrX, DataAggregate& aggrY, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggrX.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			if (aggrX.index != aggrY.index)
			{
				retVal = 0.0;
				rc = ATS_C_RANGE;
			}
			else
			{
				double sum = 0.0;
				double prod = 0.0;
				for (int i = 0; i < aggrX.index; i++)
				{
					prod = (aggrX.yValueArray[i].sTickValue * aggrY.yValueArray[i].sTickValue);
					sum += (prod * prod);
				}
				retVal = sum;
			}
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	MEAN ( ARR[X] )
ATS_CODE NumnericX::calcMean(DataAggregate& aggr, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggr.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			double sum = 0.0;
			for (int i = 0; i < aggr.index; i++)
			{
				sum += aggr.yValueArray[i].sTickValue;
			}
			retVal = sum / (aggr.index);
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	AHLD ( ABS( ARR[X].MAX - ARR[X].MIN ) / SIZEOF ARR[X] )
ATS_CODE NumnericX::calcAvgHighLowDiff(DataAggregate& aggr, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggr.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			double ahld = 0.0;
			ahld = fabs((aggr.highValue - aggr.lowValue) / (double)aggr.index);
			retVal = ahld;
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	BARS ( MIN(Y) + (INDEX * AHLD) )

//	Change this to calculate linear bars rather than using the average of high low deltas (AHLD)
ATS_CODE NumnericX::calcBarValues(DataAggregate& aggrY, DataAggregate& aggrX, double& AHLD)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	DataAggregateXYValue aggrValue;
	try
	{
		if (aggrY.index == 0)
		{
			rc = ATS_C_EMPTY;
		}
		else
		{
			//double AHLD = 0.0;
			//rc = calcAvgHighLowDiff(aggrY, AHLD);
			// Check return value
			//if (rc == ATS_C_SUCCESS)
			//{
				aggrX.clear();
				for (int i = 0; i < aggrY.index; i++)
				{
					aggrValue.sTickValue = aggrY.lowValue + ((double)i * AHLD);
					aggrValue.lastTickValueInSet = 0.0;
					rc = aggrX.pushBack(aggrValue);
					if (rc != ATS_C_SUCCESS)
					{
						// Report error
						break;
					}
				}
			//}
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	BARS ( MIN(Y) + CALC(IncValue) )
//	Change this to calculate linear bars rather than using the average of high low deltas (AHLD)
ATS_CODE NumnericX::calcBarValues(DataAggregate& aggrY, DataAggregate& aggrX)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	DataAggregateXYValue aggrValue;
	double incValue;
	try
	{
		if (aggrY.index == 0)
		{
			rc = ATS_C_EMPTY;
		}
		else
		{
			//	Calculate increment value
			incValue = (aggrY.highValue - aggrY.lowValue) / ((double)aggrY.index - 1.0);
			aggrX.clear();
			for (int i = 0; i < aggrY.index; i++)
			{
				aggrValue.sTickValue = aggrY.lowValue + (incValue * (double)i);
				aggrValue.lastTickValueInSet = 0.0;		//	Not used when aggregate contains X values
				rc = aggrX.pushBack(aggrValue);
				if (rc != ATS_C_SUCCESS)
				{
					// Report error
					break;
				}
			}
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	DELTA ( DELTA (ARR[X] )
ATS_CODE NumnericX::calcDeltaValues(DataAggregate& aggr, DataAggregate& aggrDelta)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	DataAggregateXYValue aggrValue;
	try
	{
		if (aggr.index == 0)
		{
			rc = ATS_C_EMPTY;
		}
		else
		{
			aggrDelta.clear();
			aggrValue.sTickValue = 0.0;
			aggrValue.lastTickValueInSet = 0.0;
			aggrDelta.pushBack(aggrValue);
			for (int i = 1; i < aggr.index; i++)
			{
				aggrValue.sTickValue = fabs(aggr.yValueArray[i].sTickValue - aggr.yValueArray[i - 1].sTickValue);
				aggrValue.lastTickValueInSet = 0.0;
				rc = aggrDelta.pushBack(aggrValue);
				if (rc != ATS_C_SUCCESS)
				{
					// Report error
					break;
				}
			}
		}
	}
	catch (...)
	{
		//	Process exception
	}
	return rc;
}


//	STD.S ( ARR[X] )
ATS_CODE NumnericX::calcStdDevS(DataAggregate& aggr, double& mean, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggr.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			double term1 = 0.0;
			double result = 0.0;
			double sumMSqD = 0.0;
			double s = 0.0;
			//	Calculate sumMSqD
			for (int i = 0; i < aggr.index; i++)
			{
				result = (aggr.yValueArray[i].sTickValue - mean);
				sumMSqD += (result * result);
			}

			//	Calculate stddev.s
			term1 = 1.0 / (aggr.index - 1.0);
			s = sqrt(term1*sumMSqD);
			retVal = s;
		}
	}
	catch (...)
	{
		//	Process exception
		rc = ATS_C_UNKNOWN;
	}
	return rc;
}


//	TREND ( ARR[X],  ARR[Y] )
ATS_CODE NumnericX::calcTrend(DataAggregate& aggrX, DataAggregate& aggrY, double& sumX, double& sumY, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggrX.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			if (aggrX.index != aggrY.index)
			{
				retVal = 0.0;
				rc = ATS_C_RANGE;
			}
			else
			{
				double n = aggrX.index;
				double sumXY, sumXSq;
				double m = 0.0;
				rc = sumProdArrayVars(aggrX, aggrY, sumXY);
				if (rc != ATS_C_SUCCESS)
				{
					// Process error
					goto calcTrend_leave;
				}
				rc = sumArraySqVars(aggrX, sumXSq);
				if (rc != ATS_C_SUCCESS)
				{
					// Process error
					goto calcTrend_leave;
				}
				// Calculate slope (gradient) value
				double numerator = ((n * sumXY) - (sumX * sumY));
				double denominator = ((n * sumXSq) - (sumX * sumX));
				if (isnan(denominator) || denominator == 0.0)
				{
					retVal = m;
				}
				else
				{
					retVal = numerator / denominator;
				}	
			}
		}
	}
	catch (...)
	{
		//	Process exception
		retVal = 0.0;
		rc = ATS_C_UNKNOWN;
	}
calcTrend_leave:
	return rc;
}


//	Y_Intercept ( ARR[X], ... )
ATS_CODE NumnericX::calcY_Intercept(DataAggregate& aggrY, double& slope, double& sumX, double& sumY, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggrY.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			retVal = (sumY - (slope * sumX)) / (aggrY.index);
		}
	}
	catch (...)
	{
		//	Process exception
		rc = ATS_C_UNKNOWN;
	}

	return rc;
}


//	Trend Midpoint ( Y = MX + B )
//	- X is the last X_Value in aggrX, M is the trend slope and B is the Y_Intercept
//
ATS_CODE NumnericX::calcMidPoint(DataAggregate& aggr, double& m, double& b, double& ahld, double& retVal)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	try
	{
		if (aggr.index == 0)
		{
			retVal = 0.0;
			rc = ATS_C_EMPTY;
		}
		else
		{
			double expectedX = aggr.yValueArray[aggr.index-1].sTickValue + ahld;

			retVal = (m * expectedX) + b;
		}
	}
	catch (...)
	{
		//	Process exception
		rc = ATS_C_UNKNOWN;
	}
	return rc;
}


///////////////////////////////////////////////////////////
//	DataAggregateContainer
//	- Contains all required data for trend processing
///////////////////////////////////////////////////////////
DataAggregateContainer::DataAggregateContainer()
{
	ATS_CODE rc;
	rc = initialize();
}


DataAggregateContainer::~DataAggregateContainer()
{

}

ATS_CODE DataAggregateContainer::initialize()
{
	ATS_CODE rc = ATS_C_SUCCESS;


	return rc;
}

///////////////////////////////////////////////////////////
//	Reset all objects in container
///////////////////////////////////////////////////////////
ATS_CODE DataAggregateContainer::recycle(void)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	Y_DataValues.empty();
	X_DataValues.empty();
	//
	sD = 0.0;
	m = 0.0;
	a = 0.0;
	aD = 0.0;
	sumX = 0.0;
	sumY = 0.0;
	sumXY = 0.0;
	sumXsq = 0.0;
	bias = 0;
	//
	currMidpoint = 0.0;
	midPointGapAmt = 0.0;
	currUpperBound = 0.0;
	currLowerBound = 0.0;
	//
	recycledDAContainer = true;
	//
	return rc;
}
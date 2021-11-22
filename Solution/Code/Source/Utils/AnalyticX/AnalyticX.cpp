// AnalyticX.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"
#include "../Includes/StructX.h"
#include "../Includes/AnalyticX.h"



//	Statics Initializations
int AnalyticX::isInitialized{ 0 };
DS_Globals*	AnalyticX::pGlobals{ NULL };
NumnericX*	AnalyticX::pNELib{ NULL };



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//	CLASS - AnalyticX
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
AnalyticX::AnalyticX()
{
	ATS_CODE rc;
	rc = initialize(NULL);
	return;
}

AnalyticX::AnalyticX(void* Args)
{
	ATS_CODE rc;
	rc = initialize(Args);
}


AnalyticX::~AnalyticX()
{
}

int AnalyticX::getObjectState()
{
	return isInitialized;
}


int AnalyticX::setObjectState()
{
	isInitialized = 1;
	return isInitialized;
}


ATS_CODE AnalyticX::initialize(void* Args)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	if (Args == NULL)
		return rc;

	AutoCriticalSectionLock AnalyticX_sso(AnalyticX_cs);
	if (getObjectState() != 0)
		return rc;

	setObjectState();

	try
	{
		// Set-up the "precise" contexts required for this Analytic instance
		pGlobals = (DS_Globals*)Args;

		if (!pGlobals->pProps->prop_DATA_COMPRESSION_ENABLED.empty())
		{
			(_wcsicmp(pGlobals->pProps->prop_DATA_COMPRESSION_ENABLED.c_str(), L"true") == 0) ? param_DATA_COMPRESSION_ENABLED = true : param_DATA_COMPRESSION_ENABLED = false;
		}
		if (!pGlobals->pProps->prop_DATA_GAP_ENABLED.empty())
		{
			(_wcsicmp(pGlobals->pProps->prop_DATA_GAP_ENABLED.c_str(), L"true") == 0) ? param_DATA_GAP_ENABLED = true : param_DATA_GAP_ENABLED = false;
		}

		if (!pGlobals->pProps->prop_TREND_UPPER_GRADIENT.empty())
		{
			param_TREND_UPPER_GRADIENT = (double)_wtof(pGlobals->pProps->prop_TREND_UPPER_GRADIENT.c_str());
		}
		if (!pGlobals->pProps->prop_TREND_LOWER_GRADIENT.empty())
		{
			param_TREND_LOWER_GRADIENT = (double)_wtof(pGlobals->pProps->prop_TREND_LOWER_GRADIENT.c_str());
		}
		if (!pGlobals->pProps->prop_CHANNEL_COMPRESS_RANGE_LIMIT.empty())
		{
			param_CHANNEL_COMPRESS_RANGE_LIMIT = (double)_wtof(pGlobals->pProps->prop_CHANNEL_COMPRESS_RANGE_LIMIT.c_str());
		}
		if (!pGlobals->pProps->prop_CHANNEL_GAP_RANGE_LIMIT.empty())
		{
			param_CHANNEL_GAP_RANGE_LIMIT = (double)_wtof(pGlobals->pProps->prop_CHANNEL_GAP_RANGE_LIMIT.c_str());
		}
		if (!pGlobals->pProps->prop_BASIS_POINT_ADJ.empty())
		{
			param_BASIS_POINT_ADJ = (double)_wtof(pGlobals->pProps->prop_BASIS_POINT_ADJ.c_str());
		}


		pNELib = new NumnericX();

	}
	catch (...)
	{
		// Failed during context creation - delete residual
		if (pNELib != 0)
			delete pNELib;
		rc = ATS_C_FAIL;
	}


	return rc;
}


ATS_CODE AnalyticX::establishTrend(DataAggregateContainer*)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	return rc;
}


ATS_CODE AnalyticX::followTrend(DataAggregateContainer*)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	return rc;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//	CLASS - Linear Trend Analysis
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
LTLS_Analysis::LTLS_Analysis(void* Args) : AnalyticX (Args)
{
	ATS_CODE rc;
	rc = initialize(Args);
}

LTLS_Analysis::~LTLS_Analysis()
{
}


ATS_CODE LTLS_Analysis::initialize(void* Args)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	return rc;
}

///////////////////////////////////////////////////////////////////////////////
//	Establish Linear Trend
//	- Called from trendAnalysisHandler "once processing window size is met"
//	- Performs core trend analysis & makes assessment
//	  based on SuperTick data window
//	- Data Aggregate contains all required DATA calculations
//
//	- The last entry in the data aggregate is the test value
//		- The test value may be out of range or may be a gap up or down.
//		- The slope and y-intercept estimate calculations do not include the 
//		  test data point.
//
//
//	Analytic Engine Return Codes:
//	- ATS_C_AE_NOTREND			No trend established
//	- ATS_C_SE_TRENDING			A trend has been established or continues
//	- ATS_C_AE_TRENDCHANGE		Established trend has been broken
//	- ATS_C_AE_DATA_COMPRESS	Data element eligible for compression
//	- ATS_C_AE_GAP				Established trend has gapped positive
///////////////////////////////////////////////////////////////////////////////
ATS_CODE LTLS_Analysis::processTrend(DataAggregateContainer* dac)
{
	ATS_CODE rc = ATS_C_SUCCESS;
	double	expectedMidpoint = 0.0;
	double	expectedUpperBound = 0.0;
	double	expectedLowerBound = 0.0;
	double	expectedUpperBoundLimit = 0.0;
	double	expectedLowerBoundLimit = 0.0;

	double	newTradeAmount=0.0;
	double	newLastTickValueInSet = 0.0;
	double	gapAmt = 0.0;
	double	range_difference = 0.0;

	DataAggregateXYValue aggrValue;
	unsigned int	trendState;


	/////////////////////////////////////////////////////////////////
	//	Perform all trend calculations for each individual SuperTIck
	//	- Note cascading dependencies
	/////////////////////////////////////////////////////////////////
	try
	{
		///////////////////////////////////////////////////
		//	Prime container
		///////////////////////////////////////////////////
		if (dac->recycledDAContainer == true)
		{
			dac->currUpperBound = dac->Y_DataValues.yValueArray[0].sTickValue;
			dac->currLowerBound = dac->Y_DataValues.yValueArray[0].sTickValue;
			dac->currMidpoint = dac->Y_DataValues.yValueArray[0].sTickValue;
			dac->recycledDAContainer = false;
		}

		trendState = dac->state;	//	Current trend processing state

		///////////////////////////////////////////////////
		//	Save the newest Y-Value amount for later 
		//	restoration and clear it from array so it 
		//	doesn't affect trend calculations
		///////////////////////////////////////////////////
		newTradeAmount = dac->Y_DataValues.yValueArray[dac->Y_DataValues.index - 1].sTickValue;
		newLastTickValueInSet = dac->Y_DataValues.yValueArray[dac->Y_DataValues.index - 1].lastTickValueInSet;
		dac->Y_DataValues.pop();

		///////////////////////////////////////////////////
		//	Perform prerequisite calculations
		///////////////////////////////////////////////////
		pNELib->sumArrayVars(dac->Y_DataValues, dac->sumY);							//	Sum Y
		pNELib->calcAvgHighLowDiff(dac->Y_DataValues, dac->ahld);					//	AvgHighLowDiff
		//pNELib->calcBarValues(dac->Y_DataValues, dac->X_DataValues, dac->ahld);		//	BarValues X -- old
		pNELib->calcBarValues(dac->Y_DataValues, dac->X_DataValues);				//	BarValues X
		pNELib->sumArrayVars(dac->X_DataValues, dac->sumX);							//	Sum X
		pNELib->calcMean(dac->Y_DataValues, dac->a);								//	Mean Y

		//pNELib->calcDeltaValues(dac->Y_DataValues, dac->Y_DeltaValues);				//	ABS Y Deltas
		//pNELib->calcMean(dac->Y_DeltaValues, dac->aD);								//	Mean Y Deltas
		//pNELib->calcStdDevS(dac->Y_DeltaValues, dac->aD, dac->sD);					//	Stddev.s Y_Deltas

		///////////////////////////////////////////////////
		//	Perform slope and y-intercept calculations
		///////////////////////////////////////////////////
		pNELib->calcTrend(dac->X_DataValues, dac->Y_DataValues, dac->sumX, dac->sumY, dac->m);	//	calculate slope --> m
		(dac->m >= 0) ? dac->bias = 1 : dac->bias = 0;
		pNELib->calcY_Intercept(dac->Y_DataValues, dac->m, dac->sumX, dac->sumY, dac->b);		//	calculate Y_Intercept --> b


		///////////////////////////////////////////////////
		//	Calculate expected (new) Y-Value mid-point
		//	along with new upper and lower bounds 
		//	and range limits
		///////////////////////////////////////////////////
		pNELib->calcMidPoint(dac->X_DataValues, dac->m, dac->b, dac->ahld, expectedMidpoint);
		range_difference = (expectedMidpoint * param_BASIS_POINT_ADJ) / 2.0;
		//	Upper and lower channel boundaries are equidistant from the midpoint
		expectedUpperBound = expectedMidpoint + range_difference;
		expectedLowerBound = expectedMidpoint - range_difference;
		//	Upper and lower boundary "limits" may differ in terms of distance from the midpoint
		if (dac->bias == 1)
		{
			expectedUpperBoundLimit = expectedMidpoint + (range_difference * param_CHANNEL_GAP_RANGE_LIMIT);
			expectedLowerBoundLimit = expectedMidpoint - (range_difference * param_CHANNEL_COMPRESS_RANGE_LIMIT);
		}
		else
		{
			expectedUpperBoundLimit = expectedMidpoint + (range_difference * param_CHANNEL_COMPRESS_RANGE_LIMIT);
			expectedLowerBoundLimit = expectedMidpoint - (range_difference * param_CHANNEL_GAP_RANGE_LIMIT);
		}


		///////////////////////////////////////////////////
		//	ASSESS trend calculations - BEGIN
		///////////////////////////////////////////////////

		//	- The new amount (last item in arr[]) is tested against the 
		//	  calculated expected values
		//	- Perform gradient test
		//	- Check newest amount against ranges for COMPRESSION, GAP, BREAK or KEEPER
		if (trendState == TREND_PATTERN_STATE::ESTABLISH)
		{
			if (fabs(dac->m) < param_TREND_UPPER_GRADIENT)
			{
				//	Trend gradient has not met (or dropped below) required limit while assessing ESTABLISH state
				rc = ATS_C_AE_NOTREND;
				goto processTrend_RestoreAndExit;
			}
		}
		else
		{
			if (fabs(dac->m) < param_TREND_LOWER_GRADIENT)
			{
				//	Trend gradient has dropped below acceptable limit while in FOLLOW state
				rc = ATS_C_AE_TRENDCHANGE;
				goto processTrend_RestoreAndExit;
			}
		}
		if (dac->bias == 1)		//	POSITIVE slope boundary checks
		{
			if (newTradeAmount <= expectedUpperBoundLimit && newTradeAmount >= expectedLowerBoundLimit)
			{
				//	KEEP
				rc = ATS_C_AE_TRENDING;;
			}
			else if (newTradeAmount <= expectedLowerBoundLimit && newTradeAmount >= dac->currLowerBound)
			{
				//	COMPRESSION
				if (param_DATA_COMPRESSION_ENABLED == true)
					rc = ATS_C_AE_DATA_COMPRESS;
				else
					rc = ATS_C_AE_TRENDING;
			}
			else if (newTradeAmount >= expectedUpperBoundLimit)
			{
				//	GAP
				if (param_DATA_GAP_ENABLED == true)
				{
					gapAmt = newTradeAmount - expectedUpperBoundLimit;
					rc = adjustTrendYDataValues(dac, gapAmt);
					if (rc == ATS_C_SUCCESS)
					{
						if (dac->currMidpoint != 0)
						{
							//	Adjust boundary values used for calculations
							expectedUpperBound = dac->currUpperBound;	// +gapAmt;
							expectedLowerBound = dac->currLowerBound;	// +gapAmt;
							expectedMidpoint = dac->currMidpoint;	// +gapAmt;
						}
						rc = ATS_C_AE_DATA_GAP;
					}
					else
					{
						//	PROCESSING ERROR
						rc = ATS_C_AE_NOTREND;
					}
				}
				else
				{
					rc = ATS_C_AE_TRENDING;
				}
			}
			else if (newTradeAmount <= dac->currLowerBound)
			{
				//	LIMITOUT
				rc = ATS_C_AE_TRENDLIMITOUT;
			}
			else
			{
				//	PROCESSING ERROR
				rc = ATS_C_AE_TRENDERROR;
			}
		}
		else if (dac->bias == 0)		//	NEGATIVE slope boundary checks
		{
			if (newTradeAmount <= expectedUpperBoundLimit && newTradeAmount >= expectedLowerBoundLimit)
			{
				//	KEEP
				rc = ATS_C_AE_TRENDING;;
			}
			else if (newTradeAmount <= dac->currUpperBound && newTradeAmount >= expectedUpperBoundLimit)
			{
				//	COMPRESSION
				if (param_DATA_COMPRESSION_ENABLED == true)
					rc = ATS_C_AE_DATA_COMPRESS;
				else
					rc = ATS_C_AE_TRENDING;
			}
			else if (newTradeAmount <= expectedLowerBoundLimit)
			{
				//	GAP
				if (param_DATA_GAP_ENABLED == true)
				{
					gapAmt = expectedLowerBoundLimit - newTradeAmount;
					rc = adjustTrendYDataValues(dac, gapAmt);
					aggrValue.lastTickValueInSet = newTradeAmount;	//	 Set values equal due to gap adjustment
					if (rc == ATS_C_SUCCESS)
					{
						if (dac->currMidpoint != 0)
						{
							//	Adjust boundary values used for calculations
							expectedUpperBound = dac->currUpperBound;	// +gapAmt;
							expectedLowerBound = dac->currLowerBound;	// +gapAmt;
							expectedMidpoint = dac->currMidpoint;	// +gapAmt;
						}
						rc = ATS_C_AE_DATA_GAP;
					}
					else
					{
						//	PROCESSING ERROR
						rc = ATS_C_AE_NOTREND;
					}
				}
				else
				{
					rc = ATS_C_AE_TRENDING;
				}
			}
			else if (newTradeAmount >= dac->currUpperBound)
			{
				//	LIMITOUT
				rc = ATS_C_AE_TRENDLIMITOUT;
			}
			else
			{
				//	PROCESSING ERROR
				rc = ATS_C_AE_TRENDERROR;
			}
		}
		else
		{
			//	PROCESSING ERROR
			rc = ATS_C_AE_TRENDERROR;

		}
		///////////////////////////////////////////////////
		//	ASSESS trend calculations - END
		///////////////////////////////////////////////////


		//	Set expected mid-point and boundary values as current
		if (rc == ATS_C_AE_TRENDING)
		{
			dac->currUpperBound = expectedUpperBound;
			dac->currLowerBound = expectedLowerBound;
			dac->currMidpoint = expectedMidpoint;
		}

processTrend_RestoreAndExit:
		//	Re-insert saved test amount
		aggrValue.sTickValue = newTradeAmount;
		aggrValue.lastTickValueInSet = newLastTickValueInSet;
		dac->Y_DataValues.pushBack(aggrValue);

	}
	catch (...)
	{
		//	Processing issue - clear trend
		rc = ATS_C_AE_TRENDERROR;
	}

	return rc;
}


///////////////////////////////////////////////////////////////////////////////
//	Adjust Y Value Data - Linear Trend
//	- Called from trendAnalysisHandler
//	- Adjust Y data values to maintain established trend when slot data 
//	  Gaps in trend direction
//
//	Analytic Engine Return Codes:
//	- ATS_C_SUCCESS				Completed Successfully
///////////////////////////////////////////////////////////////////////////////
ATS_CODE LTLS_Analysis::adjustTrendYDataValues(DataAggregateContainer* dac, double& gapAmt)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	//	Iterate through pDAContainer and Y-Data values
	int elemCount = dac->Y_DataValues.getCount();
	double	newAmt = dac->Y_DataValues.yValueArray[dac->Y_DataValues.index - 1].sTickValue;
	double	oldAmt = dac->Y_DataValues.yValueArray[dac->Y_DataValues.index - 2].sTickValue;
	double	gapDiffAmt = 0.0;

	if (elemCount == 0)
	{
		return ATS_C_RANGE;
	}

	//	Calculate variation
	gapDiffAmt = gapAmt;	// fabs(newAmt - oldAmt);

	for (int i = 0; i < elemCount; i++)
	{
		if (dac->bias == 1)
		{
			dac->Y_DataValues.yValueArray[i].sTickValue += gapDiffAmt;
		}
		else
		{
			dac->Y_DataValues.yValueArray[i].sTickValue -= gapDiffAmt;
		}
	}

	//	Update container current bands and midpoint before exiting loop
	if (dac->bias == 1)
	{
		dac->currUpperBound += gapDiffAmt;
		dac->currMidpoint += gapDiffAmt;
		dac->currLowerBound += gapDiffAmt;
	}
	else
	{
		dac->currUpperBound -= gapDiffAmt;
		dac->currMidpoint -= gapDiffAmt;
		dac->currLowerBound -= gapDiffAmt;

	}

	return rc;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//	CLASS - Exponential Trend Analysis
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ETLS_Analysis::ETLS_Analysis()
{
}

ETLS_Analysis::~ETLS_Analysis()
{
}

ATS_CODE ETLS_Analysis::initialize(void)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	return rc;
}

///////////////////////////////////////////////////////////
//	Establish Exponential Trend
//	- Called from trendAnalysisHandler
//	- Performs core trend analysis & makes assessment
//	  based on ST window
//	- Data Aggregate contains all required DATA for each 
//	  specific aggregate (symbol) defined by its handler
///////////////////////////////////////////////////////////
ATS_CODE ETLS_Analysis::establishTrend(DataAggregateContainer* dac)
{
	ATS_CODE rc = ATS_C_SUCCESS;

	return rc;
}





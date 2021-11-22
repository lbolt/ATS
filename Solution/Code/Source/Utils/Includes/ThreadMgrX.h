#pragma once

#include "../Includes/CommonX.h"
#include "../Includes/PropsX.h"




	struct DataServiceUtilityMgrThreads
	{
	public:
		int i;

		static unsigned __stdcall UtilityManager(void*);

	};

	struct DataManagerDBMgrThreads
	{
	public:
		int i;

		static unsigned __stdcall DBManager(void*);

	};

	struct DataAnalyticsThreads
	{
	public:
		int i;

		static unsigned __stdcall AnalyticsManager(void*);
		static unsigned __stdcall AnalyticsWorker(void*);
	};


	struct DataConnectorThreads
	{
	public:
		int i;

		static unsigned __stdcall ConnectorManager(void*);

		// TODO: Add callback routine for data provider


	};

	struct DataFileConnectorThreads
	{
	public:
		int i;

		static unsigned __stdcall DataFileConnectorManager(void*);

		// TODO: Add callback routine for data provider


	};


	struct DataCollectorThreads
	{
	public:
		int i;

		static unsigned __stdcall CollectorManager(void*);
		static unsigned __stdcall CollectorWorker(void*);

	};



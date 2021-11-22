#pragma once

#include <stdint.h>


namespace ATS_Defines
{


	//	File Descriptions and Layouts - Industry Specific
	static const int32_t ATS_StockSymbolMaxLength = 14;
	static const int32_t ATS_IssueName = 255;
	static const int32_t ATS_ListingExchange = 1;
	static const int32_t ATS_lastExchangeLen = 2;
	static const int32_t ATS_ExchangeID = 20;

	//	Industry File Descriptions and Layouts - ATS Specific
	static const int32_t ATS_CompanyNameDescriptorMaxLength = 64;




	static const int32_t ATS_FundamentalNameMaxLength = 100;
	static const int32_t ATS_MarketMoversMaxRecords = 15;
	static const int32_t ATS_SectorNameMaxLength = 100;
	static const int32_t ATS_IndustryNameMaxLength = 100;
	static const int32_t ATS_TradeConditionsCount = 4;





}
/////////////////////////////////////////////////////////////////////////////////////
//
//
//
#include "stdafx.h"
#include "../Includes/PropsX.h"
#include "XmlLite.h"
#include "Shlwapi.h"


//#pragma comment (lib, "Shlwapi.lib")

//#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
//#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
//#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)




//////////////////////////////////////////////////////////////////////////////////
//	States
//////////////////////////////////////////////////////////////////////////////////
//	DS_States
DS_States::DS_States()
{

}

DS_States::~DS_States()
{

}

int DS_States::initialize(DS_Globals* pGlobs)
{
	return S_OK;
}




//////////////////////////////////////////////////////////////////////////////////
//	Properties
//////////////////////////////////////////////////////////////////////////////////
//	DS_Props
DS_Props::DS_Props()
{

}

DS_Props::~DS_Props()
{

}

int DS_Props::initialize(DS_Globals* pGlobs)
{
	DS_Globals* pGlobals = pGlobs;
	wstring& configFilePath = (wstring&)pGlobals->configFilePath;
	IStream *pFileStream = NULL;
	IXmlReader *pReader = NULL;
	XmlNodeType nodeType;
	const WCHAR* pwszPrefix;
	const WCHAR* pwszLocalName;
	const WCHAR* pwszValue;
	const WCHAR * pwszNamespaceUri;
	UINT cwchPrefix;

	HRESULT hr = S_OK;


	// Read configuration file and setup static properties and thresholds
	//Open read-only input stream
	if (FAILED(hr = SHCreateStreamOnFile((LPCWSTR)configFilePath.c_str(), STGM_READ, &pFileStream)))
	{
		wprintf(L"Error creating file reader, error is %08.8lx", hr);
		HR(hr);
	}

	if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)))
	{
		wprintf(L"Error creating xml reader, error is %08.8lx", hr);
		HR(hr);
	}

	if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
	{
		wprintf(L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
		HR(hr);
	}

	if (FAILED(hr = pReader->SetInput(pFileStream)))
	{
		wprintf(L"Error setting input for reader, error is %08.8lx", hr);
		HR(hr);
	}
	// Iterate through the XML stream and apply values to configuration parameters
	while (S_OK == (hr = pReader->Read(&nodeType)))
	{
		switch (nodeType)
		{
		case XmlNodeType_XmlDeclaration:
			wprintf(L"XmlDeclaration\n");
			break;
		case XmlNodeType_Element:
			if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
			{
				wprintf(L"Error getting prefix, error is %08.8lx", hr);
				HR(hr);
			}
			if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
			{
				wprintf(L"Error getting local name, error is %08.8lx", hr);
				HR(hr);
			}
			if (cwchPrefix > 0)
				wprintf(L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
			else
				wprintf(L"Element: %s\n", pwszLocalName);

			// Process existing attributes on this element
			if (!pReader->IsEmptyElement())
			{
				// if configuration then processing configuration section; set configuration = 'true'
				if (_wcsicmp(pwszLocalName, L"Configuration") == 0)
				{
					// Set configuration data acquired
					;
				}
			}
			else
			{
				pReader->GetNamespaceUri(&pwszNamespaceUri, NULL);
				///////////////////////////////////////////////////////////////////////////
				//	LOGGING attributes
				///////////////////////////////////////////////////////////////////////////
				if (_wcsicmp(pwszLocalName, L"Logging") == 0)
				{
					if (pReader->MoveToAttributeByName(L"enabled", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_logging_enabled.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"level", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_logging_level.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"level_settings", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Parse-out settings
							wstring str = pwszValue;
							transform(str.begin(), str.end(), str.begin(), ::toupper);
							if (str.find(L"ERROR") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_ERROR;
							if (str.find(L"WARN") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_WARN;
							if (str.find(L"INFO") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_INFO;
							if (str.find(L"TRACE") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_TRACE;
							if (str.find(L"DEBUG") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_DEBUG;
							if (str.find(L"DEBUG_2") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_DEBUG_2;
							if (str.find(L"FUNC_ENTRY") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_FUNC_ENTRY;
							if (str.find(L"TRADE") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_TRADE;
							if (str.find(L"SPEAK") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_SPEAK;
							if (str.find(L"STATS") != std::string::npos)
								pGlobals->pProps->prop_log_level |= DS_LOG_LEVEL::DSL_STATS;
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"rollover", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_logging_rollover.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
				}
				///////////////////////////////////////////////////////////////////////////
				//	SNAPSHOT Data attributes
				///////////////////////////////////////////////////////////////////////////
				else if (_wcsicmp(pwszLocalName, L"Snapshot") == 0)
				{
					if (pReader->MoveToAttributeByName(L"enabled", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_snapshot_enabled.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"level", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_snapshot_level.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"rollover", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_snapshot_rollover.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
				}
				///////////////////////////////////////////////////////////////////////////
				//	Worker Thread Management configuration attributes
				///////////////////////////////////////////////////////////////////////////
				else if (_wcsicmp(pwszLocalName, L"ThreadManagement") == 0)
				{
					if (pReader->MoveToAttributeByName(L"MAX_DCWORKER", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_dc_worker_max.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"MAX_DAWORKER", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_da_worker_max.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
				}
				///////////////////////////////////////////////////////////////////////////
				//	AT Provider (Connector) Manager configuration attributes
				///////////////////////////////////////////////////////////////////////////
				else if (_wcsicmp(pwszLocalName, L"AT_Provider") == 0)
				{
					if (pReader->MoveToAttributeByName(L"Enabled", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATenabled.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"DisableRDQInsertion", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATDisableRDQInsertion.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"UserID", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATuserid.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"Credential", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATpassword.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"GUID_KEY", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATguid_key.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"PrimaryServer", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATprimary_server.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"SecondaryServer", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATsecondary_server.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"ServerPort", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATserver_port.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"UseInternalQueue", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATuse_internal_queue.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"CallbackCycleDelay", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATCallbackCycleDelay.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"TrackStatsForSymbol", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ATTrackStatsForSymbol.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
				}
				///////////////////////////////////////////////////////////////////////////
				//	DataFile Provider (Connector) Manager configuration attributes
				///////////////////////////////////////////////////////////////////////////
				else if (_wcsicmp(pwszLocalName, L"DataFile_Provider") == 0)
				{
					if (pReader->MoveToAttributeByName(L"Enabled", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_DataFile_enabled.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"DisableRDQInsertion", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_DataFile_DisableRDQInsertion.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"SnapshotFileName", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_DataFile_snapshot_name.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"InsertTimeStamp", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_DataFile_insert_timestamp.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"Frequency", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_DataFile_frequency.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"TradeRecordsPerCycle", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_DataFile_trade_records_per_cycle.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
				}
				///////////////////////////////////////////////////////////////////////////
				//	DATABASE connection configuration attributes
				///////////////////////////////////////////////////////////////////////////
				else if (_wcsicmp(pwszLocalName, L"DBManagement") == 0)
				{
					if (pReader->MoveToAttributeByName(L"User", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_dbUser.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"Credential", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_dbCred.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"Server", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_dbServer.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"Database", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_dbName.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"Driver", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_dbDriver.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
				}
				///////////////////////////////////////////////////////////////////////////
				//	Analytics configuration attributes
				///////////////////////////////////////////////////////////////////////////
				else if (_wcsicmp(pwszLocalName, L"AT_Analytics_Configuration") == 0)
				{
					/////////////////////////
					//	StructX Properties
					/////////////////////////
					if (pReader->MoveToAttributeByName(L"SYMBOL_MAXSIZE", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_SYMBOL_MAXSIZE.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"DATA_COMPRESSION_ENABLED", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_DATA_COMPRESSION_ENABLED.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"DATA_GAP_ENABLED", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_DATA_GAP_ENABLED.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"OUTLIER_COLLECTION_SIZE", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_OUTLIER_COLLECTION_SIZE.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"MAX_OUTLIER_DELTA", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_MAX_OUTLIER_DELTA.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"SUPERTICK_SIZE", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_SUPERTICK_SIZE.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"MIN_SLOTS_FOR_TREND_CALCS", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_MIN_SLOTS_FOR_TREND_CALCS.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"NUM_SLOTS_TO_ESTABLISH_TREND", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_NUM_SLOTS_TO_ESTABLISH_TREND.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"MAX_SLOTS_ON_FOLLOW", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_MAX_SLOTS_ON_FOLLOW.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"ACTIVE_DS_TRIM_SIZE", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_ACTIVE_DS_TRIM_SIZE.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					////////////////////////////
					//	AnalyticX Properties
					////////////////////////////
					if (pReader->MoveToAttributeByName(L"TREND_UPPER_GRADIENT", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_TREND_UPPER_GRADIENT.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"TREND_LOWER_GRADIENT", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_TREND_LOWER_GRADIENT.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"CHANNEL_COMPRESS_RANGE_LIMIT", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_CHANNEL_COMPRESS_RANGE_LIMIT.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"CHANNEL_GAP_RANGE_LIMIT", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_CHANNEL_GAP_RANGE_LIMIT.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					if (pReader->MoveToAttributeByName(L"BASIS_POINT_ADJ", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_BASIS_POINT_ADJ.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
					/////////////////////////
					//	NumericX Properties
					/////////////////////////
					if (pReader->MoveToAttributeByName(L"TREND_AGGR_SIZE_LIMIT", pwszNamespaceUri) == S_OK)
					{
						if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
						{
							wprintf(L"Error getting value, error is %08.8lx", hr);
							HR(hr);
						}
						else {
							// Set the value
							pGlobals->pProps->prop_TREND_AGGR_SIZE_LIMIT.assign(pwszValue);
							wprintf(L"Text: >%s<\n", pwszValue);
						}
					}
				}
			}
			break;
		case XmlNodeType_EndElement:
			if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
			{
				wprintf(L"Error getting prefix, error is %08.8lx", hr);
				HR(hr);
			}
			if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
			{
				wprintf(L"Error getting local name, error is %08.8lx", hr);
				HR(hr);
			}
			if (cwchPrefix > 0)
				wprintf(L"End Element: %s:%s\n", pwszPrefix, pwszLocalName);
			else
				wprintf(L"End Element: %s\n", pwszLocalName);
			break;
		case XmlNodeType_Text:
		case XmlNodeType_Whitespace:
			if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
			{
				wprintf(L"Error getting value, error is %08.8lx", hr);
				HR(hr);
			}
			wprintf(L"Text: >%s<\n", pwszValue);
			break;
		case XmlNodeType_CDATA:
			if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
			{
				wprintf(L"Error getting value, error is %08.8lx", hr);
				HR(hr);
			}
			wprintf(L"CDATA: %s\n", pwszValue);
			break;
		case XmlNodeType_ProcessingInstruction:
			if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
			{
				wprintf(L"Error getting name, error is %08.8lx", hr);
				HR(hr);
			}
			if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
			{
				wprintf(L"Error getting value, error is %08.8lx", hr);
				HR(hr);
			}
			wprintf(L"Processing Instruction name:%s value:%s\n", pwszLocalName, pwszValue);
			break;
		case XmlNodeType_Comment:
			if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
			{
				wprintf(L"Error getting value, error is %08.8lx", hr);
				HR(hr);
			}
			wprintf(L"Comment: %s\n", pwszValue);
			break;
		case XmlNodeType_DocumentType:
			wprintf(L"DOCTYPE is not printed\n");
			break;
		}
	}


CleanUp:
	SAFE_RELEASE(pFileStream);
	SAFE_RELEASE(pReader);
	return hr;

	//return 0;
}


//////////////////////////////////////////////////////////////////////////////////
//	Globals
//////////////////////////////////////////////////////////////////////////////////
//	DS_Globals
DS_Globals::DS_Globals()
{

	//	Initialize the object
	int rc = initialize();
}

DS_Globals::~DS_Globals()
{

}

ATS_CODE DS_Globals::initialize()
{
	///////////////////////////////////////////////////////////////////////////
	//	Acquire the service executable base path
	///////////////////////////////////////////////////////////////////////////
	getModuleBasePathW(serviceName);
	//	Set logfile location
	logFilePath.append(moduleBasePath);
	logFilePath.append(L"log\\");
	logFilePath.append(serviceName);
	logFilePath.append(L".log");
	//	Set snapshot file location
	snapshotFilePath.append(moduleBasePath);
	snapshotFilePath.append(L"log\\");
	snapshotFilePath.append(L"snapshot");
	snapshotFilePath.append(L".dat");
	//	Set configuration file location
	configFilePath.append(moduleBasePath);
	configFilePath.append(L"config\\");
	configFilePath.append(serviceName);
	configFilePath.append(L".config");

	//	Initialize worker thread index blocks and counts
	memset(gTAddr, 0, sizeof gTAddr);
	memset(gTAddr, 0, sizeof gTIDs);
	memset(gDCTAddr, 0, sizeof gDCTAddr);
	memset(gDCTIDs, 0, sizeof gDCTIDs);
	memset(gDCTState, 0, sizeof gDCTState);
	memset(gDATAddr, 0, sizeof gDATAddr);
	memset(gDATIDs, 0, sizeof gDATIDs);
	memset(gDATState, 0, sizeof gDATState);

	DC_THRD_COUNT = 0;
	DA_THRD_COUNT = 0;

	//	Load properties from configuration file
	pProps = new DS_Props();
	pProps->initialize((DS_Globals*)this);

	//	Setup subsystem state object
	pStates = new DS_States();
	pStates->initialize((DS_Globals*)this);


	return ATS_OK;
}

ATS_CODE DS_Globals::getModuleBasePathW(const wchar_t* filename)
{

	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	DWORD processID;
	wstring msg;

	processID = GetCurrentProcessId();
	msg = L"Current process ID: ";
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);
	if (NULL == hProcess)
		return 0;
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			wchar_t szModName[MAX_PATH];
			if (GetModuleFileNameExW(hProcess, hMods[i], szModName,
				sizeof(szModName)))
			{
				int index;
				wstring s1, s2;
				s1.append(filename);
				s1.append(L".exe");
				s2.append(szModName);
				index = (int)s2.rfind(s1);
				moduleBasePath.append(s2, 0, index);
				break;
			}
		}
	}
	CloseHandle(hProcess);
	return ATS_OK;
}


///////////////////////////////////////////////////////////////////////////////
//	Data Collector (DC)
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DS_Globals::getNextDCThreadIndex(int& index)
{
	AutoCriticalSectionLock DCWORKER_sso(DCWORKER_sso);
	if (index >= MAX_DCWORKER)
	{
		return ATS_C_RANGE;
	}
	//	Find the next available slot
	int _index = 0;
	while (index < MAX_DCWORKER)
	{
		if (gDCTAddr[_index] == NULL)
		{
			//	Empty slot
			break;
		}
		else 
		{
			_index++;
		}
	}
	//	Sanity check
	if (_index == MAX_DCWORKER)
	{
		return ATS_C_NONEXIST;
	}
	index = _index;
	return ATS_C_SUCCESS;
}


ATS_CODE DS_Globals::incrDCThreadIndexCount(void)
{
	AutoCriticalSectionLock DCWORKER_sso(DCWORKER_sso);
	DC_THRD_COUNT++;
	return ATS_C_SUCCESS;
}


ATS_CODE DS_Globals::convertDCThreadIDToIndex(DWORD tid, int& index)
{
	AutoCriticalSectionLock DCWORKER_sso(DCWORKER_sso);
	int _index = 0;
	if (DC_THRD_COUNT > MAX_DCWORKER)
	{
		return ATS_C_RANGE;
	}
	//	Find the slot that matches
	while (_index < MAX_DCWORKER)
	{
		if (gDCTIDs[_index] == tid)
		{
			//	Empty slot
			break;
		}
		else
		{
			_index++;
		}
	}
	if (_index == MAX_DCWORKER)
	{
		return ATS_C_NONEXIST;
	}
	index = _index;
	return ATS_C_SUCCESS;
}


ATS_CODE DS_Globals::clearDCThreadIndex(int& index)
{
	AutoCriticalSectionLock DCWORKER_sso(DCWORKER_sso);
	gDCTAddr[index] = (HANDLE)0; 
	gDCTIDs[index] = (DWORD)0;
	gDCTState[index] = ATS_DATASERVICE::IDLE_ST;
	DC_THRD_COUNT--;
	return ATS_C_SUCCESS;
}


bool DS_Globals::workerDCThreadsIndexFull()
{
	AutoCriticalSectionLock DCWORKER_sso(DCWORKER_sso);
	bool rc = false;
	if (DC_THRD_COUNT == MAX_DCWORKER)
	{
		rc = true;
	}
	return rc;
}


///////////////////////////////////////////////////////////////////////////////
//	Data Analytics (DA)
///////////////////////////////////////////////////////////////////////////////
ATS_CODE DS_Globals::getNextDAThreadIndex(int& index)
{
	AutoCriticalSectionLock DAWORKER_sso(DAWORKER_sso);
	if (index >= MAX_DAWORKER)
	{
		return ATS_C_RANGE;
	}
	//	Find the next available slot
	int _index = 0;
	while (index < MAX_DAWORKER)
	{
		if (gDATAddr[_index] == NULL)
		{
			//	Empty slot
			break;
		}
		else
		{
			_index++;
		}
	}
	//	Sanity check
	if (_index == MAX_DAWORKER)
	{
		return ATS_C_NONEXIST;
	}
	index = _index;
	return ATS_C_SUCCESS;
}


ATS_CODE DS_Globals::incrDAThreadIndexCount(void)
{
	AutoCriticalSectionLock DAWORKER_sso(DAWORKER_sso);
	DA_THRD_COUNT++;
	return ATS_C_SUCCESS;
}


ATS_CODE DS_Globals::convertDAThreadIDToIndex(DWORD tid, int& index)
{
	AutoCriticalSectionLock DAWORKER_sso(DAWORKER_sso);
	int _index = 0;
	if (DA_THRD_COUNT > MAX_DAWORKER)
	{
		return ATS_C_RANGE;
	}
	//	Find the slot that matches
	while (_index < MAX_DAWORKER)
	{
		if (gDATIDs[_index] == tid)
		{
			//	Empty slot
			break;
		}
		else
		{
			_index++;
		}
	}
	if (_index == MAX_DAWORKER)
	{
		return ATS_C_NONEXIST;
	}
	index = _index;
	return ATS_C_SUCCESS;
}


ATS_CODE DS_Globals::clearDAThreadIndex(int& index)
{
	AutoCriticalSectionLock DAWORKER_sso(DAWORKER_sso);
	gDATAddr[index] = (HANDLE)0;
	gDATIDs[index] = (DWORD)0;
	gDATState[index] = ATS_DATASERVICE::IDLE_ST;
	DA_THRD_COUNT--;
	return ATS_C_SUCCESS;
}


bool DS_Globals::workerDAThreadsIndexFull()
{
	AutoCriticalSectionLock DAWORKER_sso(DAWORKER_sso);
	bool rc = false;
	if (DA_THRD_COUNT == MAX_DAWORKER)
	{
		rc = true;
	}
	return rc;
}






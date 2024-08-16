#pragma once
// class for calculating k-line

#include <vector>
#include <string>

// k-line data structure
struct KLineDataType
{
	double open_price;   
	double high_price;   
	double low_price;    
	double close_price;
	int volume;          
};

class TickToKlineHelper
{
public:
	// construct k-line from local data and store it locally (assuming no data loss in local data)
	void KLineFromLocalData(const std::string &sFilePath, const std::string &dFilePath); 
	// construct k-line from real-time data
	void KLineFromRealtimeData(CThostFtdcDepthMarketDataField *pDepthMarketData);
public:
	std::vector<double> m_priceVec; // store prices for 1 minute
	std::vector<int> m_volumeVec; // store volumes for 1 minute
	std::vector<KLineDataType> m_KLineDataArray;
};

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "CTP_API/ThostFtdcUserApiStruct.h"
#include "TickToKlineHelper.h"

const int kDataLineNum = 2 * 60; // number of lines required for 1-minute k-line (incomplete data for the last minute is discarded)

void TickToKlineHelper::KLineFromLocalData(const std::string &sFilePath, const std::string &dFilePath) {
	// clear any residual data
	m_priceVec.clear();
	m_volumeVec.clear();
	m_KLineDataArray.clear();

	std::cout << "Starting conversion from tick to k-line..." << std::endl;
	// by default, the tick data table reads 4 fields: contract code, update time, latest price, and volume
	std::ifstream srcInFile;
	std::ofstream dstOutFile;
	srcInFile.open(sFilePath, std::ios::in);
	dstOutFile.open(dFilePath, std::ios::out);
	dstOutFile << "Open price" << ','
		<< "Highest price" << ','
		<< "Lowest price" << ','
		<< "Close price" << ',' 
		<< "Volume" << std::endl;

	// parse the file while calculating k-line data, each 1-minute k-line reads 60 * 2 = 120 lines of data
	std::string lineStr;
	bool isFirstLine = true;
	while (std::getline(srcInFile, lineStr))
	{
		if (isFirstLine)
		{
			// skip the first line header
			isFirstLine = false;
			continue;
		}
		std::istringstream ss(lineStr);
		std::string fieldStr;
		int count = 4;
		while (std::getline(ss, fieldStr, ','))
		{
			count--;
			if (count == 1)
				m_priceVec.push_back(std::atof(fieldStr.c_str()));
			else if (count == 0)
			{
				m_volumeVec.push_back(std::atoi(fieldStr.c_str()));
				break;
			}
		}

		// calculate k-line
		if (m_priceVec.size() == kDataLineNum)
		{
			KLineDataType k_line_data;
			k_line_data.open_price = m_priceVec.front();
			k_line_data.high_price = *std::max_element(m_priceVec.cbegin(), m_priceVec.cend());
			k_line_data.low_price = *std::min_element(m_priceVec.cbegin(), m_priceVec.cend());
			k_line_data.close_price = m_priceVec.back();
			// the actual algorithm for volume is the last volume of the current interval minus the last volume of the previous interval
			k_line_data.volume = m_volumeVec.back() - m_volumeVec.front(); 
			//m_KLineDataArray.push_back(k_line_data); // can be stored in memory here
			
			dstOutFile << k_line_data.open_price << ','
				<< k_line_data.high_price << ','
				<< k_line_data.low_price << ','
				<< k_line_data.close_price << ','
				<< k_line_data.volume << std::endl;

			m_priceVec.clear();
			m_volumeVec.clear();
		}
	}

	srcInFile.close();
	dstOutFile.close();

	std::cout << "K-line generation successful" << std::endl;
}

void TickToKlineHelper::KLineFromRealtimeData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	m_priceVec.push_back(pDepthMarketData->LastPrice);
	m_volumeVec.push_back(pDepthMarketData->Volume);
	if (m_priceVec.size() == kDataLineNum)
	{
		KLineDataType k_line_data;
		k_line_data.open_price = m_priceVec.front();
		k_line_data.high_price = *std::max_element(m_priceVec.cbegin(), m_priceVec.cend());
		k_line_data.low_price = *std::min_element(m_priceVec.cbegin(), m_priceVec.cend());
		k_line_data.close_price = m_priceVec.back();
		// the actual algorithm for volume is the last volume of the current interval minus the last volume of the previous interval
		k_line_data.volume = m_volumeVec.back() - m_volumeVec.front();
		m_KLineDataArray.push_back(k_line_data); // can be stored in memory here

		m_priceVec.clear();
		m_volumeVec.clear();
	}
}

#include <iostream>
#include <fstream>
#include <unordered_map>
#include "CustomMdSpi.h"
#include "TickToKlineHelper.h"

// ---- global parameter declarations ---- //
extern CThostFtdcMdApi *g_pMdUserApi;            // market data pointer
extern char gMdFrontAddr[];                      // simulated market data front address
extern TThostFtdcBrokerIDType gBrokerID;         // simulated broker code
extern TThostFtdcInvestorIDType gInvesterID;     // investor account name
extern TThostFtdcPasswordType gInvesterPassword; // investor password
extern char *g_pInstrumentID[];                  // list of market data contract codes
extern int instrumentNum;                        // number of subscribed market data contracts
extern std::unordered_map<std::string, TickToKlineHelper> g_KlineHash; // k-line storage table

// ---- ctp_api callback functions ---- //
// connection successful response
void CustomMdSpi::OnFrontConnected()
{
	std::cout << "=====network connection established successfully=====" << std::endl;
	// start login
	CThostFtdcReqUserLoginField loginReq;
	memset(&loginReq, 0, sizeof(loginReq));
	strcpy(loginReq.BrokerID, gBrokerID);
	strcpy(loginReq.UserID, gInvesterID);
	strcpy(loginReq.Password, gInvesterPassword);
	static int requestID = 0; // request id
	int rt = g_pMdUserApi->ReqUserLogin(&loginReq, requestID);
	if (!rt)
		std::cout << ">>>>>>login request sent successfully" << std::endl;
	else
		std::cerr << "--->>>login request failed" << std::endl;
}

// disconnected notification
void CustomMdSpi::OnFrontDisconnected(int nReason)
{
	std::cerr << "=====network connection disconnected=====" << std::endl;
	std::cerr << "error code: " << nReason << std::endl;
}

// heartbeat timeout warning
void CustomMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	std::cerr << "=====network heartbeat timeout=====" << std::endl;
	std::cerr << "time since last connection: " << nTimeLapse << std::endl;
}

// login response
void CustomMdSpi::OnRspUserLogin(
	CThostFtdcRspUserLoginField *pRspUserLogin, 
	CThostFtdcRspInfoField *pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====account login successful=====" << std::endl;
		std::cout << "trading day: " << pRspUserLogin->TradingDay << std::endl;
		std::cout << "login time: " << pRspUserLogin->LoginTime << std::endl;
		std::cout << "broker: " << pRspUserLogin->BrokerID << std::endl;
		std::cout << "account name: " << pRspUserLogin->UserID << std::endl;
		// start subscribing to market data
		int rt = g_pMdUserApi->SubscribeMarketData(g_pInstrumentID, instrumentNum);
		if (!rt)
			std::cout << ">>>>>>market data subscription request sent successfully" << std::endl;
		else
			std::cerr << "--->>>market data subscription request failed" << std::endl;
	}
	else
		std::cerr << "returned error--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// logout response
void CustomMdSpi::OnRspUserLogout(
	CThostFtdcUserLogoutField *pUserLogout,
	CThostFtdcRspInfoField *pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====account logout successful=====" << std::endl;
		std::cout << "broker: " << pUserLogout->BrokerID << std::endl;
		std::cout << "account name: " << pUserLogout->UserID << std::endl;
	}
	else
		std::cerr << "returned error--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// error notification
void CustomMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (bResult)
		std::cerr << "returned error--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// market data subscription response
void CustomMdSpi::OnRspSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
	CThostFtdcRspInfoField *pRspInfo, 
	int nRequestID, 
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====market data subscription successful=====" << std::endl;
		std::cout << "instrument code: " << pSpecificInstrument->InstrumentID << std::endl;
		// if saving to a file or database is needed, create the header here, and store different instruments separately
		char filePath[100] = {'\0'};
		sprintf(filePath, "%s_market_data.csv", pSpecificInstrument->InstrumentID);
		std::ofstream outFile;
		outFile.open(filePath, std::ios::out); // create new file
		outFile << "instrument code" << ","
			<< "update time" << ","
			<< "latest price" << ","
			<< "volume" << ","
			<< "bid price 1" << ","
			<< "bid volume 1" << ","
			<< "ask price 1" << ","
			<< "ask volume 1" << ","
			<< "open interest" << ","
			<< "turnover rate"
			<< std::endl;
		outFile.close();
	}
	else
		std::cerr << "returned error--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// market data unsubscription response
void CustomMdSpi::OnRspUnSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, 
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====market data unsubscription successful=====" << std::endl;
		std::cout << "instrument code: " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "returned error--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// quote subscription response
void CustomMdSpi::OnRspSubForQuoteRsp(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====quote subscription successful=====" << std::endl;
		std::cout << "instrument code: " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "returned error--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// quote unsubscription response
void CustomMdSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====quote unsubscription successful=====" << std::endl;
		std::cout << "instrument code: " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "returned error--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// market data notification
void CustomMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	// print market data, only partial fields shown
	std::cout << "=====received depth market data=====" << std::endl;
	std::cout << "trading day: " << pDepthMarketData->TradingDay << std::endl;
	std::cout << "exchange code: " << pDepthMarketData->ExchangeID << std::endl;
	std::cout << "instrument code: " << pDepthMarketData->InstrumentID << std::endl;
	std::cout << "instrument code on exchange: " << pDepthMarketData->ExchangeInstID << std::endl;
	std::cout << "latest price: " << pDepthMarketData->LastPrice << std::endl;
	std::cout << "volume: " << pDepthMarketData->Volume << std::endl;
	// if only market data for a specific instrument is needed, data can be saved to a file or database on a per-tick basis
	char filePath[100] = {'\0'};
	sprintf(filePath, "%s_market_data.csv", pDepthMarketData->InstrumentID);
	std::ofstream outFile;
	outFile.open(filePath, std::ios::app); // append to the file 
	outFile << pDepthMarketData->InstrumentID << "," 
		<< pDepthMarketData->UpdateTime << "." << pDepthMarketData->UpdateMillisec << "," 
		<< pDepthMarketData->LastPrice << "," 
		<< pDepthMarketData->Volume << "," 
		<< pDepthMarketData->BidPrice1 << "," 
		<< pDepthMarketData->BidVolume1 << "," 
		<< pDepthMarketData->AskPrice1 << "," 
		<< pDepthMarketData->AskVolume1 << "," 
		<< pDepthMarketData->OpenInterest << "," 
		<< pDepthMarketData->Turnover << std::endl;
	outFile.close();

	// calculate real-time k-line
	std::string instrumentKey = std::string(pDepthMarketData->InstrumentID);
	if (g_KlineHash.find(instrumentKey) == g_KlineHash.end())
		g_KlineHash[instrumentKey] = TickToKlineHelper();
	g_KlineHash[instrumentKey].KLineFromRealtimeData(pDepthMarketData);

	// unsubscribe from market data
	//int rt = g_pMdUserApi->UnSubscribeMarketData(g_pInstrumentID, instrumentNum);
	//if (!rt)
	//	std::cout << ">>>>>>market data unsubscription request sent successfully" << std::endl;
	//else
	//	std::cerr << "--->>>market data unsubscription request failed" << std::endl;
}

// quote notification
void CustomMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
	// partial quote results
	std::cout << "=====received quote result=====" << std::endl;
	std::cout << "trading day: " << pForQuoteRsp->TradingDay << std::endl;
	std::cout << "exchange code: " << pForQuoteRsp->ExchangeID << std::endl;
	std::cout << "instrument code: " << pForQuoteRsp->InstrumentID << std::endl;
	std::cout << "quote id: " << pForQuoteRsp->ForQuoteSysID << std::endl;
}

#pragma once
#include <vector>
#include "CTP_API/ThostFtdcMdApi.h"

class CustomMdSpi: public CThostFtdcMdSpi
{
	/*
	 * Inherit CThostFtdcMdSpi to implement your own market callback class CustomMdSpi. 
	 * When the system is running, these rewritten functions will be called back by the CTP system API 
	 * to achieve personalized market conditions.
	*/ 
public:
	// When the front connection is successful(before log in), this function will be called back.
	void OnFrontConnected();

	/*
	 * When the front connection is disconnected, this function will be called back.
	 * @param nReason: the reason of disconnection.
	 * 0x1001: network connection is broken.
	 * 0x1002: network read failure.
	 * 0x2001: network write failure.
	 * 0x2002: network read timeout.
	 * 0x2003: network write timeout.
	 * 0x2004: network read error.
	*/
	void OnFrontDisconnected(int nReason);

	/**
	 * When the heart beat warning is triggered, this function will be called back.
	 * @param nTimeLapse: the time interval from the last successful heart beat.
	*/
	void OnHeartBeatWarning(int nTimeLapse);

	// When the login response is received, this function will be called back.
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// When the logout response is received, this function will be called back.
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// When the error response is received, this function will be called back.
	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// When the subscription request is received, this function will be called back.
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// When the unsubscription request is received, this function will be called back.
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// When the subscription request of quotation is received, this function will be called back.
	void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// When the unsubscription request of quotation is received, this function will be called back.
	void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// When the depth market data is received, this function will be called back.
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	// When the quotation of price is received, this function will be called back.
	void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);
};

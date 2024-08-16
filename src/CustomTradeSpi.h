#pragma once
// ---- Derived Trading Class ---- //
#include "CTP_API/ThostFtdcTraderApi.h"

class CustomTradeSpi : public CThostFtdcTraderSpi
{
// ---- ctp_api callback interfaces ---- //
public:
	///Called when the client establishes a communication connection with the trading backend (before logging in).
	void OnFrontConnected();

	///Login request response
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Error response
	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Called when the client disconnects from the trading backend. After this happens, the API will automatically reconnect, and the client does not need to take action.
	void OnFrontDisconnected(int nReason);

	///Heartbeat timeout warning. Called when no message has been received for a long time.
	void OnHeartBeatWarning(int nTimeLapse);

	///Logout request response
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Investor settlement result confirmation response
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Query instrument response
	void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Query trading account response
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Query investor position response
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Order insert request response
	void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Order action request response
	void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Order notification
	void OnRtnOrder(CThostFtdcOrderField *pOrder);

	///Trade notification
	void OnRtnTrade(CThostFtdcTradeField *pTrade);
	
// ---- Custom functions ---- //
public:
	bool loginFlag; // Login success flag
	void reqOrderInsert(
		TThostFtdcInstrumentIDType instrumentID,
		TThostFtdcPriceType price,
		TThostFtdcVolumeType volume,
		TThostFtdcDirectionType direction); // Custom order insert, called externally
private:
	void reqUserLogin(); // Login request
	void reqUserLogout(); // Logout request
	void reqSettlementInfoConfirm(); // Investor settlement result confirmation
	void reqQueryInstrument(); // Query instrument request
	void reqQueryTradingAccount(); // Query trading account request
	void reqQueryInvestorPosition(); // Query investor position request
	void reqOrderInsert(); // Order insert request
	
	void reqOrderAction(CThostFtdcOrderField *pOrder); // Order action request
	bool isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo); // Check if the response contains error information
	bool isMyOrder(CThostFtdcOrderField *pOrder); // Check if the order belongs to the current user
	bool isTradingOrder(CThostFtdcOrderField *pOrder); // Check if the order is currently trading
};

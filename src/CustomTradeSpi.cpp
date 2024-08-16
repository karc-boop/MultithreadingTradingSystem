#include <iostream>
#include <cstring>
#include <time.h>
#include <thread>
#include <chrono>
#include "CustomTradeSpi.h"
#include "TradeStrategy.h"


// ---- Global Parameter Declarations ---- //
extern TThostFtdcBrokerIDType gBrokerID;                      // Simulated broker code
extern TThostFtdcInvestorIDType gInvesterID;                  // Investor account name
extern TThostFtdcPasswordType gInvesterPassword;              // Investor password
extern CThostFtdcTraderApi *g_pTradeUserApi;                  // Trading pointer
extern char gTradeFrontAddr[];                                // Simulated trading front address
extern TThostFtdcInstrumentIDType g_pTradeInstrumentID;       // Traded contract code
extern TThostFtdcDirectionType gTradeDirection;               // Buy/Sell direction
extern TThostFtdcPriceType gLimitPrice;                       // Trading price

// Session Parameters
TThostFtdcFrontIDType	trade_front_id;	// Front ID
TThostFtdcSessionIDType	session_id;	// Session ID
TThostFtdcOrderRefType	order_ref;	// Order reference
time_t lOrderTime;
time_t lOrderOkTime;

void CustomTradeSpi::OnFrontConnected()
{
	std::cout << "=====Network connection established successfully=====" << std::endl;
	// Start login
	reqUserLogin();
}

void CustomTradeSpi::OnRspUserLogin(
	CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::cout << "=====Account login successful=====" << std::endl;
		loginFlag = true;
		std::cout << "Trading day: " << pRspUserLogin->TradingDay << std::endl;
		std::cout << "Login time: " << pRspUserLogin->LoginTime << std::endl;
		std::cout << "Broker: " << pRspUserLogin->BrokerID << std::endl;
		std::cout << "Account name: " << pRspUserLogin->UserID << std::endl;
		// Save session parameters
		trade_front_id = pRspUserLogin->FrontID;
		session_id = pRspUserLogin->SessionID;
		strcpy(order_ref, pRspUserLogin->MaxOrderRef);

		// Investor settlement result confirmation
		reqSettlementInfoConfirm();
	}
}

void CustomTradeSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	isErrorRspInfo(pRspInfo);
}

void CustomTradeSpi::OnFrontDisconnected(int nReason)
{
	std::cerr << "=====Network connection disconnected=====" << std::endl;
	std::cerr << "Error code: " << nReason << std::endl;
}

void CustomTradeSpi::OnHeartBeatWarning(int nTimeLapse)
{
	std::cerr << "=====Network heartbeat timeout=====" << std::endl;
	std::cerr << "Time since last connection: " << nTimeLapse << std::endl;
}

void CustomTradeSpi::OnRspUserLogout(
	CThostFtdcUserLogoutField *pUserLogout,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		loginFlag = false; // Cannot trade after logout
		std::cout << "=====Account logout successful=====" << std::endl;
		std::cout << "Broker: " << pUserLogout->BrokerID << std::endl;
		std::cout << "Account name: " << pUserLogout->UserID << std::endl;
	}
}

void CustomTradeSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::cout << "=====Investor settlement result confirmation successful=====" << std::endl;
		std::cout << "Confirmation date: " << pSettlementInfoConfirm->ConfirmDate << std::endl;
		std::cout << "Confirmation time: " << pSettlementInfoConfirm->ConfirmTime << std::endl;
		// Request to query instrument
		reqQueryInstrument();
	}
}

void CustomTradeSpi::OnRspQryInstrument(
	CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::cout << "=====Query instrument result successful=====" << std::endl;
		std::cout << "Exchange code: " << pInstrument->ExchangeID << std::endl;
		std::cout << "Instrument code: " << pInstrument->InstrumentID << std::endl;
		std::cout << "Instrument code on exchange: " << pInstrument->ExchangeInstID << std::endl;
		std::cout << "Strike price: " << pInstrument->StrikePrice << std::endl;
		std::cout << "Expiration date: " << pInstrument->EndDelivDate << std::endl;
		std::cout << "Current trading status: " << pInstrument->IsTrading << std::endl;
		// Request to query investor trading account
		reqQueryTradingAccount();
	}
}

void CustomTradeSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::cout << "=====Query investor trading account successful=====" << std::endl;
		std::cout << "Investor account: " << pTradingAccount->AccountID << std::endl;
		std::cout << "Available funds: " << pTradingAccount->Available << std::endl;
		std::cout << "Withdrawable funds: " << pTradingAccount->WithdrawQuota << std::endl;
		std::cout << "Current margin: " << pTradingAccount->CurrMargin << std::endl;
		std::cout << "Close profit: " << pTradingAccount->CloseProfit << std::endl;
		// Request to query investor position
		reqQueryInvestorPosition();
	}
}

void CustomTradeSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::cout << "=====Query investor position successful=====" << std::endl;
		if (pInvestorPosition)
		{
			std::cout << "Instrument code: " << pInvestorPosition->InstrumentID << std::endl;
			std::cout << "Opening price: " << pInvestorPosition->OpenAmount << std::endl;
			std::cout << "Opening volume: " << pInvestorPosition->OpenVolume << std::endl;
			std::cout << "Opening direction: " << pInvestorPosition->PosiDirection << std::endl;
			std::cout << "Margin used: " << pInvestorPosition->UseMargin << std::endl;
		}
		else
			std::cout << "----->No position held for this instrument" << std::endl;
		
		// Order insert request (this is an asynchronous interface, executed sequentially here)
		/*if (loginFlag)
			reqOrderInsert();*/
		//if (loginFlag)
		//	reqOrderInsertWithParams(g_pTradeInstrumentID, gLimitPrice, 1, gTradeDirection); // Custom trade

		// Strategy trading
		std::cout << "=====Starting strategy trading=====" << std::endl;
		while (loginFlag)
			StrategyCheckAndTrade(g_pTradeInstrumentID, this);
	}
}

void CustomTradeSpi::OnRspOrderInsert(
	CThostFtdcInputOrderField *pInputOrder, 
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::cout << "=====Order insert successful=====" << std::endl;
		std::cout << "Instrument code: " << pInputOrder->InstrumentID << std::endl;
		std::cout << "Price: " << pInputOrder->LimitPrice << std::endl;
		std::cout << "Volume: " << pInputOrder->VolumeTotalOriginal << std::endl;
		std::cout << "Opening direction: " << pInputOrder->Direction << std::endl;
	}
}

void CustomTradeSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::cout << "=====Order action successful=====" << std::endl;
		std::cout << "Instrument code: " << pInputOrderAction->InstrumentID << std::endl;
		std::cout << "Action flag: " << pInputOrderAction->ActionFlag;
	}
}

void CustomTradeSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	char str[10];
	snprintf(str, sizeof(str), "%d", pOrder->OrderSubmitStatus);
	int orderState = atoi(str) - 48;	// Order status: 0 = Submitted, 3 = Accepted

	std::cout << "=====Order response received=====" << std::endl;

	if (isMyOrder(pOrder))
	{
		if (isTradingOrder(pOrder))
		{
			std::cout << "--->>> Waiting for trade!" << std::endl;
			//reqOrderAction(pOrder); // You can cancel the order here
			//reqUserLogout(); // Logout test
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
			std::cout << "--->>> Order canceled successfully!" << std::endl;
	}
}

void CustomTradeSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	std::cout << "=====Order traded successfully=====" << std::endl;
	std::cout << "Trade time: " << pTrade->TradeTime << std::endl;
	std::cout << "Instrument code: " << pTrade->InstrumentID << std::endl;
	std::cout << "Trade price: " << pTrade->Price << std::endl;
	std::cout << "Trade volume: " << pTrade->Volume << std::endl;
	std::cout << "Opening/closing direction: " << pTrade->Direction << std::endl;
}

bool CustomTradeSpi::isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (bResult)
		std::cerr << "Returned error--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
	return bResult;
}

void CustomTradeSpi::reqUserLogin()
{
	CThostFtdcReqUserLoginField loginReq;
	memset(&loginReq, 0, sizeof(loginReq));
	strcpy(loginReq.BrokerID, gBrokerID);
	strcpy(loginReq.UserID, gInvesterID);
	strcpy(loginReq.Password, gInvesterPassword);
	static int requestID = 0; // Request ID
	int rt = g_pTradeUserApi->ReqUserLogin(&loginReq, requestID);
	if (!rt)
		std::cout << ">>>>>>Login request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Login request failed" << std::endl;
}

void CustomTradeSpi::reqUserLogout()
{
	CThostFtdcUserLogoutField logoutReq;
	memset(&logoutReq, 0, sizeof(logoutReq));
	strcpy(logoutReq.BrokerID, gBrokerID);
	strcpy(logoutReq.UserID, gInvesterID);
	static int requestID = 0; // Request ID
	int rt = g_pTradeUserApi->ReqUserLogout(&logoutReq, requestID);
	if (!rt)
		std::cout << ">>>>>>Logout request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Logout request failed" << std::endl;
}

void CustomTradeSpi::reqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField settlementConfirmReq;
	memset(&settlementConfirmReq, 0, sizeof(settlementConfirmReq));
	strcpy(settlementConfirmReq.BrokerID, gBrokerID);
	strcpy(settlementConfirmReq.InvestorID, gInvesterID);
	static int requestID = 0; // Request ID
	int rt = g_pTradeUserApi->ReqSettlementInfoConfirm(&settlementConfirmReq, requestID);
	if (!rt)
		std::cout << ">>>>>>Investor settlement result confirmation request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Investor settlement result confirmation request failed" << std::endl;
}

void CustomTradeSpi::reqQueryInstrument()
{
	CThostFtdcQryInstrumentField instrumentReq;
	memset(&instrumentReq, 0, sizeof(instrumentReq));
	strcpy(instrumentReq.InstrumentID, g_pTradeInstrumentID);
	static int requestID = 0; // Request ID
	int rt = g_pTradeUserApi->ReqQryInstrument(&instrumentReq, requestID);
	if (!rt)
		std::cout << ">>>>>>Instrument query request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Instrument query request failed" << std::endl;
}

void CustomTradeSpi::reqQueryTradingAccount()
{
	CThostFtdcQryTradingAccountField tradingAccountReq;
	memset(&tradingAccountReq, 0, sizeof(tradingAccountReq));
	strcpy(tradingAccountReq.BrokerID, gBrokerID);
	strcpy(tradingAccountReq.InvestorID, gInvesterID);
	static int requestID = 0; // Request ID
	std::this_thread::sleep_for(std::chrono::milliseconds(700)); // Sometimes a delay is needed for the query to succeed
	int rt = g_pTradeUserApi->ReqQryTradingAccount(&tradingAccountReq, requestID);
	if (!rt)
		std::cout << ">>>>>>Investor trading account query request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Investor trading account query request failed" << std::endl;
}

void CustomTradeSpi::reqQueryInvestorPosition()
{
	CThostFtdcQryInvestorPositionField positionReq;
	memset(&positionReq, 0, sizeof(positionReq));
	strcpy(positionReq.BrokerID, gBrokerID);
	strcpy(positionReq.InvestorID, gInvesterID);
	strcpy(positionReq.InstrumentID, g_pTradeInstrumentID);
	static int requestID = 0; // Request ID
	std::this_thread::sleep_for(std::chrono::milliseconds(700)); // Sometimes a delay is needed for the query to succeed
	int rt = g_pTradeUserApi->ReqQryInvestorPosition(&positionReq, requestID);
	if (!rt)
		std::cout << ">>>>>>Investor position query request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Investor position query request failed" << std::endl;
}

void CustomTradeSpi::reqOrderInsert()
{
	CThostFtdcInputOrderField orderInsertReq;
	memset(&orderInsertReq, 0, sizeof(orderInsertReq));
	///Broker code
	strcpy(orderInsertReq.BrokerID, gBrokerID);
	///Investor code
	strcpy(orderInsertReq.InvestorID, gInvesterID);
	///Instrument code
	strcpy(orderInsertReq.InstrumentID, g_pTradeInstrumentID);
	///Order reference
	strcpy(orderInsertReq.OrderRef, order_ref);
	///Order price type: Limit price
	orderInsertReq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	///Buy/Sell direction:
	orderInsertReq.Direction = gTradeDirection;
	///Combination open/close flag: Open
	orderInsertReq.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///Combination hedge flag
	orderInsertReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///Price
	orderInsertReq.LimitPrice = gLimitPrice;
	///Volume: 1
	orderInsertReq.VolumeTotalOriginal = 1;
	///Validity type: Valid for the day
	orderInsertReq.TimeCondition = THOST_FTDC_TC_GFD;
	///Volume type: Any quantity
	orderInsertReq.VolumeCondition = THOST_FTDC_VC_AV;
	///Minimum volume: 1
	orderInsertReq.MinVolume = 1;
	///Trigger condition: Immediate
	orderInsertReq.ContingentCondition = THOST_FTDC_CC_Immediately;
	///Force close reason: Non-force close
	orderInsertReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///Auto suspend flag: No
	orderInsertReq.IsAutoSuspend = 0;
	///User force close flag: No
	orderInsertReq.UserForceClose = 0;

	static int requestID = 0; // Request ID
	int rt = g_pTradeUserApi->ReqOrderInsert(&orderInsertReq, ++requestID);
	if (!rt)
		std::cout << ">>>>>>Order insert request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Order insert request failed" << std::endl;
}

void CustomTradeSpi::reqOrderInsert(
	TThostFtdcInstrumentIDType instrumentID,
	TThostFtdcPriceType price,
	TThostFtdcVolumeType volume,
	TThostFtdcDirectionType direction)
{
	CThostFtdcInputOrderField orderInsertReq;
	memset(&orderInsertReq, 0, sizeof(orderInsertReq));
	///Broker code
	strcpy(orderInsertReq.BrokerID, gBrokerID);
	///Investor code
	strcpy(orderInsertReq.InvestorID, gInvesterID);
	///Instrument code
	strcpy(orderInsertReq.InstrumentID, instrumentID);
	///Order reference
	strcpy(orderInsertReq.OrderRef, order_ref);
	///Order price type: Limit price
	orderInsertReq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	///Buy/Sell direction:
	orderInsertReq.Direction = direction;
	///Combination open/close flag: Open
	orderInsertReq.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///Combination hedge flag
	orderInsertReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///Price
	orderInsertReq.LimitPrice = price;
	///Volume: 1
	orderInsertReq.VolumeTotalOriginal = volume;
	///Validity type: Valid for the day
	orderInsertReq.TimeCondition = THOST_FTDC_TC_GFD;
	///Volume type: Any quantity
	orderInsertReq.VolumeCondition = THOST_FTDC_VC_AV;
	///Minimum volume: 1
	orderInsertReq.MinVolume = 1;
	///Trigger condition: Immediate
	orderInsertReq.ContingentCondition = THOST_FTDC_CC_Immediately;
	///Force close reason: Non-force close
	orderInsertReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///Auto suspend flag: No
	orderInsertReq.IsAutoSuspend = 0;
	///User force close flag: No
	orderInsertReq.UserForceClose = 0;

	static int requestID = 0; // Request ID
	int rt = g_pTradeUserApi->ReqOrderInsert(&orderInsertReq, ++requestID);
	if (!rt)
		std::cout << ">>>>>>Order insert request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Order insert request failed" << std::endl;
}

void CustomTradeSpi::reqOrderAction(CThostFtdcOrderField *pOrder)
{
	static bool orderActionSentFlag = false; // Whether the order action was sent
	if (orderActionSentFlag)
		return;

	CThostFtdcInputOrderActionField orderActionReq;
	memset(&orderActionReq, 0, sizeof(orderActionReq));
	///Broker code
	strcpy(orderActionReq.BrokerID, pOrder->BrokerID);
	///Investor code
	strcpy(orderActionReq.InvestorID, pOrder->InvestorID);
	///Order reference
	strcpy(orderActionReq.OrderRef, pOrder->OrderRef);
	///Front ID
	orderActionReq.FrontID = trade_front_id;
	///Session ID
	orderActionReq.SessionID = session_id;
	///Action flag
	orderActionReq.ActionFlag = THOST_FTDC_AF_Delete;
	///Instrument code
	strcpy(orderActionReq.InstrumentID, pOrder->InstrumentID);
	static int requestID = 0; // Request ID
	int rt = g_pTradeUserApi->ReqOrderAction(&orderActionReq, ++requestID);
	if (!rt)
		std::cout << ">>>>>>Order action request sent successfully" << std::endl;
	else
		std::cerr << "--->>>Order action request failed" << std::endl;
	orderActionSentFlag = true;
}

bool CustomTradeSpi::isMyOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->FrontID == trade_front_id) &&
		(pOrder->SessionID == session_id) &&
		(strcmp(pOrder->OrderRef, order_ref) == 0));
}

bool CustomTradeSpi::isTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

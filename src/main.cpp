#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include "CustomMdSpi.h"
#include "CustomTradeSpi.h"
#include "TickToKlineHelper.h"

using namespace std;

// link libraries
#pragma comment (lib, "thostmduserapi.lib")
#pragma comment (lib, "thosttraderapi.lib")

// ---- global variables ---- //
// common parameters
TThostFtdcBrokerIDType gBrokerID = "9999";                         // simulated broker code
TThostFtdcInvestorIDType gInvesterID = "";                         // investor account name
TThostFtdcPasswordType gInvesterPassword = "";                     // investor password

// market data parameters
CThostFtdcMdApi *g_pMdUserApi = nullptr;                           // market data pointer
char gMdFrontAddr[] = "tcp://180.168.146.187:10010";               // simulated market data front address
const char *g_pInstrumentID[] = {"TF1706", "zn1705", "cs1801", "CF705"};
int instrumentNum = 4;                                             // number of market data subscriptions
unordered_map<string, TickToKlineHelper> g_KlineHash;              // k-line storage table for different contracts

// trading parameters
CThostFtdcTraderApi *g_pTradeUserApi = nullptr;                    // trading pointer
char gTradeFrontAddr[] = "tcp://180.168.146.187:10001";            // simulated trading front address
TThostFtdcInstrumentIDType g_pTradeInstrumentID = "zn1705";        // contract code for trading
TThostFtdcDirectionType gTradeDirection = THOST_FTDC_D_Sell;       // buy/sell direction
TThostFtdcPriceType gLimitPrice = 22735;                           // trading price

int main()
{
	// account and password
	cout << "Please enter account: ";
	scanf("%s", gInvesterID);
	cout << "Please enter password: ";
	scanf("%s", gInvesterPassword);

	// initialize market data thread
	cout << "Initializing market data..." << endl;
	g_pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();   // create market data instance
	CThostFtdcMdSpi *pMdUserSpi = new CustomMdSpi;       // create market data callback instance
	g_pMdUserApi->RegisterSpi(pMdUserSpi);               // register event class
	g_pMdUserApi->RegisterFront(gMdFrontAddr);           // set market data front address
	g_pMdUserApi->Init();                                // connect and run
	

	// initialize trading thread
	cout << "Initializing trading..." << endl;
	g_pTradeUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi(); // create trading instance
	CustomTradeSpi *pTradeSpi = new CustomTradeSpi;               // create trading callback instance
	g_pTradeUserApi->RegisterSpi(pTradeSpi);                      // register event class
	g_pTradeUserApi->SubscribePublicTopic(THOST_TERT_RESTART);    // subscribe to public stream
	g_pTradeUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);   // subscribe to private stream
	g_pTradeUserApi->RegisterFront(gTradeFrontAddr);              // set trading front address
	g_pTradeUserApi->Init();                                      // connect and run
		
	// wait for threads to exit
	g_pMdUserApi->Join();
	delete pMdUserSpi;
	g_pMdUserApi->Release();

	g_pTradeUserApi->Join();
	delete pTradeSpi;
	g_pTradeUserApi->Release();

	// convert local k-line data
	//TickToKlineHelper tickToKlineHelper;
	//tickToKlineHelper.KLineFromLocalData("market_data.csv", "K_line_data.csv");
	
	getchar();
	return 0;
}

#ifndef _BALANCE_H_
#define _BALANCE_H_
#include "stm32f10x_conf.h"	
#include "stdint.h"

#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                                                                 
	extern "C"        {
	#endif
#endif
void USART2_IRQHandler();			   
#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                 
	}
	#endif
#endif

void TaskCommBalance(void);
#define MAX_WEIGHT_BUF 4
#define ZERO 	Command[1] 	
#define WEIGHT	Command[0]  
////////////////////////////////////////////////////////
class Balance
{
public:
	enum BalanceState{balUnstable = 0,balStablish,balCantDO,balOverload,balUnload};	
	struct BalanceBuf  
	{
		char	DeviceName[24];	 		
		int CurrWeight;		  
	};
	unsigned int	StableWeight[MAX_WEIGHT_BUF];
	unsigned int	StableWeightPtr;
private:	 
	static char Command[2][10] ;
	char BuffRx[64];
	char * volatile BuffTx;
	volatile uint8_t		receTimeOut;
	volatile uint8_t		receCount ;
	volatile uint8_t		sendPosi;
	volatile uint8_t		sendCount;
	volatile uint16_t		TxTimeOut;
	////////////////////////////////////////
	char 	* volatile	m_cOldCommand;
	char	* volatile	m_cCommand;	
	char	* volatile	m_cTareCommand;		  
	char	* volatile	m_cCycCommand;
	////////////////////////////////////////
	/*
	char	 	DeviceName[24];
	uint32_t	StableWeight[MAX_WEIGHT_BUF];
	uint32_t	StableWeightPtr;
	uint32_t	CurrWeight;		  
	*/

	
	BalanceBuf      *pBalBuf;
	BalanceState    bState;		//
	bool 		    bWaitRx;
	//
	
	////////////////////////////////////////
	USART_TypeDef* port;
private:
	void DecodeWeight(void);
	void Send(void); 
	bool SendCommand(void);
public: 
	
	Balance(); 
	void InitPort(void);
	void ZeroWeight(void); 
	void TaskBalance (void);    
	void CommRoutine(void);
	long GetBalanceAvr();
	bool BalanceMaxErr(long lMaxErr);
};



extern 	Balance MTBalan;
#endif

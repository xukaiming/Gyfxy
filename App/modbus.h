#ifndef _MODBUS_H_
#define _MODBUS_H_
#include "task.h"


#define BAUDRATE0  9600
#define USART1_Tx_DMA_Channel    DMA1_Channel1
#define USART1_Rx_DMA_Channel    DMA1_Channel2
#define b485Send	PA12_O																		  

enum COMDIR{ComRx=0,ComTx=!ComRx};
uint16_t setRegisterVal(uint16_t addr,uint16_t tempData);    
void InitUART0(void);
void InitModbus(void) ;
void TaskCommTimer(void);

uint16_t getCoilVal(uint16_t addr,uint8_t *tempData);
void TaskIO(void);
void UpdateOutPut(void)	 ; 
#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                                                                 
	extern "C"        {
	#endif
#endif
void USART1_IRQHandler();
void USART2_IRQHandler();
void DMA1_Channel1_IRQHandler();
void DMA1_Channel2_IRQHandler();

#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                 
	}
	#endif
#endif
#endif

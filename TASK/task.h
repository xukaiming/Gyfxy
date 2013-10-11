/******************************************************************************
	FILE:       IED_TASK.H
	POPURSE:
	WRITER:      xukaiming
	DATE:         
******************************************************************************/
#ifndef _TASK_H_
#define _TASK_H_
					    
#include "stm32f10x.h"
#include "stdint.h"
////////////////////////////////////////
typedef unsigned char       BYTE;
typedef unsigned char      	UCHAR;
typedef short 				SHORT;
typedef unsigned short	 	USHORT;
typedef  long				LONG;
typedef unsigned int		UINT;
/////////////////////////////////////////
#ifndef   NULL   
  #ifdef   __cplusplus   
  	#define   NULL         0   
  #else   
  	#define   NULL         ((void   *)0)   
  #endif   
#endif  
#define ASSERT assert_param
//////////////////////////////////////////
/*-----------------------------------------------------------------------------
	定义任务表，增加任务时需在此增加表项。
-------------------------------------------------------------------------------*/
enum TASK_TYPE {TASK_TIMER=1,TASK_BLINK,TASK_COMMTIME,
				TASK_CMMD,TASK_BEEP	,TASK_STOP_BAR,TASK_COMM_TIMER,
				TASK_BALANCE,TASK_AD,TASK_FUZZY,TASK_IO	,TASK_CMD ,	 TASK_RESET,
				TASK_TEST,TASK_MANUAL,TASK_METAGE ,TASK_KEY,TASK_BURN	

};			   
#define TICKCOUNT (1000L)     //系统时间脉冲为1000次	   

#define PRIOD_FUZZY	2000UL 
#define PRIOD_TEST  1000UL
enum TASK_MODE {TASK_MODE_ONCE,TASK_MODE_PERIOD}; 	  
typedef struct task
{
    uint8_t   	cActive;            // >0:活动,1:初始化后立即启动
    uint8_t   	cTaskMode;          // 任务工作方式
    int32_t   	lLastTime;
    uint32_t  	lPeriod;            // 活动周期,uint=1mS
    uint8_t   	cType;              // 任务类型
    void    	(*TaskProc)(void);  // 任务处理子程序
	
}TASK_DATA;	   
extern TASK_DATA TaskDb[] ;
/***************************************************************************
    定时器
****************************************************************************/
#define MAX_TIMER               5L          // the number of timer can be used
enum TimerType{TIMER_MODE_TIMER=0,TIMER_MODE_ONCEROUTINE,TIMER_MODE_CYCROUTINE}	;
class TIMER_DATA
{

	uint8_t   	cUse;
    uint8_t   	cMode;                  // timer work mode
    int32_t   	lPeriod;                // timer period, unit=10ms.
    int32_t   	lLastTime;              // timer count for reload	    
    void    	(*routine)(void *   Param1);        // user's timer routine
	void 		*Param1;
	void 		TimerFun(void){if(routine!=0) routine(Param1);}
	/////////////////////////////
	public:
	static void TaskTimer(void);
	static int8_t StartTimer(uint8_t cMode,uint32_t lPeriod,void (* routine)(void * Param1),void * Param1);
	static int8_t StopTimer (int8_t * pcTimerNumber );
	static void   StopAllTimer(void);
};
extern TIMER_DATA TimerData[ ];


/////////////////////////////////////////////////////////////////////////////////////////
 	//绝对地址定位变量
#define BITBAND(addr, bitnum)  ((addr & 0xF0000000)+0x2000000 + ((addr &0xFFFFF)<<5) + (bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long *) addr)

#if !defined ( __CC_ARM   ) 
///////////////////////////////////////////////////////////////////////////////////////////////////////
	#ifndef __CONCAT
	#define __CONCAT(a, b)		__CONCAT_I(a, b)
	#define __CONCAT_I(a, b)	a ## b
	#endif	// __CONCAT
	
	#define SFR_ADDR(SFR, ADDR)	__SFR_ADDR(__COUNTER__, SFR, ADDR)	 
	#define __SFR_ADDR(CNT, SFR, ADDR)											\
		__attribute__((always_inline, naked, used)) static void __CONCAT(__sfr_addr, CNT)()	\
		{																		\
			__asm__ __volatile__(												\
				".global %c[sym]"	"\n\t"										\
				".weak %c[sym]"		"\n\t"										\
				".set %c[sym],%c[addr]"	"\n\t"									\
				".size %c[sym],%c[size]"										\
				: : [sym] "i" (&SFR), [addr] "i" (ADDR), [size] "i" (sizeof(SFR))	\
			);																	\
		}																		\
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	extern  unsigned long PD2_I;	SFR_ADDR(PD2_I, (BITBAND((int)&(GPIOD->IDR),2))); 
	extern  unsigned long PD3_I;	SFR_ADDR(PD3_I, (BITBAND((int)&(GPIOD->IDR),3)));
	extern  unsigned long PD4_I;	SFR_ADDR(PD4_I, (BITBAND((int)&(GPIOD->IDR),4)));
	extern  unsigned long PD5_I;	SFR_ADDR(PD5_I, (BITBAND((int)&(GPIOD->IDR),5)));
	extern  unsigned long PD6_I;	SFR_ADDR(PD6_I, (BITBAND((int)&(GPIOD->IDR),6)));
	extern  unsigned long PD7_I;	SFR_ADDR(PD7_I, (BITBAND((int)&(GPIOD->IDR),7)));
	
	extern  unsigned long PD15_I;	SFR_ADDR(PD7_I, (BITBAND((int)&(GPIOD->IDR),15)));
		
	extern  unsigned long PC0_O;	SFR_ADDR(PC0_O, (BITBAND((int)&(GPIOC->ODR),0)));
	extern  unsigned long PC1_O;	SFR_ADDR(PC1_O, (BITBAND((int)&(GPIOC->ODR),1)));
	extern  unsigned long PC2_O;	SFR_ADDR(PC2_O, (BITBAND((int)&(GPIOC->ODR),2)));
	extern  unsigned long PC3_O;	SFR_ADDR(PC3_O, (BITBAND((int)&(GPIOC->ODR),3)));
	extern  unsigned long PC4_O;	SFR_ADDR(PC4_O, (BITBAND((int)&(GPIOC->ODR),4)));
	extern  unsigned long PC5_O;	SFR_ADDR(PC5_O, (BITBAND((int)&(GPIOC->ODR),5)));
	extern  unsigned long PC13_O;	SFR_ADDR(PC13_O,(BITBAND((int)&(GPIOC->ODR),13)));
	
	extern  unsigned long PE15_O;   SFR_ADDR(PE15_O,(BITBAND((int)&(GPIOE->ODR),15)));
																					    
	/////////////////////////////////////////////////////////////////////////////////////
	 
#else
	#include <ABSACC.H>	  
	extern  uint32_t PB14_I;
	extern  uint32_t PD2_I;
	extern  uint32_t PD3_I;
	extern  uint32_t PD4_I;
	extern  uint32_t PD5_I;
	extern  uint32_t PD6_I;
	extern  uint32_t PD7_I;
	extern  uint32_t PD8_I;
	extern  uint32_t PD9_I;
	extern  uint32_t PD10_I;
	extern  uint32_t PD11_I;
	extern  uint32_t PD12_I;	
	extern  uint32_t PD13_I;
	extern  uint32_t PD14_I;
	extern  uint32_t PD15_I;
		
	extern 	uint32_t PA12_O;

	extern  uint32_t PB0_O ;
	extern  uint32_t PB1_O  ;	    
	extern  uint32_t PB7_O  ;
	extern  uint32_t PB8_O  ;
	extern  uint32_t PB9_O  ;				  

	extern  uint32_t PB12_O  ;	    
	extern  uint32_t PB13_O  ;  
	extern  uint32_t PB14_O  ; 
	extern  uint32_t PB15_O  ;

	extern  uint32_t PC0_O;
	extern  uint32_t PC1_O;
	extern  uint32_t PC2_O;
	extern  uint32_t PC3_O;
	extern  uint32_t PC4_O;
	extern  uint32_t PC5_O;
	extern  uint32_t PC13_O; 

	extern  uint32_t PE9_O;
	extern  uint32_t PE11_O;
	extern  uint32_t PE13_O;

	extern  uint32_t PE15_O;
#endif
// function prototype
void Init_task_timer(void);	 
void TaskProc(void);   
void Delay(unsigned long delayCnt) ;



//Task Function 
void ActiveTask(uint8_t cTaskType, uint32_t lPeriod);
void SuspendTask(uint8_t cTaskType); 

#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                                                                 
	extern "C"        {
	#endif
#endif

void SysTick_Handler(void);

#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                 
	}
	#endif
#endif

#define setbit(var,bit)  (var|=1L<<bit)
#define chkbit(var,bit)  (var&(1L<<bit))
#define clrbit(var,bit)  (var&=~(1L<<bit))
#endif 	

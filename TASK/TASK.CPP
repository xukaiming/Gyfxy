#include "task.h"
#include "ctrl.h"
#include "modbus.h"
#include "balance.h"	  
#include "cs1242.h"
#include "Test.H"
#include "KEY.H"


static void TaskTimer(void);

#if defined ( __CC_ARM   )

    uint32_t PB14_I  __at (BITBAND((int)&(GPIOB->IDR),14));

    uint32_t PD2_I __at (BITBAND((int)&(GPIOD->IDR),2));
    uint32_t PD3_I __at (BITBAND((int)&(GPIOD->IDR),3));
    uint32_t PD4_I __at (BITBAND((int)&(GPIOD->IDR),4));
    uint32_t PD5_I __at (BITBAND((int)&(GPIOD->IDR),5));
    uint32_t PD6_I __at (BITBAND((int)&(GPIOD->IDR),6));
    uint32_t PD7_I __at (BITBAND((int)&(GPIOD->IDR),7));
    uint32_t PD8_I __at (BITBAND((int)&(GPIOD->IDR),8));
    uint32_t PD9_I __at (BITBAND((int)&(GPIOD->IDR),9));
    uint32_t PD10_I __at (BITBAND((int)&(GPIOD->IDR),10));
    uint32_t PD11_I __at (BITBAND((int)&(GPIOD->IDR),11));
    uint32_t PD12_I __at (BITBAND((int)&(GPIOD->IDR),12));
    uint32_t PD13_I __at (BITBAND((int)&(GPIOD->IDR),13));
    uint32_t PD14_I __at (BITBAND((int)&(GPIOD->IDR),14));
    uint32_t PD15_I __at (BITBAND((int)&(GPIOD->IDR),15));


    uint32_t PA12_O __at (BITBAND((int)&(GPIOA->ODR),12));
    uint32_t PB0_O  __at (BITBAND((int)&(GPIOB->ODR),0));
    uint32_t PB1_O  __at (BITBAND((int)&(GPIOB->ODR),1));
    uint32_t PB7_O  __at (BITBAND((int)&(GPIOB->ODR),7));
    uint32_t PB8_O  __at (BITBAND((int)&(GPIOB->ODR),8));	   
    uint32_t PB9_O  __at (BITBAND((int)&(GPIOB->ODR),9));

    uint32_t PB12_O  __at (BITBAND((int)&(GPIOB->ODR),12));
    uint32_t PB13_O  __at (BITBAND((int)&(GPIOB->ODR),13));
    uint32_t PB14_O  __at (BITBAND((int)&(GPIOB->ODR),14));	   
    uint32_t PB15_O  __at (BITBAND((int)&(GPIOB->ODR),15));


    uint32_t PC0_O 	__at (BITBAND((int)&(GPIOC->ODR),0));
    uint32_t PC1_O 	__at (BITBAND((int)&(GPIOC->ODR),1));
    uint32_t PC2_O 	__at (BITBAND((int)&(GPIOC->ODR),2));
    uint32_t PC3_O 	__at (BITBAND((int)&(GPIOC->ODR),3));
    uint32_t PC4_O 	__at (BITBAND((int)&(GPIOC->ODR),4));
    uint32_t PC5_O 	__at (BITBAND((int)&(GPIOC->ODR),5));
    uint32_t PC13_O __at (BITBAND((int)&(GPIOC->ODR),13));

    //uint32_t PE9_O 	__at (BITBAND((int)&(GPIOE->ODR),9));
    uint32_t PE9_O	__at (BITBAND((int)&(GPIOE->ODR),9));
    uint32_t PE11_O	__at (BITBAND((int)&(GPIOE->ODR),11));
    uint32_t PE13_O	__at (BITBAND((int)&(GPIOE->ODR),13));
    uint32_t PE15_O __at (BITBAND((int)&(GPIOE->ODR),15));
#endif

//任务数据库
TASK_DATA TaskDb[]={										
    {1, TASK_MODE_PERIOD, 10L,    			10L,		    TASK_TIMER,			TaskTimer},
    {1, TASK_MODE_PERIOD, 100L,    			100L,		    TASK_BLINK,			TaskBlinLed} , 
    {0, TASK_MODE_PERIOD, 10L,    			10L,		    TASK_BEEP,			TaskBeep} , 
    {1, TASK_MODE_PERIOD, 10L,    			10L,		    TASK_STOP_BAR,		TaskStopBar} , 
    {1, TASK_MODE_PERIOD, 5L,				5L,			    TASK_COMM_TIMER,	TaskCommTimer},	
    {1, TASK_MODE_PERIOD, 5L,				5L,			    TASK_BALANCE,		TaskCommBalance},	
    {1, TASK_MODE_PERIOD, 10L,				10L,		    TASK_AD,			TaskReadAD},
    {1, TASK_MODE_PERIOD, PRIOD_FUZZY,	    PRIOD_FUZZY,    TASK_FUZZY,		    TaskFuzzy},
    {1, TASK_MODE_PERIOD, 5L,				5L,			    TASK_IO,			TaskIO},
    {1, TASK_MODE_PERIOD, 2L,				2L,			    TASK_CMD,	        TaskParsaCmd}, 
    {1, TASK_MODE_PERIOD, 20L,				20L,		    TASK_MANUAL,	    TaskManualKey}, 
    {0, TASK_MODE_PERIOD, 1000L,			1000L,		    TASK_RESET,	        TaskReset}, 
    {0, TASK_MODE_PERIOD, 1000L,			1000L,		    TASK_METAGE,	    TaskMetage}, 
    {0, TASK_MODE_PERIOD, PRIOD_TEST,	    PRIOD_TEST,	    TASK_TEST,	        TaskTest}, 	    
    {1, TASK_MODE_PERIOD, 20L,    		    20L,			TASK_KEY,			TaskScanKey} , 
    {0, TASK_MODE_PERIOD, 1000L,    		1000L,			TASK_BURN,			TaskBurnSample} , 
    };

TIMER_DATA TimerData[MAX_TIMER];
uint8_t gc1ms = 0;                //1ms到标志，控制多任务处理的时间隔为1ms
uint8_t TMP1msCount = 0;	   
void Init_task_timer(void)
{
} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/**********************************************************************************************************
    任务调度,在主循环中调用。
    由于是模拟多任务，所以要求每个任务的处理时间尽可能短。
***********************************************************************************************************/

void idle(void) 
{     
       		
}	 				   

void TaskProc(void)
{	   
	uint32_t i;
    if(!gc1ms)  //保证1ms扫描一次
	{
		idle();
	}	
	else
	{
		if(TMP1msCount>0)
		{
			for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
			{
				TaskDb[i].lLastTime-=TMP1msCount;
			}
		}
		gc1ms=0;
		TMP1msCount = 0;	  	  
	    for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
	    {
	        if(!TaskDb[i].cActive)
	            continue;
			
	        if(--TaskDb[i].lLastTime<=0)
					{
						TaskDb[i].lLastTime += TaskDb[i].lPeriod;
						if(TaskDb[i].TaskProc!=NULL)
								TaskDb[i].TaskProc();                                 //执行任务
	        }
	    } 
	}  
}
//Task Function 
void ActiveTask(uint8_t cTaskType, uint32_t lPeriod)
{

    unsigned int i;

    for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
    {
        if(TaskDb[i].cType==cTaskType)
        {
            if(TaskDb[i].cActive==0)
            {
                TaskDb[i].cActive=1;
                TaskDb[i].lPeriod=lPeriod;
                TaskDb[i].lLastTime=lPeriod;
            }
            break;
        }
    }
	
}


void SuspendTask(uint8_t cTaskType)
{

    unsigned int i;
    for(i=0;i<sizeof(TaskDb)/sizeof(TaskDb[0]);i++)
    {
        if(TaskDb[i].cType==cTaskType)
        {
            TaskDb[i].cActive=0;
            break;
        }
    }
	
}
//Timer Function 
int8_t TIMER_DATA::StartTimer(uint8_t cMode,uint32_t lPeriod,void (* routine)(void *   Param1),void *   Param1)
{

	
    unsigned int   i;
    TIMER_DATA *pt;

    pt=&TimerData[0];
    for(i=0;i<MAX_TIMER;i++,pt++)
    {
        if(!pt->cUse)
        {
            switch(cMode)
            {
                case TIMER_MODE_TIMER:              // timer only
                case TIMER_MODE_ONCEROUTINE:        // timely routine
                case TIMER_MODE_CYCROUTINE:         // cyclely routine
                    break;
                default:
                    pt->cUse = 0;
                    return -1;                      // error
            }
            pt->lPeriod     = lPeriod;              // set timer period
            pt->lLastTime   = lPeriod;	    		// get current system time as start timer
            pt->routine     = routine;              // user timer's routine
            pt->cMode       = cMode;                // timer's work mode
            pt->cUse        = 1;
			pt->Param1      = Param1;
            return (i);                             // return timer number
        }
    }
    
    #ifdef _DEBUG
	if(i== MAX_TIMER)
    	printf("定时器资源不足,申请不成功");
    #endif 	
    return (-1);                                    // no timer available
	
}

int8_t TIMER_DATA::StopTimer (int8_t * pcTimerNumber )
{
	
    if(*pcTimerNumber>=MAX_TIMER || *pcTimerNumber==-1)
        return -1;                            // timer number error

    TimerData[*pcTimerNumber].cUse=0;
    *pcTimerNumber = -1;  
    return 0;
	
	
}
    


void TIMER_DATA::StopAllTimer(void)
{  
    unsigned int i;
    for(i=0;i<MAX_TIMER;i++)
    {
        TimerData[i].cUse=0;
    }	   
}


void TIMER_DATA::TaskTimer(void)
{
	TIMER_DATA *pt;
    unsigned int i;

    pt=&TimerData[0];                         				// pointer to the first timer
    for(i=0;i<MAX_TIMER;i++,pt++)
    {
        if(!pt->cUse)
            continue;
        if((--(pt->lLastTime))<=0)
        {
            switch(pt->cMode)
            {
                case TIMER_MODE_TIMER:          			// timer only
                     pt->cUse=0;
                    break;
                case TIMER_MODE_CYCROUTINE:               	// cyclely routine
                     pt->lLastTime+=pt->lPeriod;
                     pt->TimerFun();                      	// start user's routine
                    break;
                case TIMER_MODE_ONCEROUTINE:            	// timer routine once
                     pt->cUse=0;
                     pt->TimerFun();                     	// start user routine 
                    break;
                default:
                    pt->cUse=0;
                    break;
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FUNCTION:   TaskTimer
//	PURPOSE:    timer service routine.
////////////////////////////////////////////////////////////////////////////////////////////////////////

void TaskTimer(void)
{
	 TIMER_DATA::TaskTimer(); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//    定时器中断服务程序,定时器0中断1次/10mS.
//////////////////////////////////////////////////////////////////////////////////////////////////////


volatile unsigned  char i = 0;
void SysTick_Handler(void)
{
	if(gc1ms==1)
	{
		TMP1msCount++;
	}   	 
	
	gc1ms=1;		 			  
}

void Delay(unsigned long delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}



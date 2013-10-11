#include "cs1242.h"	   
#include "main.h"
#include "EEPROM.H"	
#include "modbus.h"
#include "balance.h"
#include "ctrl.h"
#include "stm32_init.h"


int main(void)
{	 
    #ifdef _DEBUG
    char fdWord[]={"褒饼昌除待"}; //0xfd错误测试
    #endif
    
	stm32_Init();
	TIMER_DATA::StopAllTimer();
	InitIED();
	#if defined ( __CC_ARM   )
	InitStove();				//初始化三个炉子的送样电机
	InitStepMotor();			//初始化转盘电机
 	#endif
	//StepMotor[0].MoveStepMotor(1); 
	Delay(0xFFFFFF);
	InitCS1242();
	MTBalan.InitPort();
	InitFuzzy();
 	//PB0_O=PB1_O=PB9_O =1;	   //加热风扇输出口
	beep.beep(1,500);	
	InitModbus(); 
	while(1)
    {
        TaskProc();					   
	}		    
}


#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{ 
	__nop();
  	while (1)
  	{
  	}
}

#endif

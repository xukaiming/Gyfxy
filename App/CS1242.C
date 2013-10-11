#include "cs1242.h"
#include "ctrl.h"
#include "task.h"
volatile unsigned char Int0IntFlag = 0;  
unsigned char gcChannel   = 0;			   

unsigned char count   = 0;


unsigned char SPI1_Receive(void);
void  SPI1_Send(unsigned char  i);
unsigned char gcChannelDef[3]; 
#define LOW 	0
#define HIGH 	1
//#define  HW_SPI_CS1242
#ifdef   HW_SPI_CS1242  
void  InitSPI1(void)
{
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA   |RCC_APB2Periph_AFIO, ENABLE);
}											   
#define  CS_Enable()  			//SPI1_SS0      
#define  CS_Disable() 			//SPI1_SS0    
void  SPI1_Send(unsigned char SPI_data)
{
    									//Check busy                
}

unsigned char SPI1_Receive(void) 
{      
	return 0 ;
} 

#else   

#define CS   PB12_O 
#define SCLK PB13_O
#define SDO  PB14_I
#define SDO_H  PB14_O
#define SDI  PB15_O

void  InitSPI1(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);	 
	GPIO_InitTypeDef GPIO_InitStructure;	     

  	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_14;
  	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15;
  	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	CS 		= 1;
	SCLK	= 1;	
	PB14_O  = 1;
	SDI		= 1;
	 
}
/**************************************************************************************
*函数原型：	void SPI_clock(void)
*功能	 ：	向spi总线上发送CLK时信号
*/

#define delay() Delay(50) 

/**************************************************************************************
*函数原型：	void SPI_send(unsigned char SPI_data)
*功能	 ：	向spi总线上发送8位数据信号
**************************************************************************************/
void SPI1_Send(unsigned char SPI_data)
{				    
	unsigned char bitCount;
//	disable();
	for (bitCount=0; bitCount<8; bitCount++)
	{					   
		SCLK = LOW;
		if(SPI_data & 0x80)
			SDI = 1;
		else
			SDI = 0;
		
		SPI_data <<= 1;
		delay();delay();delay();delay();
		delay();delay();delay();delay();
		SCLK = HIGH;
		delay();delay();delay();delay();
		delay();delay();delay();delay();
		
	}
//	enable();		            
}
/**************************************************************************************
*函数原型：	unsigned char SPI_receive(void)
*功能	 ：	从spi总线上接受8位数据信号，并将接收到的数据作为一个字节返回
**************************************************************************************/
 
unsigned char SPI1_Receive(void)
{
	unsigned char bitCount;
	unsigned char SPI_data=0;
	SDO_H = 1;
//	disable();
	for (bitCount=0; bitCount<8; bitCount++)
	{
		SPI_data <<=1;
		
		SCLK = LOW;
		
		delay();delay();delay();delay();
		delay();delay();delay();delay();
		SCLK = HIGH;
		delay();delay();delay();delay();
		delay();delay();delay();delay();
		if (SDO)
		{
			SPI_data++;
		}
	}
//	enable();
	return (SPI_data);
 
}   

#define  CS_Enable()  	CS = 0;
#define  CS_Disable()  	CS = 1;

#endif

unsigned char buf[17]={'\0'};
void InitCS1242(void)
{                        
    unsigned char m;
    gcChannelDef[0] = CHANNEL0  ;
    gcChannelDef[1] = CHANNEL1  ;
	gcChannelDef[2] = CHANNEL2  ;
    InitExt1(); 
    InitSPI1();   
	Delay(0xFF);                              
    CS_Enable();
    SPI1_Send(CMD_RESET);
    CS_Disable();
    Delay(0xFF);
    CS_Enable();                                  
    SPI1_Send(CMD_WREG);
	SPI1_Send(0x02);					//发送3个指令
	SPI1_Send(0x03);					// 0x00	pga = 4
	SPI1_Send(0x03);					// 0x01	设置ad 通道
	SPI1_Send(0x64);					// 关闭BUF
    CS_Disable();
	Delay(0xFF);
    CS_Enable();
	SPI1_Send(CMD_SELFCAL);                         
    CS_Disable();
    Delay(0xFFFF);                     //做通讯检测用，主要是读取寄存器的值看是否与写入的值相符合
					
    /*
    while(1)
    {                                                    
        CS_Enable();
    	SPI1_Send(CMD_RREG);         
    	SPI1_Send(15);   
		Delay(0xFF); 
    	for (m=0; m <16; m++)
    	{
    		buf[m] = SPI1_Receive( );
    		//printf("%x ",(int)buf[m])	 ;
    	}                                               
        __NOP();
        CS_Disable();
		Delay(0xFFFF);
    } 
     */           	
}

/****************************************************************************
 Function:      AdsWriteReg
 Description:   Write a register from ADS1242,只写一个寄存器
****************************************************************************/
void WriteReg(unsigned char cRegAddr,unsigned char cData)
{
    CS_Enable();

    SPI1_Send(CMD_WREG|cRegAddr);      //registe ID
    SPI1_Send(0);                     //# of regs-1,只写一个寄存器     
    SPI1_Send(cData);

    CS_Disable();
}

long ReadAdData(void)
{                       
	union LongData a; 
    a.word = 0; 
    CS_Enable();
	Delay(5);
	SPI1_Send(CMD_RDATA);   
	Delay(0xF);
    a.byte[3] = SPI1_Receive();   
    a.byte[2] = SPI1_Receive(); 
    a.byte[1] = SPI1_Receive();   
	a.byte[0] = 0xFF; 
	//a.word = (a.word<<8);
    //a.word /= 256	;
    CS_Disable();
	return(a.word);   
}

                                                   
														   
void  InitExt1(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;                        
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
  /* Configure PA.00 pin as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable AFIO clock */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  /* Connect EXTI0 Line to PB.05 pin */
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

  /* Configure EXTI0 line */
  	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI0 Interrupt to the lowest priority */
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
 
} 

double ADToVol(int iADValue)
{
    double i = VREF>>3;
    i*=iADValue;
    i/=0xFFFFFF;
    return i;
}



//外部中断函数  
void EXTI9_5_IRQHandler(void)
{									   
  if(EXTI_GetITStatus(EXTI_Line5) != RESET)
  {									    
    /* Clear the  EXTI line 0 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line5);
	Int0IntFlag = 1; 
  } 
}


//long ad[3] = {0};
long long Voltage;
void TaskReadAD(void)
{    
	
	long *ad = IedDb.ADValue.ad;
    if(Int0IntFlag)
    {
        Int0IntFlag = 0; 
        if(count>4)
        {
			ad[gcChannel] =   ReadAdData()>>8;  	
			/*
			Voltage 	=  (ad[gcChannel]);
			Voltage		*= 5000000;
			Voltage		/= (0xFFFFFF<<4);				    
			*/
			Delay(0xF);
            #ifdef _DEBUG  

            //printf("\n"); 
            #endif
            count=0;
            gcChannel=(++gcChannel)%3;  
            WriteReg(REG_MUX,gcChannelDef[gcChannel]);
        }
        count++;  
    }    
	    
}   

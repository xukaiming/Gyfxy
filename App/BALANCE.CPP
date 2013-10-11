#include "balance.h"
#include "string.h"	 
#include <stdlib.h>  
#include "stm32f10x_usart.h"
#include "ctrl.h"

char Balance::Command[2][10] = 
{
	"SI\r\n", 	//读取重量
	"ZI\r\n",	//清0	

} ;

Balance MTBalan;  
#define CMD_WEIGHT  Command[0]
#define CMD_TARE  	Command[1]
Balance::Balance()
{
	port 		= USART2;		 
	receTimeOut	= 0;
	receCount 	= 0;
	sendPosi	= 0;
	sendCount	= 0;
	TxTimeOut	= 0;
	////////////////////////////////////////

	m_cCycCommand 	= Command[0];	  	 
	bState			= balUnstable;
	bWaitRx			= 0;	

	////////////////////////////////////////		  
}


void USART2_IRQHandler()
{										  
	MTBalan.CommRoutine();
}



void Balance::CommRoutine(void)
{
	  uint32_t sr = port->SR;
	  if(USART_GetITStatus(port, USART_IT_RXNE) != RESET)
	  {
	  		receTimeOut = 5;     //通讯超时值 
			BuffRx[receCount++]=  port->DR;
	  }
	  	   													    
	  if(USART_GetITStatus(port, USART_IT_TXE) != RESET)
	  {   	
		if(sendPosi < sendCount)
		{    
			port->DR =   BuffTx[sendPosi]; 		           
			sendPosi++;				
		}
        else
        {  								  
			USART_ITConfig(port, USART_IT_TXE, DISABLE); 	 
			USART_ITConfig(port, USART_IT_TC, ENABLE); 	    
			
        }
	  }
	   
	  if(USART_GetITStatus(port, USART_IT_TC) != RESET)
	  {
	  		USART_ITConfig(port, USART_IT_TC, DISABLE);   	  
	  }
	  	   
}
				   
long Balance::GetBalanceAvr()
{
    char i;
    long result;
    double v;
    
    v=0.0f;
    for(i=0;i<MAX_WEIGHT_BUF;i++)
        v+=StableWeight[i];
    result = v/MAX_WEIGHT_BUF+0.5f;
    return result;
}

/*****************************************************************************
    判极差
*****************************************************************************/
bool Balance::BalanceMaxErr(long lMaxErr)
{
    char i,j;
    long d;
    long min,max;

	if(bState!=balStablish)
		return false;
	min=max=StableWeight[0];

    for(i=0;i<MAX_WEIGHT_BUF;i++)
    {
		if(StableWeight[i]<min)
			min = StableWeight[i]; 
		if(StableWeight[i]>max)
			max = StableWeight[i]; 
		if(max-min>lMaxErr)		
			return false;
    }
    return true;
}
 
void Balance::ZeroWeight(void)
{								  
 	m_cTareCommand = CMD_TARE;
}

void TaskCommBalance(void)
{	 										    
	 MTBalan.TaskBalance();
}

void Balance::TaskBalance(void)
{
	if(receTimeOut>0)
	{	
		receTimeOut--;				 			   
		if(receTimeOut==0 )   //判断通讯接收是否超时
		{	
            if(receCount>2)
            {
                DecodeWeight();	
            }                    
			receCount 		= 0;      //将接收地址偏移寄存器清零       
		}	
		return;		
	}
	if(TxTimeOut>0)							//延时启动发送任务
	{
		TxTimeOut--;
		__nop();
		if(TxTimeOut==0)
		{								  
    		sendPosi = 0;
			Send();				   
		}
		return;
	}
	
	if(m_cCommand)						//有待发命令
	{
		if(SendCommand())				//发送命令
		{
			m_cOldCommand=m_cCommand;	//保存已发命令
			m_cCommand=0;				//清待发命令
			__nop();
		}
		return;
	}  
	bWaitRx = true;	
	if(m_cTareCommand)
	{					    
		m_cCommand	= m_cTareCommand;
		m_cTareCommand = 0;
		return;
	}
	if(m_cCycCommand)		//有循环命令
	{	 		  
		m_cCommand	= m_cCycCommand;
  	}
	
	return;
}

bool Balance::SendCommand(void)
{								 	 
	if(!bWaitRx&&(bState==balStablish))	 //接受完毕,并且为稳定数据	   	  
	   return true;

	TxTimeOut = 20;		  //0.1s	    
	
	return false;
}

void Balance::Send(void)
{ 								  
	receTimeOut = 10;
	BuffTx = m_cCommand;
	sendCount = strlen(BuffTx);
	USART_ITConfig(port, USART_IT_TXE, ENABLE); //运行发送中断   
} 
  
void Balance::DecodeWeight(void)
{
	double dWeight;
	bWaitRx = false;	
	if((BuffRx[0]!='S')||(BuffRx[1]!=' '))		  //不是正常的包头
		return;
	switch(BuffRx[2])
	{
		case 'S':
			bState	= balStablish;
			break;
		case 'D':
			bState	= balUnstable;
			break;
		case 'I':
			bState	= balCantDO;
			break;
		case '+':
			bState	= balOverload; 
			//TODO: SafeBalance();
			break;
		case '-':
			bState	= balUnload;
			break;
		default:
			break;
	}
	if((bState==balStablish)||(bState==balUnstable))	  //接收到正常的重量数据,开始解码
	{
		//BuffRx[15]=BuffRx[0]=BuffRx[2] = ' ';
		dWeight 	= atof(BuffRx+3);
		pBalBuf->CurrWeight	= dWeight*10000L;	
        if(pBalBuf->CurrWeight==0x7FFFFFFF)
        {
            __nop();
        }

		if(bState==balStablish)
		{ 
			StableWeight[StableWeightPtr]	= pBalBuf->CurrWeight;
			if(MAX_WEIGHT_BUF&(MAX_WEIGHT_BUF-1))			 //如果是2 的n次方,结果=0
				StableWeightPtr				= (++(StableWeightPtr))%MAX_WEIGHT_BUF;
			else
				StableWeightPtr				= (++(StableWeightPtr))&(MAX_WEIGHT_BUF-1);
			__nop();
		}		  
	}
	
}

void Balance::InitPort(void)
{	
	NVIC_InitTypeDef NVIC_InitStructure;		 					    
	USART_InitTypeDef USART_InitStructure;	    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	 
	//端口初始化
	// Configure USART1 Tx as alternate function open-drain 
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_3;		    	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);  
		
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_1;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//串口初始化
	USART_InitStructure.USART_BaudRate = 9600;
  	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  	USART_InitStructure.USART_StopBits = USART_StopBits_1;
  	USART_InitStructure.USART_Parity = USART_Parity_No;
  	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_NONE;//;
  	USART_InitStructure.USART_Mode = USART_Mode_Rx| USART_Mode_Tx;
	USART_Init(port, &USART_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);		
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;		   //DMA完成.错误中断									 
	if(port==USART2)
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; 
	NVIC_Init(&NVIC_InitStructure);	

	//USART_ITConfig(USART1,USART_IT_TXE, ENABLE);
	//USART_ITConfig(USART1,USART_IT_TC, ENABLE);
	USART_ITConfig(port,USART_IT_RXNE, ENABLE);
	
	USART_Cmd(port, ENABLE); 		  

	///////////////////////////
	pBalBuf = &IedDb.buf;
 	strcpy(pBalBuf->DeviceName,"METTLER-WKE204/01");
}
 

#include "ctrl.h"
#include "test.h"
#include "task.h"
#include "string.h"
#include "stm32f10x_tim.h"
#include "math.h"
#include "KEY.H"
#include "modbus.h"
#include "EEPROM.h"
#include "string.h"
IED_DATABASE  IedDb __at (0x20001000);
void InitIED(void)
{

	memset(&IedDb,0,sizeof(IedDb));
//	strcpy(IedDb.DownloadWorkParam.reserve,"Attrib Block\n");
	strcpy(IedDb.InstrumentName,"TF_GYFXY2012");
	IedDb.cID = 21;

	IED_DATABASE::YP_DATA volatile *  Sample = IedDb.Sample1;
	memset((void*)Sample,0,sizeof(IedDb.Sample1)/sizeof(char));
    
    IedDb.DownloadWorkParam.m_btMaxSampleCnt        = 24;				//最大工位	 
	IedDb.DownloadWorkParam.m_sPulsePerRound1Spit   = 200;;				//每圈1细分下所需要的脉冲
// 	IedDb.DownloadWorkParam.m_btDriverSpitCnt       = 16;;
// 	for(int i=0;i<sizeof(IedDb.Sample1)/sizeof(IED_DATABASE::YP_DATA);i++)
// 	{
// 		Sample[i].cSta=S_WAIT;	
// 	} 
}
//					  
 				  
class Timer4{
    public:
        static Timer4 *getTimer()
        {
            if (timer == NULL) {	  
                timer  = new Timer4();
				
            }
            return timer;
        }
	
    private: 
        Timer4(){InitTimer();};
        Timer4(const Timer4 &s){};
        void operator=(const Timer4 &rhs){};
		void InitTimer()
		{
			GPIO_InitTypeDef GPIO_InitStructure;
			TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_9;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //下拉接地，检测输入的高电平
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50M时钟速度
			GPIO_Init(GPIOB, &GPIO_InitStructure);
		
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_7|GPIO_Pin_8;		  //方向/脱机IO口
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //下拉接地，检测输入的高电平
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50M时钟速度
			GPIO_Init(GPIOB, &GPIO_InitStructure);
													  
		    TIM_DeInit(TIM4);
		
		  	TIM_TimeBaseStructure.TIM_Period 		= 0xFFFF;//65535;
		  	TIM_TimeBaseStructure.TIM_Prescaler 	= 17;
		  	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		  	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;
		  	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
		
		  	/* TIM enable counter */
		  	TIM_Cmd(TIM4, ENABLE);
		}
		private:
        	static Timer4  *timer;	 //TIM_TypeDef	
		public:
			StepMotorCtrl::StepMotorState  *pMtrState[5];			  
	 
};
							    
 
Timer4 *Timer4::timer  = NULL;	    							    
////////////////////////////////////////////////////////////////////
Beep beep;
void TaskBeep(void)
{
	beep.TaskBeep();
}	

////////////////////////////////////////////////////////////////////
#if !defined ( __CC_ARM   )
VertStoveBar VertStove[3] =

{
	{&PD3_I,&PD2_I,&PC4_O,&PC5_O,500,1000},
	{&PD5_I,&PD4_I,&PC0_O,&PC1_O,500,1000},
	{&PD7_I,&PD6_I,&PC2_O,&PC3_O,500,1000} 
};
#else 
LowVertStoveBar  m_LowTempStoveBar[1];
HighVertStoveBar m_HighTempStoveBar[2]; 

void InitStove(void)
{																	   
	IED_DATABASE::DownParam *param = &IedDb.DownloadWorkParam;														
	LowTempStove.InitStoveBar(&PD12_I,&PD13_I,&PC0_O,&PC1_O,&param->m_sTimePut2weigh);
	LeftHighTempStove.InitStoveBar(&PD10_I,&PD11_I,&PC2_O,&PC3_O,&param->m_sTimePut2HighTemp);
	RightHighTempStove.InitStoveBar(&PD8_I,&PD9_I,&PC4_O,&PC5_O,&param->m_sTimePut2HighTemp);
    
	LowTempStove.InitStoveBar(&PD12_I,&PD13_I,&PC0_O,&PC1_O,&param->m_sTimePut2weigh);
	LeftHighTempStove.InitStoveBar(&PD10_I,&PD11_I,&PC2_O,&PC3_O,&param->m_sTimePut2HighTemp);
	RightHighTempStove.InitStoveBar(&PD8_I,&PD9_I,&PC4_O,&PC5_O,&param->m_sTimePut2HighTemp);
      
}
#endif




void TaskStopBar(void)
{												 
	for(unsigned int i=0;i<sizeof(m_LowTempStoveBar)/sizeof(LowVertStoveBar);i++)
	{
		m_LowTempStoveBar[i].TaskStopBar();
	}
    for(unsigned int i=0;i<sizeof(m_HighTempStoveBar)/sizeof(HighVertStoveBar);i++)
	{
		m_HighTempStoveBar[i].TaskStopBar();
	}
}
///////////////////////////////////////////////////////////////////////////////
StepMotorCtrl StepMotor[2];	 
 
StepMotorCtrl::StepMotorCtrl()
{
    /*
	AllSampleCnt 			= MAX_SAMPLE_CNT ;				//最大工位	 
	PulsePerRoundSubDiv		= DEFAULT_PULSECNT_PERROUND;	//每圈1细分下所需要的脉冲
	SubDiv					= DEFAULT_SUBDIV;				//细分数
	*/
	//pMtrState->CurSamplePos		= 0;						//当前工位	 
	AllSampleCnt 			= &IedDb.DownloadWorkParam.m_btMaxSampleCnt;				//最大工位	 
	PulsePerRoundSubDiv		= &IedDb.DownloadWorkParam.m_sPulsePerRound1Spit;;				//每圈1细分下所需要的脉冲
	SubDiv					= &IedDb.DownloadWorkParam.m_btDriverSpitCnt;;				//细分数 
	
}

void StepMotorCtrl::InitMotor(long AllSampleCnt,long PulsePerRoundSubDiv,long SubDiv)
{
	*this->AllSampleCnt 		= AllSampleCnt ;		//最大工位	 
	*this->PulsePerRoundSubDiv	= PulsePerRoundSubDiv;	//每圈1细分下所需要的脉冲
	*this->SubDiv				= SubDiv;				//细分数	
	
}

void StepMotorCtrl::InitCtrl(TIM_TypeDef *TIMx ,int Channel, uint32_t* pPortFree, uint32_t* pPortDir,uint32_t* pResetIn)
{
 	pMtrState	  				= &IedDb.MtrState;
	pMtrState->bRun				= false;					//运行
	pMtrState->bDir				= FORWARD;					//方向
	pMtrState->DestPulseCnt		= 0x4FFFF;					//到达目标位置脉冲数
	pMtrState->CurPulseCnt		= 0x3FFFF;					//当前位置脉冲书
	pMtrState->CurSamplePos		= 0;
	pMtrState->ResetIn			= pResetIn;

	this->TIMx				= TIMx;						//使用的定时器
	this->Channel			= Channel;					//定时器通道
	if(TIMx==TIM4)
	{							  
		Timer4 *timer  				= Timer4::getTimer();	
		timer->pMtrState[Channel]	= pMtrState;	   
		__nop();
	}

	this->pPortFree			= pPortFree;				//锁定引脚
	*pPortFree				= UNLOCK ;					//初始化后未被锁定
	this->pPortDir			= pPortDir;					//方向引脚
	*pPortDir				= pMtrState->bDir;
	InitTimer();
}

void InitStepMotor(void)
{				
	StepMotor[0].InitCtrl(TIM4,1,&PB7_O,&PB8_O,&PD15_I);									    
	//StepMotor[1].InitCtrl(TIM4,4,&PB1_O,&PB0_O);
}	   

int StepMotorCtrl::MoveStepMotor(long DestPulseCnt,StepMotorCtrl::DIR direction)
{ 
	if(DestPulseCnt==pMtrState->CurPulseCnt)
	{
		return 0;
	}										  
	if((LeftHighTempStove.m_StovePos!=HighVertStoveBar::POS_BOT)
		||(RightHighTempStove.m_StovePos!=HighVertStoveBar::POS_BOT))
    {
        strncpy(ErrBuf,"左炉或右炉样杆不在底位",INFO_LENTH);
        ErrFlg = true;
		return -2;
    }
	if(LowTempStove.m_StovePos!=LowVertStoveBar::POS_TOP)
    {
        strncpy(ErrBuf,"样盘不在顶位",INFO_LENTH);
        ErrFlg = true;
		return -3; 
    }
	pMtrState->bRun 			= true;
	*pPortFree					= LOCK ;
	*pPortDir = pMtrState->bDir = direction;    	 
	pMtrState->DestPulseCnt	= DestPulseCnt;
	SetFreq(200*(*SubDiv)/16); 	
	#ifdef _DEBUG
		printf("转动步进电机") ;
	#endif 	
	return 0;	
}

void StepMotorCtrl::StopMotor(void)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;							  
	TIM_OCInitStructure.TIM_OCMode			= TIM_OCMode_Timing	;	  	
	TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable; 	  
	TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_Low; 
	TIM_OCInitStructure.TIM_Pulse 			= 0;//CCR3_Val;
	pMtrState->bRun							= 0;
	switch(Channel)
	{
		case 1:					    
			TIM_OC1Init(TIMx, &TIM_OCInitStructure);
			TIM_ITConfig(TIMx,  TIM_IT_CC1, DISABLE); 
			
			break;
		case 2:					    
			TIM_OC2Init(TIMx, &TIM_OCInitStructure);
			TIM_ITConfig(TIMx,  TIM_IT_CC2, DISABLE);
			break;
		case 3:					    
			TIM_OC3Init(TIMx, &TIM_OCInitStructure);
			TIM_ITConfig(TIMx,  TIM_IT_CC3, DISABLE);
			break;
		case 4:					    
			TIM_OC4Init(TIMx, &TIM_OCInitStructure);
			TIM_ITConfig(TIMx,  TIM_IT_CC4, DISABLE);
			break;
		default:
			break;
	}
}
		    
int StepMotorCtrl::MoveStepMotor(long DestPos)
{
	int32_t step;
	StepMotorCtrl::DIR direction = FORWARD; 
	////////////////////////////////////////////////////////////////////////////////
	//将目标位置限定于24个样品之内
    if(DestPos<0)
        DestPos		= (DestPos-1)%(*AllSampleCnt)+*AllSampleCnt+1;
	if(DestPos!=0)
		DestPos		= (DestPos-1)%(*AllSampleCnt)+1;
	pMtrState->DestSamplePos = DestPos;
    ////////////////////////////////////////////////////////////////////////////////

	int tDest = (DestPos<pMtrState->CurSamplePos)?DestPos+(*AllSampleCnt):DestPos;
	//判断正反转	
	direction = ((tDest - pMtrState->CurSamplePos )>=(*AllSampleCnt)/2)?BACKWARD:FORWARD;
	////////////////////////////////////////////////////////////////////////////////
	
	pMtrState->PulsePerRound 	= 2 //每个脉冲要翻转两次
								*(*PulsePerRoundSubDiv)*(*SubDiv);	 		
	////////////////////////////////////////////////////////////////////////////////
	if(pMtrState->CurSamplePos==0)									  //第一次旋转
	{
		pMtrState->DestPulseCnt = pMtrState->PulsePerRound;//
	} 																			  	
 	////////////////////////////////////////////////////////////////////////////////
	step =  (DestPos-1)*pMtrState->PulsePerRound/(*AllSampleCnt); 			//走到目标位置从1位起的脉冲数目,(绝对定位)
     ////////////////////////////////////////////////////////////////////////////////
	return MoveStepMotor(step,direction);  
}

vu16 CCR1_Val = 10000;	
vu16 CCR2_Val = 10000;	
vu16 CCR3_Val = 10000;	
vu16 CCR4_Val = 10000;	
 
void StepMotorCtrl::InitTimer(void)
{	
	if(TIMx==TIM4)							  
		Timer4 *timer  = Timer4::getTimer();
}

void StepMotorCtrl:: SetFreq(uint16_t freq)
{
	uint16_t CCR_Val;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	
	RCC_ClocksTypeDef ClockFreq;
	RCC_GetClocksFreq(&ClockFreq) ;   

	ASSERT(freq<MAX_FREQ);				//限定最大输出频率
	if(freq==0)
	{
	 	// Output Compare Toggle Mode configuration: Channel1 
	  	TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_Timing; 
		TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable; 
		TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_Low; 
		TIM_OCInitStructure.TIM_Pulse 			= CCR_Val;//CCR3_Val;  
		
		TIM_OC1Init(TIMx, &TIM_OCInitStructure); 
		TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Disable);
		TIM_ITConfig(TIM4,  TIM_IT_CC1, DISABLE);   
	}
	else 
	{
		CCR_Val = ClockFreq.SYSCLK_Frequency/freq/36;	 
		TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_Toggle;  
		
	}
	TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_Low; 
	TIM_OCInitStructure.TIM_Pulse 			= CCR_Val;//CCR3_Val;
	switch(Channel)
	{
		case 1:
			CCR1_Val = CCR_Val;
			if(((TIMx->CCMR1)&TIM_OCMode_Toggle)!=TIM_OCMode_Toggle)
			{
				TIM_OC1Init(TIMx, &TIM_OCInitStructure); 
				TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Disable);
				TIM_ITConfig(TIM4,  TIM_IT_CC1, ENABLE); 
			}
			break;
		case 2:
			CCR2_Val = CCR_Val;
			if(((TIMx->CCMR1>>8)&TIM_OCMode_Toggle)!=TIM_OCMode_Toggle)
			{
				TIM_OC2Init(TIMx, &TIM_OCInitStructure); 
				TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Disable);	
				TIM_ITConfig(TIM4,  TIM_IT_CC2, ENABLE);
			}
			break;
		case 3:
			CCR3_Val = CCR_Val;
			if(((TIMx->CCMR2)&TIM_OCMode_Toggle)!=TIM_OCMode_Toggle)
			{
				TIM_OC3Init(TIMx, &TIM_OCInitStructure); 
				TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Disable);
				
				TIM_ITConfig(TIM4,  TIM_IT_CC3, ENABLE);
			}
			break;
		case 4:	  
			CCR4_Val = CCR_Val;
			if(((TIMx->CCMR2>>8)&TIM_OCMode_Toggle)!=TIM_OCMode_Toggle)
			{
				TIM_OC4Init(TIMx, &TIM_OCInitStructure); 
				TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Disable);	 
				TIM_ITConfig(TIM4,  TIM_IT_CC4, ENABLE);
			}
			break;
		default:
			break;
	} 
}
	 

void TIM4_IRQHandler(void) 
{ 	
 
	TIM_OCInitTypeDef  TIM_OCInitStructure;	
	TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_Timing;  	
	TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_Low; 
																		  
	Timer4 *timer  = Timer4::getTimer();
	StepMotorCtrl::StepMotorState  **pMtrSte = timer->pMtrState;

  	if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)               	//检查指定的TIM中断发生与否
  	{  
		static uint32_t PreResetIn        = 0;
		TIM4->SR = (uint16_t)~TIM_IT_CC1;				   			//清除TIMx的中断待处理位
 		TIM4->CCR1 	+=CCR1_Val;							  			//件也是相同的脉冲数

		if(pMtrSte[1]!=0)								    		//被初始化
		{
			if(pMtrSte[1]->bDir==StepMotorCtrl::FORWARD)
			{
				(pMtrSte[1]->CurPulseCnt)++;
			}
			else
			{
				(pMtrSte[1]->CurPulseCnt)--;
			} 
		}
		if(pMtrSte[1]->CurPulseCnt >pMtrSte[1]->PulsePerRound)
			pMtrSte[1]->CurPulseCnt %=pMtrSte[1]->PulsePerRound; 
		if(pMtrSte[1]->CurPulseCnt <0)
			pMtrSte[1]->CurPulseCnt +=pMtrSte[1]->PulsePerRound; 	  
		if(pMtrSte[1]->CurPulseCnt%(pMtrSte[1]->DestPulseCnt/pMtrSte[1]->DestSamplePos)==0)          //求出动态的样位
			pMtrSte[1]->CurSamplePos = pMtrSte[1]->CurPulseCnt/(pMtrSte[1]->DestPulseCnt/(pMtrSte[1]->DestSamplePos-1))+1;

		/////////////////////////////////////////////
		//样盘经过1号位时自动复位
		if((PreResetIn==0)&&
			(*pMtrSte[1]->ResetIn==1)&&
			(pMtrSte[1]->bDir==StepMotorCtrl::FORWARD)
		) //正向旋转,上升沿,脉冲数目复位
		{
			pMtrSte[1]->CurSamplePos 	= 1;		  
			pMtrSte[1]->CurPulseCnt		= 0;   
		}
		PreResetIn = *pMtrSte[1]->ResetIn; 
		//////////////////////////////////////
		if(pMtrSte[1]->CurPulseCnt==pMtrSte[1]->DestPulseCnt)				//设定到位中断次数;
//			||((pMtrSte[1]->DestSamplePos==1)&&(*pMtrSte[1]->ResetIn==1))   //或者目标为1号位时抵达1号
			
		{
			//停止IO口翻转		
			TIM_OCInitStructure.TIM_Pulse 	= TIM4->CCR1;//CCR3_Val;						    
			TIM_OC1Init(TIM4, &TIM_OCInitStructure);
			TIM_ITConfig(TIM4,  TIM_IT_CC1, DISABLE);
			pMtrSte[1]->bRun	= 0;
			__nop();													 
			pMtrSte[1]->CurSamplePos 	= pMtrSte[1]->DestSamplePos;
		}
	} 
}
//////////////////////////////////////////////////////////////////////////////////////////////
void TaskBlinLed(void)
{ 
	static unsigned int count =0;
	++count;			
	count=count%10;	
	PE15_O = count>0?1:0;		  	                     
}



enum COMMAND{CMD_NAK=0,            	//无命令
             CMD_SETID,	            //设置ID
             CMD_SETPW,	         	//设置密码
             CMD_LOGON,	          	//登录
             CMD_SETDATETIME,       //设置时间
             CMD_READDATETIME,     	//读取时间
             CMD_RESET_INST,       	//复位设备 
			 CMD_MOVEMOTOR,			//移动电机
             CMD_MOVEMOTORSAMPLENO, //移动样盘
             CMD_WARMUP ,
			 CMD_TARE, 				//清零
			 CMD_WEIGHT,			//称坩埚	  
			 CMD_START_TEST,		//启动测试
			 CMD_STOP_TEST 			//停止测试
			};
void TaskParsaCmd(void)
{
	uint32_t BaseAddress;
	LONG *destTempAD;
	int NO;
	int SW;
	uint8_t nID[4];
	Metage::METAGE_TYPE 		metage_type;
	Metage::METAGE_SPEED 		metage_speed;

	

	if(IedDb.Cmd.CmdType&0x80)
		return;
    switch(IedDb.Cmd.CmdType)
    {
            case CMD_NAK:
                __NOP();
                break;              //无命令
            case CMD_SETID:
				IedDb.cID = IedDb.Cmd.CmdParam1;
				ReadIdFromEE();
				if(IedDb.cID!= nID[0])
				{
					nID[0] =  IedDb.cID;
					EEWriteStruct(nID,1,BaseAddress);   //char 按long写入FLASH,虽然会写过界,但是不要紧
				 }
				__NOP();    
                break;              //设置ID
            case CMD_SETPW:
                __NOP();
                break;              //设置密码
            case CMD_LOGON:
                __NOP();
                break;              //登录
            case CMD_SETDATETIME:
                __NOP();
                break;              //设置时间
            case CMD_READDATETIME:
                __NOP();
                break;              //读取时间 
       	   case CMD_MOVEMOTOR:
		   		if(IedDb.Cmd.CmdParam2)
                {
                    if(IedDb.Cmd.CmdParam1==0)
                        LowTempStove.MoveStoveBar(IedDb.Cmd.CmdParam3);
                    else
                        m_HighTempStoveBar[IedDb.Cmd.CmdParam1].MoveStoveBar(IedDb.Cmd.CmdParam3);
                }
                else
                {
                    if(IedDb.Cmd.CmdParam1==0)
                        LowTempStove.StopStoveBar();
                    else
                        m_HighTempStoveBar[IedDb.Cmd.CmdParam1].StopStoveBar();
                }
                break;
            case CMD_MOVEMOTORSAMPLENO: //移动样品转盘       PARAM  目标样位
                __NOP();
                //MoveSamplePos(IED.Cmd.CmdParam1);  
				if(IedDb.Cmd.CmdParam2==0)
				{
					//停止步进电机
					//StepMotor[0].SetFreq(150);
					StepMotor[0].StopMotor();
				}
				else if(IedDb.Cmd.CmdParam1==0)
				{
					//连续转动						   
					StepMotor[0].MoveStepMotor(0) ;
				}
				else
				{
					StepMotor[0].MoveStepMotor(IedDb.Cmd.CmdParam1);
				} 
                __NOP();
				break;
            case CMD_WARMUP:
                __NOP();							     
				   
				NO = IedDb.Cmd.CmdParam1;
				SW = IedDb.Cmd.CmdParam2; 
				destTempAD 		= &IedDb.DownloadWorkParam.m_sWaterTestTemp;	 //目标温度AD地址
				destTempAD[NO]	= IedDb.Cmd.CmdParam;							 //目标温度	
				IedDb.fuzzy[NO].CtrlStove(SW, &destTempAD[NO]);
                __NOP();
                break;              //温度升温            param1 1 0		    
			case CMD_TARE:
				MTBalan.ZeroWeight();
				break;
			case CMD_WEIGHT:		//称坩埚
				metage_type   = (Metage::METAGE_TYPE ) IedDb.Cmd.CmdParam1;
				metage_speed  = (Metage::METAGE_SPEED )IedDb.Cmd.CmdParam2;	//快速/一样一称 称样品
				metage.StartMetage(metage_type,metage_speed);
				__NOP();
				break;		
			case CMD_START_TEST:		//启动测试
				IedDb.DownloadWorkParam.m_szalgorithm = IedDb.Cmd.CmdParam1;
				test.StartTest();
				__NOP();
				break;
            case CMD_RESET_INST:	 
			case CMD_STOP_TEST: 			//停止测试 
				test.StopTest();
				__NOP();
				break;
            default:
                __NOP();
                if(IedDb.Cmd.CmdType&0x80)
                {
                    __NOP();
                    #ifdef _DEBUG
                        //printf("命令已经执行\n");
                    #endif   
                }
                else
                {
                    __NOP();
                    #ifdef _DEBUG
                        printf("命令下载错误,不予执行");
                    #endif
                }
                break;
    }
    IedDb.Cmd.CmdType |=0x80;
}	 

void CtrlOxygenValve(bool bEnable)
{
	IedDb.OutputCoil.sCoil16.OXYGEN = bEnable;
    UpdateOutPut();
}

void CtrlNitrogenValve(bool bEnable)
{
	IedDb.OutputCoil.sCoil16.NITROGEN = bEnable;
    UpdateOutPut();
}

void StopAllFan(void)
{
	IedDb.OutputCoil.sCoil16.WATER_STOVE_FAN	= 0;		//水分炉子交流风扇
	IedDb.OutputCoil.sCoil16.OXYGEN		        = 0;		//氧气开关
	IedDb.OutputCoil.sCoil16.NITROGEN		    = 0;		//氮气开关
	IedDb.OutputCoil.sCoil16.LEFT_STOVE_FAN		= 0;		//左炉风扇
	IedDb.OutputCoil.sCoil16.RIGHT_STOVE_FAN	= 0;		//右炉风扇	
	PB0_O=PB1_O=PB9_O = 0;	   								//恒温炉加热风扇输出口
	UpdateOutPut();
}

void EnableLeftFan(bool Enable)
{ 								  
	IedDb.OutputCoil.sCoil16.LEFT_STOVE_FAN		= Enable;		//左炉氧气开关		
    UpdateOutPut();    
}
void EnableRightFan(bool Enable)
{								 																					 
	IedDb.OutputCoil.sCoil16.RIGHT_STOVE_FAN	= Enable;		//右炉风扇				  
	UpdateOutPut();
}
void EnableWaterFan(bool Enable)
{

	IedDb.OutputCoil.sCoil16.WATER_STOVE_FAN	= Enable;		//水分炉子交流风扇		
	UpdateOutPut();
	PB0_O=PB1_O=PB9_O = Enable;	   								//恒温炉加热风扇输出口
}




template<typename TBase>
void VertStoveBar<TBase>::ReadOutputInfo(void)
{
    IedDb.OutputCoil.iCoil16 = GPIOC->ODR;
}

template<typename TBase>
int VertStoveBar<TBase>::MoveStoveBar(unsigned int pos)
{
	//  样盘正在转动,不能上升								    
	StepMotorCtrl::StepMotorState *MtrState = &IedDb.MtrState;
    
	if(((MtrState->bRun!=0)		//样盘未对孔或者正在转动,不允许上升
		||(MtrState->DestPulseCnt!=MtrState->CurPulseCnt))
		&&(pos!=POS_BOT))
		return -2;
    //BUG............样盘升降电机只能在不对位的情况下只能上升,不能下降
    
	if(m_StovePos==pos)  //已经到位,不允许移动
	{
		#ifdef _DEBUG
		printf("已经到位,不需要再移动!\n");
		#endif
		return -1; 
	}	
	switch(pos)
	{
		case POS_TOP:
			bDirUp 	= true;
			bRun	= true;	
			break;
		case POS_BOT:
			t_RunTimer +=500;		  //5s
			bDirUp	= false;
			bRun	= true;
			break;
		case POS_UNKNOWN:
			bRun	= false;	   
			break;				
	}								  
	return 0;
}	

	

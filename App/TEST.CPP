#include "TEST.H"
#include "Task.H"
#include "CTRL.H"
#include "balance.h"
#include "KEY.H"

 

void ResetSystem(void)
{ 
    LowTempStove.SetMinRunTimer();
    LeftHighTempStove.SetMaxRunTimer();
    RightHighTempStove.SetMaxRunTimer(); 
    IedDb.ResetState    = REST_START;	
    IedDb.MetageState   = Metage::M_INIT;
    ActiveTask(TASK_RESET,100L); 	//系统复位
} 

void TaskReset(void)
{ 

	switch(IedDb.ResetState)
	{
		case REST_START:								 
			SuspendTask(TASK_MANUAL);		  	////停止手动进位任务
			CtrlOxygenValve(false);				//关闭气阀
            CtrlNitrogenValve(false);
			StopAllFan(); 
			StepMotor[0].StopMotor();
			LowTempStove.StopStoveBar();
			LeftHighTempStove.StopStoveBar();
			RightHighTempStove.StopStoveBar();
			////////////////////////////////////////////////////////////
			//将左右炉移动至底位,样盘上升至称样位																		 
			LowTempStove.MoveStoveBar(LowVertStoveBar::POS_TOP);
			LeftHighTempStove.MoveStoveBar(HighVertStoveBar::POS_BOT);
			RightHighTempStove.MoveStoveBar(HighVertStoveBar::POS_BOT);
			//
		   	IedDb.ResetState = REST_SAMPLEDISH;
			break;
		case REST_SAMPLEDISH:
			if( //中间炉子到位
                //(LowTempStove.m_StovePos==VertStoveBar::POS_TOP)
                (LowTempStove.GetTopOpto() == 0)
                &&(LowTempStove.GetBotOpto() == 1))
                {
                    if((IedDb.DownloadWorkParam.m_szSystemType==G5000)
                        ||((LeftHighTempStove.m_StovePos==HighVertStoveBar::POS_BOT)
                    &&(RightHighTempStove.m_StovePos==HighVertStoveBar::POS_BOT)))
                    {
                        StepMotor[0].MoveStepMotor(1);
                        IedDb.ResetState = REST_END;
                    } 
				}
			break;
		case REST_END:
			if(IedDb.MtrState.CurSamplePos==1)
			{
				SuspendTask(TASK_RESET);		
	            ActiveTask(TASK_MANUAL,20L);       //启动手动进位
			}
			break;
		default:
			IedDb.ResetState = REST_START;
			break;
	}
}
					 
/**************************************************************************
    任务：称坩埚,称样
***************************************************************************/

char Metage::TARE_DELAY 	= 30;          //s,tare delay
char Metage::BALANCE_ZERO 	= 2;		   //0.2mg
char Metage::M_DELAY 		= 6;		   //称量延时

char Metage::METAGE_MAXERR	= 3L;          //0.3mg 判称量稳定极差
char Metage::TARE_MAXERR    = 1L;          //0.1mg 判去皮稳定极差

Metage metage;
void Metage::TaskMetage(void)
{   
	long l;
	IED_DATABASE::YP_DATA *Sample = &IedDb.Sample1[0];	   
	MetagePtr = &Sample[m_cPos-1].UQuality.Q[metagType];
		
	int MetageBit = MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lMPot?S_WEIGHT_POT:  //置当前状态:正在称坩埚或称样
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lM0?S_WEIGHT_SAMPLE:
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lM1?S_TEST_WATER:
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lMPotSample2?S_WEIGHT_COVER:
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lM2?S_TEST_VOL:
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lM3?S_TEST_ASH:S_TEST_ASH+1;

	gcKey=GetKey();
	
    switch(*m_cSchedule)
    {
    case M_INIT:    //初始化
		ResetSystem();
        m_cMDelay=0;   
        m_cPos=1;
        *m_cSchedule=M_POS;
        break;
 
    case M_POS:     //移动炉子到称样位
		SuspendTask(TASK_MANUAL);       //停止手动进位
        if((m_cPos==IedDb.MtrState.CurSamplePos) &&
			(0==IedDb.MtrState.bRun)&& 
			(LowTempStove.m_StovePos==LowVertStoveBar::POS_TOP)&&
            ((IedDb.DownloadWorkParam.m_szSystemType==G5000)||
			((LeftHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT)&&
			(RightHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT))))//判位置			
        {
            //if(IedDb.Sample1[m_cPos-1].cSta>S_NULL)   //有样
			if(Sample[m_cPos-1].cSta&(1<<MetageBit))
            { 
                m_cMDelay	= 0;
				m_cDelayCnt	= 0;
				MTBalan.ZeroWeight();		//清0
                *m_cSchedule=M_TARE;		
				//TODO:清除目标位置重量数据
            }
            else
            {                                //无样
                *m_cSchedule=M_NEXTPOS;
            }
        }
        break;	    
    case M_TARE:
        l=MTBalan.GetBalanceAvr();
        if((l>-BALANCE_ZERO && l<BALANCE_ZERO)||m_cDelayCnt>MAX_WEIGHTING_CNT)	 //除皮成功或者除皮次数超过规定次数
        {											  
			LowTempStove.MoveStoveBar(LowVertStoveBar::POS_BOT); //移动炉子到称样位置
			*m_cSchedule=M_METAGE;
			m_cDelayCnt = 0;
            m_cMDelay=0;
        }
        else if(m_cMDelay++>TARE_DELAY)	  //不稳定,再次清零
        { 						 
            BeepBalErr();               //除皮不稳定,提醒一次	
			MTBalan.ZeroWeight();		//清0
			m_cMDelay=0;
			m_cDelayCnt ++;
        }
        break;
		
    case M_METAGE:  //称量
        if((LowTempStove.m_StovePos==LOW_POS_BOT)&&
            ((IedDb.DownloadWorkParam.m_szSystemType==G5000)||
			((LeftHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT)&&
			(RightHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT))))//判位置
        {		
			//称的不是样重,或者快速称样,或者确认,进入保存重量环节 	  	
            if(((metagType==T_SAMPLE)&&(gcKey==KEY_ENT))||(T_QUICK==metagSpeed)||(metagType!=T_SAMPLE))
			{
                *m_cSchedule=M_SAVE;		   
				if(KEY_ENT==gcKey)
				{ 
                    m_cMDelay = M_DELAY;
                    BeepOK();
				}
			}
			m_cMDelay ++;
			//TODO:增加称样时到达重量范围内的提示音
        }
        break;
	case M_SAVE:
	    if(m_cMDelay++>M_DELAY)              //延时M_DELAY后存数
		{
			m_cMDelay=M_DELAY-2;
			m_cDelayCnt++;
            if((MTBalan.BalanceMaxErr(METAGE_MAXERR))||m_cDelayCnt>MAX_WEIGHTING_CNT)		//增加称样超时
            {						
                *MetagePtr= MTBalan.GetBalanceAvr(); //存坩埚质量	
				if(m_cDelayCnt>MAX_WEIGHTING_CNT)	
				{   
                    BeepBalErr(); 
					//*MetagePtr= IedDb.buf.CurrWeight; //存坩埚不稳定质量
				}					    
								
				LowTempStove.MoveStoveBar(LOW_POS_BOT); //移动炉子到放样位置	  
                m_cDelayCnt = 0;       	

				setbit(Sample[m_cPos-1].cCSta,MetageBit);		 
				*m_cSchedule=M_NEXTPOS;
			}
        }
		break;	
    case M_NEXTPOS: //转到下一个样位
      if((LowTempStove.m_StovePos==LOW_POS_TOP)&&
            ((IedDb.DownloadWorkParam.m_szSystemType==G5000)||
			((LeftHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT)&&
			(RightHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT))))//判位置
        {										 
			while(m_cPos<*MAX_SAMPLE)
            {										
                //有坩埚,该位有上位机下载下来,只要有一个实验项目就会有	    					  
                // 是否称量该重量,由是否做该实验决定	  
				if(Sample[m_cPos].cSta&(1<<MetageBit))
                {
					m_cPos++;
					m_cMDelay = 0;
                    StepMotor[0].MoveStepMotor(m_cPos);     //转到下个样位	   
                    *m_cSchedule=M_POS;
                    return;
                }
				m_cPos++;
            }
			
            StepMotor[0].MoveStepMotor(1);           //转到1st样位
            *m_cSchedule=M_END;
			__NOP();
        }
        break;
		
    case M_END:
        m_cPos=1;                    //完成,复位 
        SuspendTask(TASK_METAGE);                   //停止“称量”任务
        ActiveTask(TASK_MANUAL,100L);               //启动手动进位
        beep.beep(2,1000L);                         //称量完毕,提示音            
        break;	 
    }	  
}

void Metage::ClearMetageFlag(void)
{
    
	IED_DATABASE::YP_DATA *Sample = &IedDb.Sample1[0];	 
    int samplecnt  = *MAX_SAMPLE;
    if(samplecnt>24)
       samplecnt = 24 ;
    for(m_cPos=0;m_cPos<samplecnt;m_cPos++)
    {
        Sample[m_cPos].cCSta = 0;
        
        for(int i=0;i<6;i++)
        {
            Sample[m_cPos].UQuality.Q[i] = 0;
        }
        
    }
} 
void Metage::StartMetage(METAGE_TYPE type,METAGE_SPEED speed)
{
	metagType 	= type;
	metagSpeed  = speed;
	
    if(type==T_POT)
    {
        ClearMetageFlag ();
        IedDb.TestState = Test::T_INIT;
    }
	m_cSchedule = &IedDb.MetageState;
	*m_cSchedule = M_INIT;
	MAX_SAMPLE	= &IedDb.DownloadWorkParam.m_btMaxSampleCnt;
	ActiveTask(TASK_METAGE,1000L);
	
}
void TaskMetage(void)
{
	metage.TaskMetage(); 
}	 

#define KEY_PIN PD3_I
void TaskManualKey(void)
{
    gcKey=GetKey();
    if(gcKey!=0)
    {
        __nop();
    }
	if(gcKey==KEY_ENT)
	{
		if(IedDb.MtrState.CurSamplePos==0)
			StepMotor[0].MoveStepMotor(1);		  
		else
			StepMotor[0].MoveStepMotor(IedDb.MtrState.CurSamplePos+2);		  
	}			    
}
																						  
/***************************************************************************************/

Test test;


#define RESET_POSITON ((IedDb.MtrState.CurSamplePos==m_Pos)&&\
                (IedDb.MtrState.bRun==0)&&\
                (LowTempStove.m_StovePos==LowVertStoveBar::POS_TOP)&&\
				(LeftHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT)&&\
				(RightHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT))


void TaskTest()
{
	test.TaskTest(); 
}

void Test::TaskTest(void)
{
	switch(IedDb.DownloadWorkParam.m_szalgorithm)
	{
		case IED_DATABASE::DownParam::M_GB:
			TaskTestGB();
			break;
		case IED_DATABASE::DownParam::M_QK:
			TaskTestQK();
			break;
		default:
			break;
	}
	
	
}

void Test::StartTest(void)
{
	IedDb.TestState = T_INIT;
	SuspendTask(TASK_MANUAL);
	SuspendTask(TASK_METAGE);
	ActiveTask(TASK_TEST,PRIOD_TEST);
}

void Test::StopTest(void)
{
	SuspendTask(TASK_TEST);
	SuspendTask(TASK_METAGE);
	StepMotor[0].StopMotor();
	StopAllFan();
	for(int i=0;i<FUZZY_ARRAY_SIZE;i++)	
	{
		IedDb.fuzzy[i].StopStove();
	}
	IedDb.TestState	= T_INIT;
	ResetSystem();
}

int Test::GetWillBeTestedType(void)
{
    int type =0;    
    IED_DATABASE::YP_DATA *Sample = &IedDb.Sample1[0];	
    int TestType[] = {S_TEST_WATER,S_TEST_VOL,S_TEST_ASH};
    for(int j=0;j<3;j++)
    { 
        //完成项目被跳过
        //
        bool bBreak = FALSE;
        for(int i=0;i<IedDb.DownloadWorkParam.m_btMaxSampleCnt;i++)
        {
            if(chkbit(Sample[i].cCSta,TestType[j]))           //     
            {
                bBreak = TRUE;
                break;
            }
        }
        if(bBreak==TRUE)
            continue;
        
        //样品测试项目计算,如果有一个需要测试该项目,就测试该项目
        for(int cnt=0;cnt<IedDb.DownloadWorkParam.m_btMaxSampleCnt;cnt++) 
        {
            if(Sample[cnt].cItem&(1L<<j))
                return TestType[j];
        }   
    }
    return type;
}

void Test::TaskTestQK(void)
{
}

void Test::TaskTestGB(void)
{
    char i;
    long l;
	CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	//升温到105度 	;
	long	 	*ADValue = IedDb.ADValue.ad ; 
	SuspendTask(TASK_MANUAL);	 
	
    switch(IedDb.TestState)
    {

		case T_INIT:					     //初始化
			SuspendTask(TASK_MANUAL);       //停止手动进位 
            ResetSystem();
			m_MDelay 		= 0;
			m_Pos			= 1;
			m_TestTimer		= -1;
			//增加条件判断,是否做该实验
            switch(GetWillBeTestedType())
            {
                case S_TEST_WATER:
                    IedDb.TestState = T_WARM_WATER;	
                    break;
                case S_TEST_VOL:
                    IedDb.TestState = T_WARM_HIGH1;	
                    break;
                case S_TEST_ASH:
                    IedDb.TestState = T_WARM_HIGH2;	
                    break;
                default:
                    IedDb.TestState = T_END;	
                    break;
            } 
			break;
        case T_WARM_WATER:
        case T_TEST_WATER:            
        case T_WEIGHT_WATER:
        case T_WATERCOOL:
        case T_WATER_END:
            TestWater();
            break;
        case T_WARM_HIGH1:
        case T_TEST_VOL:
        case T_BURN_VOL:
        case T_WEIGHT_VOL:
        case T_VOL_END:
            TestVol();
            break;
        case T_WARM_HIGH2:
        case T_TEST_ASH:
        case T_BURN_ASH:
        case T_WEIGHT_ASH:
        case T_ASH_END:
            TestAsh();
            break;
        case T_END:
            __NOP();
            break;
        default:
            ASSERT(false);
            break;
        
    }	
}

void Test::TestWater(void)
{
    
	CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	//升温到105度 	;
	long	 	*ADValue = IedDb.ADValue.ad ; 
    switch(IedDb.TestState)
    {
		case T_WARM_WATER:
			if(RESET_POSITON)
			{						
                //不停旋转样盘	    
                StepMotor[0].MoveStepMotor(0) ;	
                EnableWaterFan(true);
                pFuzzy[WATER_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//开始水分升温									
                pFuzzy[LEFT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//开启高温炉辅助升温									
                pFuzzy[RIGHT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//开启高温炉辅助升温								
                IedDb.TestState = T_TEST_WATER;
                m_MDelay = 0;
			}
            else
            { //每秒叫一次,一次50ms 提醒样盘不能移动错误
                BeepMtrErr();
            }
			break;															
		case T_TEST_WATER:
			//如果温度达到105度													 //1度
			if((ADValue[WATER_STOVE] >= (IedDb.DownloadWorkParam.m_sWaterTestTemp-KC*10))  //目标温度AD地址
			&&(m_MDelay++>IedDb.DownloadWorkParam.m_sWaterTestTime))	 				   //计算时间达到
			{
                pFuzzy[LEFT_STOVE].CtrlStove(false, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//关闭高温炉辅助升温									
                pFuzzy[RIGHT_STOVE].CtrlStove(false, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//关闭高温炉辅助升温								
				pFuzzy[WATER_STOVE].CtrlStove(false, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//停止水分升温
				                
				IedDb.TestState = T_WATERCOOL; 
                m_MDelay = IedDb.DownloadWorkParam.m_sWaterCoolTime;
			}
			break;  
        case T_WATERCOOL:
            
            if(m_MDelay--==0) { //冷却时间到
                StopAllFan();
                StepMotor[0].StopMotor();	    //停止转动	   
                metage.StartMetage(Metage::T_WATER,Metage::T_NORMAL);	//称水分
                IedDb.TestState = T_WEIGHT_WATER; 
            }
            break;
		case T_WEIGHT_WATER:    //水分测试完毕
			if(IedDb.MetageState==Metage::M_END)//称量水分完成
			{   //称样完毕                
                IedDb.TestState = T_WATER_END; 
                //LowTempStove.MoveStoveBar(VertStoveBar::POS_TOP); //移动炉子到放样位置	  
               
                __NOP();
			}
			break;
		case T_WATER_END:
            IedDb.TestState = T_INIT; 
			break;
        default:
            ASSERT(false);
            break;
    }
}
void Test::StartCool(void)
{
    
}

void Test::StopCool(void)
{
    
}

void Test::TestVol(void)
{    
	CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	//升温到105度 	;
	long	 	*ADValue = IedDb.ADValue.ad ; 
    switch(IedDb.TestState)
    {
        case T_WARM_HIGH1: 
            {	
                //TODO:判断是否需要升温,分别升温
                pFuzzy[LEFT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sVolTestTemp);//开启高温炉升温									                    
                pFuzzy[RIGHT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sVolTestTemp);//开启高温炉升温								
                IedDb.TestState = T_TEST_VOL;
            }	
            break;
        case T_TEST_VOL	:
            
			//如果温度达到920度													 //1度
			if((ADValue[LEFT_STOVE] >= (IedDb.DownloadWorkParam.m_sVolTestTemp-KC*10))&&  //目标温度AD地址
                (ADValue[RIGHT_STOVE] >= (IedDb.DownloadWorkParam.m_sVolTestTemp-KC*10)))  //目标温度AD地址
			//&&(++m_MDelay>IedDb.DownloadWorkParam.m_sGB_WaterTestTime))	 	
            { 
                IedDb.TestState = T_BURN_VOL;
                BurnSample(D_VOL);
            }
            __NOP();
            break;
        case T_BURN_VOL:            
            if(m_burnState==B_END)//灼烧完毕
            { 
                StopAllFan();
                metage.StartMetage(Metage::T_VOL,Metage::T_NORMAL);	
                IedDb.TestState = T_WEIGHT_VOL;
            }
            __NOP();
            break;
        case T_WEIGHT_VOL:
            if(IedDb.MetageState==Metage::M_END)
            { 
                __NOP();
                IedDb.TestState = T_VOL_END;
            }
           break;
        case T_VOL_END:
            IedDb.TestState = T_INIT;
            __NOP();
            break;
        default:
            ASSERT(false);
    }
}


void Test::TestAsh(void)
{
    
	CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	//升温到105度 	;
	long	 	*ADValue = IedDb.ADValue.ad ; 
    switch(IedDb.TestState)
    {

        case T_WARM_HIGH2:
            //TODO:判断是否需要升温,分别升温
            pFuzzy[LEFT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sAshTestTemp);//开启高温炉升温									                    
            pFuzzy[RIGHT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sAshTestTemp);//开启高温炉升温								                
            IedDb.TestState = T_TEST_ASH;
            __NOP();
            break;
        case T_TEST_ASH:
            if((ADValue[LEFT_STOVE] >= (IedDb.DownloadWorkParam.m_sAshTestTemp-KC*10))&&  //目标温度AD地址
                (ADValue[RIGHT_STOVE] >= (IedDb.DownloadWorkParam.m_sAshTestTemp-KC*10)))  //目标温度AD地址
			//&&(++m_MDelay>IedDb.DownloadWorkParam.m_sGB_WaterTestTime))	 	
            { 
                IedDb.TestState = T_BURN_ASH;
                CtrlOxygenValve(true); 
                    
                EnableLeftFan(true);
                EnableRightFan(true);
                BurnSample(D_ASH);
            }
            __NOP();
            break;
        case T_BURN_ASH:
            if(m_burnState==B_END)//灼烧完毕
            {
                StopAllFan();
                metage.StartMetage(Metage::T_ASH,Metage::T_NORMAL);	
                IedDb.TestState = T_WEIGHT_ASH;
            } 
            break;
        case T_WEIGHT_ASH:            
            if(IedDb.MetageState==Metage::M_END)
            { 
                __NOP();
                IedDb.TestState = T_ASH_END;
            }
            __NOP();
            break;
        case T_ASH_END	:
            __NOP();            
            IedDb.TestState = T_INIT;
            break;
        default:
            ASSERT(false);
            break;
    }
}

void Test::BurnSample(BURN_SAMPLE_TYPE sampleType)
{    
    m_burnState     = B_INIT  ;    //烧样品状态;
    m_burnType      = sampleType;     //烧样类型
    ActiveTask(TASK_BURN,1000L);
}


    
void Test::TaskBurnSample(void)
{    
    IED_DATABASE::YP_DATA *Sample = &IedDb.Sample1[0];	
    CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	
    long	 	*ADValue = IedDb.ADValue.ad ;	
    //检查下两个工位
    #define CheckBurnEnd() {\
    if(m_Pos<MaxSampleCnt/2-2)\
        {m_Pos+=2;m_burnState = B_CHECK;}\
    else\
        {\
            m_burnState = B_COOL;\
            StepMotor[0].MoveStepMotor(0);\
            EnableLeftFan(true);\
            EnableRightFan(true);\
            m_MDelay = IedDb.DownloadWorkParam.m_sHighTempCoolTime;\
        }\
    }     
   
    switch(m_burnState)
    {
        case B_INIT:
            //ResetSystem();
            m_burnState = B_CHECK; 
            if(m_burnType==D_VOL)     //烧样类型)
            { 
                EnableLeftFan(false) ;
                EnableRightFan(false);                
            }
            else
            { 
                EnableLeftFan(true) ;
                EnableRightFan(true);
            }
         
            break;
        case B_CHECK:
            //m_burnType
            //检查左炉下面的样品是否要做实验?
            if(Sample[m_Pos-1].cItem&(1L<<m_burnType)||
                Sample[m_Pos].cItem&(1L<<m_burnType)||
            
                Sample[m_Pos-1+MaxSampleCnt/2].cItem&(1L<<m_burnType)||
                Sample[m_Pos+MaxSampleCnt/2].cItem&(1L<<m_burnType))
            //检查右炉下面的样品是否要做实验?
            {
                
                if(RightHighTempStove.isRun()||
                    LeftHighTempStove.isRun())
                    break;      //等待样杆移动完成
                //移动到炉子下方做实验
                if(!StepMotor[0].MoveStepMotor(m_Pos+BURN_SAMPLE_OFFSET))
                {                    
                    m_burnState = B_POS; //转动正常,进入等待到位,
                    switch(m_burnType)
                    {
                        case D_WATER://水分干燥时间(分);
                            m_BurnTime  = IedDb.DownloadWorkParam.m_sWaterTestTime;
                            ASSERT(FALSE);  //做水分不能调用这个任务
                            break;
                        case D_VOL://挥发分干燥时间(分);
                            m_BurnTime  = IedDb.DownloadWorkParam.m_sVolTestTime;
                            break;
                        case D_ASH://灰分干燥时间(分);
                            m_BurnTime  = IedDb.DownloadWorkParam.m_sAshTestTime;
                            break;
                        case D_POT:
                            m_BurnTime  = 300; //5分钟
                            break; 
                    }
                }
                else
                {
                    //每秒叫一次,一次50ms 提醒样盘不能移动错误
                    BeepMtrErr();
                }
            }
            else
            {
                //该位置4个坩埚均不做实验,检查下一批坩埚,并判断是否结束
                CheckBurnEnd();
            }
            break;
        case B_POS:
            if(!IedDb.MtrState.bRun)  
            {   // 灼烧坩埚流程 
                if(Sample[m_Pos-1].cItem&(1L<<m_burnType)||
                    Sample[m_Pos].cItem&(1L<<m_burnType))
                {
                    //送左送样干
                    LeftHighTempStove.MoveStoveBar(HIGH_POS_TOP);
                }
                if(Sample[m_Pos-1+MaxSampleCnt/2].cItem&(1L<<m_burnType)||
                    Sample[m_Pos+MaxSampleCnt/2].cItem&(1L<<m_burnType))
                {
                    RightHighTempStove.MoveStoveBar(HIGH_POS_TOP);
                }
                //然后开始计时
                m_burnState = B_BURN;
            }
            __NOP();
            break;
        case B_BURN:
            if((!RightHighTempStove.isRun())
                &&((!LeftHighTempStove.isRun())))
            {
                if(m_BurnTime--==0)  //灼烧时间到
                {
                    LeftHighTempStove.MoveStoveBar(HIGH_POS_BOT);
                    RightHighTempStove.MoveStoveBar(HIGH_POS_BOT); 
                    CheckBurnEnd();
                }
            }
            break;
        case B_COOL:
            pFuzzy[LEFT_STOVE].CtrlStove(false, &IedDb.DownloadWorkParam.m_sVolTestTemp);							                    
            pFuzzy[RIGHT_STOVE].CtrlStove(false, &IedDb.DownloadWorkParam.m_sVolTestTemp); 
                
            if(m_MDelay--==0) { //冷却时间到  
                m_burnState = B_END;  //冷却,然后称量
            }
            break;
        case B_END:
            __NOP();
            break;
        default:
            ASSERT(FALSE);
            break;
    }
}



void TaskBurnSample(void)
{
    test.TaskBurnSample(); 
}





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
    ActiveTask(TASK_RESET,100L); 	//ϵͳ��λ
} 

void TaskReset(void)
{ 

	switch(IedDb.ResetState)
	{
		case REST_START:								 
			SuspendTask(TASK_MANUAL);		  	////ֹͣ�ֶ���λ����
			CtrlOxygenValve(false);				//�ر�����
            CtrlNitrogenValve(false);
			StopAllFan(); 
			StepMotor[0].StopMotor();
			LowTempStove.StopStoveBar();
			LeftHighTempStove.StopStoveBar();
			RightHighTempStove.StopStoveBar();
			////////////////////////////////////////////////////////////
			//������¯�ƶ�����λ,��������������λ																		 
			LowTempStove.MoveStoveBar(LowVertStoveBar::POS_TOP);
			LeftHighTempStove.MoveStoveBar(HighVertStoveBar::POS_BOT);
			RightHighTempStove.MoveStoveBar(HighVertStoveBar::POS_BOT);
			//
		   	IedDb.ResetState = REST_SAMPLEDISH;
			break;
		case REST_SAMPLEDISH:
			if( //�м�¯�ӵ�λ
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
	            ActiveTask(TASK_MANUAL,20L);       //�����ֶ���λ
			}
			break;
		default:
			IedDb.ResetState = REST_START;
			break;
	}
}
					 
/**************************************************************************
    ���񣺳�����,����
***************************************************************************/

char Metage::TARE_DELAY 	= 30;          //s,tare delay
char Metage::BALANCE_ZERO 	= 2;		   //0.2mg
char Metage::M_DELAY 		= 6;		   //������ʱ

char Metage::METAGE_MAXERR	= 3L;          //0.3mg �г����ȶ�����
char Metage::TARE_MAXERR    = 1L;          //0.1mg ��ȥƤ�ȶ�����

Metage metage;
void Metage::TaskMetage(void)
{   
	long l;
	IED_DATABASE::YP_DATA *Sample = &IedDb.Sample1[0];	   
	MetagePtr = &Sample[m_cPos-1].UQuality.Q[metagType];
		
	int MetageBit = MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lMPot?S_WEIGHT_POT:  //�õ�ǰ״̬:���ڳ����������
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lM0?S_WEIGHT_SAMPLE:
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lM1?S_TEST_WATER:
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lMPotSample2?S_WEIGHT_COVER:
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lM2?S_TEST_VOL:
					MetagePtr==&Sample[m_cPos-1].UQuality.SQuality.lM3?S_TEST_ASH:S_TEST_ASH+1;

	gcKey=GetKey();
	
    switch(*m_cSchedule)
    {
    case M_INIT:    //��ʼ��
		ResetSystem();
        m_cMDelay=0;   
        m_cPos=1;
        *m_cSchedule=M_POS;
        break;
 
    case M_POS:     //�ƶ�¯�ӵ�����λ
		SuspendTask(TASK_MANUAL);       //ֹͣ�ֶ���λ
        if((m_cPos==IedDb.MtrState.CurSamplePos) &&
			(0==IedDb.MtrState.bRun)&& 
			(LowTempStove.m_StovePos==LowVertStoveBar::POS_TOP)&&
            ((IedDb.DownloadWorkParam.m_szSystemType==G5000)||
			((LeftHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT)&&
			(RightHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT))))//��λ��			
        {
            //if(IedDb.Sample1[m_cPos-1].cSta>S_NULL)   //����
			if(Sample[m_cPos-1].cSta&(1<<MetageBit))
            { 
                m_cMDelay	= 0;
				m_cDelayCnt	= 0;
				MTBalan.ZeroWeight();		//��0
                *m_cSchedule=M_TARE;		
				//TODO:���Ŀ��λ����������
            }
            else
            {                                //����
                *m_cSchedule=M_NEXTPOS;
            }
        }
        break;	    
    case M_TARE:
        l=MTBalan.GetBalanceAvr();
        if((l>-BALANCE_ZERO && l<BALANCE_ZERO)||m_cDelayCnt>MAX_WEIGHTING_CNT)	 //��Ƥ�ɹ����߳�Ƥ���������涨����
        {											  
			LowTempStove.MoveStoveBar(LowVertStoveBar::POS_BOT); //�ƶ�¯�ӵ�����λ��
			*m_cSchedule=M_METAGE;
			m_cDelayCnt = 0;
            m_cMDelay=0;
        }
        else if(m_cMDelay++>TARE_DELAY)	  //���ȶ�,�ٴ�����
        { 						 
            BeepBalErr();               //��Ƥ���ȶ�,����һ��	
			MTBalan.ZeroWeight();		//��0
			m_cMDelay=0;
			m_cDelayCnt ++;
        }
        break;
		
    case M_METAGE:  //����
        if((LowTempStove.m_StovePos==LOW_POS_BOT)&&
            ((IedDb.DownloadWorkParam.m_szSystemType==G5000)||
			((LeftHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT)&&
			(RightHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT))))//��λ��
        {		
			//�ƵĲ�������,���߿��ٳ���,����ȷ��,���뱣���������� 	  	
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
			//TODO:���ӳ���ʱ����������Χ�ڵ���ʾ��
        }
        break;
	case M_SAVE:
	    if(m_cMDelay++>M_DELAY)              //��ʱM_DELAY�����
		{
			m_cMDelay=M_DELAY-2;
			m_cDelayCnt++;
            if((MTBalan.BalanceMaxErr(METAGE_MAXERR))||m_cDelayCnt>MAX_WEIGHTING_CNT)		//���ӳ�����ʱ
            {						
                *MetagePtr= MTBalan.GetBalanceAvr(); //����������	
				if(m_cDelayCnt>MAX_WEIGHTING_CNT)	
				{   
                    BeepBalErr(); 
					//*MetagePtr= IedDb.buf.CurrWeight; //���������ȶ�����
				}					    
								
				LowTempStove.MoveStoveBar(LOW_POS_BOT); //�ƶ�¯�ӵ�����λ��	  
                m_cDelayCnt = 0;       	

				setbit(Sample[m_cPos-1].cCSta,MetageBit);		 
				*m_cSchedule=M_NEXTPOS;
			}
        }
		break;	
    case M_NEXTPOS: //ת����һ����λ
      if((LowTempStove.m_StovePos==LOW_POS_TOP)&&
            ((IedDb.DownloadWorkParam.m_szSystemType==G5000)||
			((LeftHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT)&&
			(RightHighTempStove.m_StovePos==LowVertStoveBar::POS_BOT))))//��λ��
        {										 
			while(m_cPos<*MAX_SAMPLE)
            {										
                //������,��λ����λ����������,ֻҪ��һ��ʵ����Ŀ�ͻ���	    					  
                // �Ƿ����������,���Ƿ�����ʵ�����	  
				if(Sample[m_cPos].cSta&(1<<MetageBit))
                {
					m_cPos++;
					m_cMDelay = 0;
                    StepMotor[0].MoveStepMotor(m_cPos);     //ת���¸���λ	   
                    *m_cSchedule=M_POS;
                    return;
                }
				m_cPos++;
            }
			
            StepMotor[0].MoveStepMotor(1);           //ת��1st��λ
            *m_cSchedule=M_END;
			__NOP();
        }
        break;
		
    case M_END:
        m_cPos=1;                    //���,��λ 
        SuspendTask(TASK_METAGE);                   //ֹͣ������������
        ActiveTask(TASK_MANUAL,100L);               //�����ֶ���λ
        beep.beep(2,1000L);                         //�������,��ʾ��            
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
        //�����Ŀ������
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
        
        //��Ʒ������Ŀ����,�����һ����Ҫ���Ը���Ŀ,�Ͳ��Ը���Ŀ
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
	CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	//���µ�105�� 	;
	long	 	*ADValue = IedDb.ADValue.ad ; 
	SuspendTask(TASK_MANUAL);	 
	
    switch(IedDb.TestState)
    {

		case T_INIT:					     //��ʼ��
			SuspendTask(TASK_MANUAL);       //ֹͣ�ֶ���λ 
            ResetSystem();
			m_MDelay 		= 0;
			m_Pos			= 1;
			m_TestTimer		= -1;
			//���������ж�,�Ƿ�����ʵ��
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
    
	CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	//���µ�105�� 	;
	long	 	*ADValue = IedDb.ADValue.ad ; 
    switch(IedDb.TestState)
    {
		case T_WARM_WATER:
			if(RESET_POSITON)
			{						
                //��ͣ��ת����	    
                StepMotor[0].MoveStepMotor(0) ;	
                EnableWaterFan(true);
                pFuzzy[WATER_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//��ʼˮ������									
                pFuzzy[LEFT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//��������¯��������									
                pFuzzy[RIGHT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//��������¯��������								
                IedDb.TestState = T_TEST_WATER;
                m_MDelay = 0;
			}
            else
            { //ÿ���һ��,һ��50ms �������̲����ƶ�����
                BeepMtrErr();
            }
			break;															
		case T_TEST_WATER:
			//����¶ȴﵽ105��													 //1��
			if((ADValue[WATER_STOVE] >= (IedDb.DownloadWorkParam.m_sWaterTestTemp-KC*10))  //Ŀ���¶�AD��ַ
			&&(m_MDelay++>IedDb.DownloadWorkParam.m_sWaterTestTime))	 				   //����ʱ��ﵽ
			{
                pFuzzy[LEFT_STOVE].CtrlStove(false, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//�رո���¯��������									
                pFuzzy[RIGHT_STOVE].CtrlStove(false, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//�رո���¯��������								
				pFuzzy[WATER_STOVE].CtrlStove(false, &IedDb.DownloadWorkParam.m_sWaterTestTemp);//ֹͣˮ������
				                
				IedDb.TestState = T_WATERCOOL; 
                m_MDelay = IedDb.DownloadWorkParam.m_sWaterCoolTime;
			}
			break;  
        case T_WATERCOOL:
            
            if(m_MDelay--==0) { //��ȴʱ�䵽
                StopAllFan();
                StepMotor[0].StopMotor();	    //ֹͣת��	   
                metage.StartMetage(Metage::T_WATER,Metage::T_NORMAL);	//��ˮ��
                IedDb.TestState = T_WEIGHT_WATER; 
            }
            break;
		case T_WEIGHT_WATER:    //ˮ�ֲ������
			if(IedDb.MetageState==Metage::M_END)//����ˮ�����
			{   //�������                
                IedDb.TestState = T_WATER_END; 
                //LowTempStove.MoveStoveBar(VertStoveBar::POS_TOP); //�ƶ�¯�ӵ�����λ��	  
               
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
	CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	//���µ�105�� 	;
	long	 	*ADValue = IedDb.ADValue.ad ; 
    switch(IedDb.TestState)
    {
        case T_WARM_HIGH1: 
            {	
                //TODO:�ж��Ƿ���Ҫ����,�ֱ�����
                pFuzzy[LEFT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sVolTestTemp);//��������¯����									                    
                pFuzzy[RIGHT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sVolTestTemp);//��������¯����								
                IedDb.TestState = T_TEST_VOL;
            }	
            break;
        case T_TEST_VOL	:
            
			//����¶ȴﵽ920��													 //1��
			if((ADValue[LEFT_STOVE] >= (IedDb.DownloadWorkParam.m_sVolTestTemp-KC*10))&&  //Ŀ���¶�AD��ַ
                (ADValue[RIGHT_STOVE] >= (IedDb.DownloadWorkParam.m_sVolTestTemp-KC*10)))  //Ŀ���¶�AD��ַ
			//&&(++m_MDelay>IedDb.DownloadWorkParam.m_sGB_WaterTestTime))	 	
            { 
                IedDb.TestState = T_BURN_VOL;
                BurnSample(D_VOL);
            }
            __NOP();
            break;
        case T_BURN_VOL:            
            if(m_burnState==B_END)//�������
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
    
	CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	//���µ�105�� 	;
	long	 	*ADValue = IedDb.ADValue.ad ; 
    switch(IedDb.TestState)
    {

        case T_WARM_HIGH2:
            //TODO:�ж��Ƿ���Ҫ����,�ֱ�����
            pFuzzy[LEFT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sAshTestTemp);//��������¯����									                    
            pFuzzy[RIGHT_STOVE].CtrlStove(true, &IedDb.DownloadWorkParam.m_sAshTestTemp);//��������¯����								                
            IedDb.TestState = T_TEST_ASH;
            __NOP();
            break;
        case T_TEST_ASH:
            if((ADValue[LEFT_STOVE] >= (IedDb.DownloadWorkParam.m_sAshTestTemp-KC*10))&&  //Ŀ���¶�AD��ַ
                (ADValue[RIGHT_STOVE] >= (IedDb.DownloadWorkParam.m_sAshTestTemp-KC*10)))  //Ŀ���¶�AD��ַ
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
            if(m_burnState==B_END)//�������
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
    m_burnState     = B_INIT  ;    //����Ʒ״̬;
    m_burnType      = sampleType;     //��������
    ActiveTask(TASK_BURN,1000L);
}


    
void Test::TaskBurnSample(void)
{    
    IED_DATABASE::YP_DATA *Sample = &IedDb.Sample1[0];	
    CFuzzy 		*pFuzzy  = IedDb.fuzzy; 	
    long	 	*ADValue = IedDb.ADValue.ad ;	
    //�����������λ
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
            if(m_burnType==D_VOL)     //��������)
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
            //�����¯�������Ʒ�Ƿ�Ҫ��ʵ��?
            if(Sample[m_Pos-1].cItem&(1L<<m_burnType)||
                Sample[m_Pos].cItem&(1L<<m_burnType)||
            
                Sample[m_Pos-1+MaxSampleCnt/2].cItem&(1L<<m_burnType)||
                Sample[m_Pos+MaxSampleCnt/2].cItem&(1L<<m_burnType))
            //�����¯�������Ʒ�Ƿ�Ҫ��ʵ��?
            {
                
                if(RightHighTempStove.isRun()||
                    LeftHighTempStove.isRun())
                    break;      //�ȴ������ƶ����
                //�ƶ���¯���·���ʵ��
                if(!StepMotor[0].MoveStepMotor(m_Pos+BURN_SAMPLE_OFFSET))
                {                    
                    m_burnState = B_POS; //ת������,����ȴ���λ,
                    switch(m_burnType)
                    {
                        case D_WATER://ˮ�ָ���ʱ��(��);
                            m_BurnTime  = IedDb.DownloadWorkParam.m_sWaterTestTime;
                            ASSERT(FALSE);  //��ˮ�ֲ��ܵ����������
                            break;
                        case D_VOL://�ӷ��ָ���ʱ��(��);
                            m_BurnTime  = IedDb.DownloadWorkParam.m_sVolTestTime;
                            break;
                        case D_ASH://�ҷָ���ʱ��(��);
                            m_BurnTime  = IedDb.DownloadWorkParam.m_sAshTestTime;
                            break;
                        case D_POT:
                            m_BurnTime  = 300; //5����
                            break; 
                    }
                }
                else
                {
                    //ÿ���һ��,һ��50ms �������̲����ƶ�����
                    BeepMtrErr();
                }
            }
            else
            {
                //��λ��4������������ʵ��,�����һ������,���ж��Ƿ����
                CheckBurnEnd();
            }
            break;
        case B_POS:
            if(!IedDb.MtrState.bRun)  
            {   // ������������ 
                if(Sample[m_Pos-1].cItem&(1L<<m_burnType)||
                    Sample[m_Pos].cItem&(1L<<m_burnType))
                {
                    //����������
                    LeftHighTempStove.MoveStoveBar(HIGH_POS_TOP);
                }
                if(Sample[m_Pos-1+MaxSampleCnt/2].cItem&(1L<<m_burnType)||
                    Sample[m_Pos+MaxSampleCnt/2].cItem&(1L<<m_burnType))
                {
                    RightHighTempStove.MoveStoveBar(HIGH_POS_TOP);
                }
                //Ȼ��ʼ��ʱ
                m_burnState = B_BURN;
            }
            __NOP();
            break;
        case B_BURN:
            if((!RightHighTempStove.isRun())
                &&((!LeftHighTempStove.isRun())))
            {
                if(m_BurnTime--==0)  //����ʱ�䵽
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
                
            if(m_MDelay--==0) { //��ȴʱ�䵽  
                m_burnState = B_END;  //��ȴ,Ȼ�����
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




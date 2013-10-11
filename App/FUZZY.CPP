#include "FUZZY.H"
#include "ctrl.h"

#define PWM_WATER_STOVE	 	PE9_O 
#define PWM_LEFT_STOVE 		PE11_O	
#define PWM_RIGHT_STOVE	 	PE13_O




void InitFuzzy(void)
{
	//AdValue = 0;
	
	CFuzzy 		*pFuzzy 	= IedDb.fuzzy; 
	long	 	*ADValue	= IedDb.ADValue.ad ;	    

	pFuzzy[LEFT_STOVE].InitFuzzyParam(	&PWM_LEFT_STOVE  ,&ADValue[LEFT_STOVE]);
	pFuzzy[RIGHT_STOVE].InitFuzzyParam(	&PWM_RIGHT_STOVE, &ADValue[RIGHT_STOVE]);
	pFuzzy[WATER_STOVE].InitFuzzyParam(	&PWM_WATER_STOVE, &ADValue[WATER_STOVE]);

	//pFuzzy[LEFT_STOVE].InitFuzzyFanPort(uint32_t *pFanport) ;
}

void CFuzzy::SetPWM(UINT iPeriod)
{				    
	if(pParam.iTimerID!=-1)			  
		TIMER_DATA::StopTimer(&pParam.iTimerID); 	  
	if(iPeriod>0)
	{                    
        *pParam.pPWMPort = ON;
		if(iPeriod!=PRIOD_FUZZY)	 
			pParam.iTimerID = TIMER_DATA::StartTimer(TIMER_MODE_ONCEROUTINE,iPeriod/10,StopPOut,&pParam);	   
	}
	else
		*pParam.pPWMPort = OFF;   
}

void CFuzzy::FuzzyRouting(void)
{
	if(WarmFlag)
	{	
		if(this==&IedDb.fuzzy[WATER_STOVE])
			PB0_O=PB1_O=PB9_O = ON;	   //���ȷ��������
		if(*AdValue/KC>10000)		   //����1000��
		{
			SetPWM(0);
			return ;
		}
			
	    //��ƫ��          
		Error[1]	= Error[0];	  	                   //�����ϴ�ƫ��ֵ    	 
		Error[0]	= (*gDest-*AdValue)/KC;	
		dErr 		= Error[0]-Error[1];														//����ƫ��仯��    
		if(Error[0]<CTEMP)      
		{  
			if(((Error[0]*dErr)>0) || ((Error[0]==0) &&( dErr!=0)))   //���������
			{ 
									//������ʱ��,�����			
				du = Error[0]*Kp;//	 
	            SumErrLimit +=Error[0];  
	            du+=SumErrLimit*Kp/Ki/KI;     //��������ģʽ	    
	        }
			else              								//��������ģʽ
			{
				if(((Error[0]*dErr)<0)||(dErr==0))				  	//����С�����
				{		   
					du =  SumErrLimit*Kp/Ki/KI;                  
				}
				else
				{
					du = PreviewOut;	
				}
			}  							 
		}
		else	
			du=PWM_PERIOD;      //ȫ������   		    	   
		//////////////////////////////////////////////////
		if(du<0)            du=0; 
		if(du>PWM_PERIOD)   du=PWM_PERIOD;  
																		    
	#ifdef _DEBUG   
		/*
		printf("%ld,",CTEMP);
		printf("%ld,%ld,",Error[0],dErr );
	    printf("%ld,",SumErrLimit );
	    printf("%ld,%ld\n",PreviewOut,du );
		*/
	    
	#endif
		SetPWM(du); 	
		PreviewOut=du;	//�����ϴ����    
	}
	else
	{
		SetPWM(0);
	}
}

void CFuzzy::StopPOut(void* p)
{
	PoutParam *param 	= (PoutParam*)p;
	param->iTimerID		= -1;		//�ضϼ��ȶ�ʱ��
	*param->pPWMPort   	= OFF;		//���ȿ��ƶ˿�		 
}

void CFuzzy::CtrlStove(UCHAR cOpen, long *lDefADValue)
{
    //ֹͣ			 
	SetPWM(0);
    WarmFlag = cOpen;
	gDest	=  lDefADValue;
    if(cOpen)       //����
    {	 					  
	    //*gDest          = lDefADValue;        //�� Ŀ�� �¶�ֵ ת�� Ϊ ADֵ  
        CTEMP          = 500;
        SumErrLimit    = 10*Ki*KI/Kp ;	
	}
}

void TaskFuzzy(void)
{
	CFuzzy 		*pFuzzy 	= IedDb.fuzzy;
	for(int i=0;i<FUZZY_ARRAY_SIZE;i++)
	{
		pFuzzy[i].FuzzyRouting( );	
	}
}

void CFuzzy::InitFuzzyParam(uint32_t *pHtCtlPrt,long * pTempChannel)
{
	pParam.pPWMPort = 	pHtCtlPrt;
	pParam.iTimerID	= 	-1;
	AdValue			= 	pTempChannel;
	WarmFlag		= 	false;
	Kp				=   40;
	Ki				= 	20;
	KI				= 	20;
}
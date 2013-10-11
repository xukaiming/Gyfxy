#ifndef _FUZZY_H_
#define _FUZZY_H_
#include "task.h"

#define FUZZY_ARRAY_SIZE 3
#define WATER_STOVE 0
#define LEFT_STOVE	1
#define RIGHT_STOVE 2




void TaskFuzzy(void);
void InitFuzzy(void);
void StopPOut(void* param);	  

#define PWM_PITCED 0
#define PWM_PERIOD PRIOD_FUZZY


#define	KC			(81*4L)//(54*4L)
#define	MAX_TEMP	1000L	    //����¯���1000�汣��,1000*556.870912 

class CFuzzy
{  
    enum {OFF = 0,ON=!OFF};
    long Reserve;
public:
	short WarmFlag;			//���±�־
	short PreviewOut;       //�ϴ������ֵ   
	short du;               //���         
	unsigned short Kp;		//����ϵ��
	unsigned short Ki;		//˥��ϵ��1
	unsigned short KI;		//˥��ϵ��2
	struct PoutParam{ 
		int8_t 	iTimerID;		//�ضϼ��ȶ�ʱ��
		int8_t	iReserve;
		uint32_t *pPWMPort;		//���ȿ��ƶ˿�		   
	}pParam;

	long *gDest;  			//Ŀ���¶ȵ�ADֵ
	long *AdValue ;
	long CTEMP;				//��ʼ�����¶� 
	long Error[2];			//ƫ��
	long dErr ;				//ƫ��仯��	  
	long SumErrLimit; 		//���ȳ�ֵ	
	/////////////////////////////////////

public:
	static void StopPOut(void* p);
	void SetPWM(UINT cPeriod);	
	void FuzzyRouting(void);
	void CtrlStove(UCHAR cOpen, long *lDefADValue);
    void InitFuzzyParam(uint32_t* pPort,long *ADAddr); 
	void StopStove()
	{
		SetPWM(0);
    	WarmFlag = 0;
	}
};		   

#endif
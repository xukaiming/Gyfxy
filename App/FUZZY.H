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
#define	MAX_TEMP	1000L	    //高温炉最高1000℃保护,1000*556.870912 

class CFuzzy
{  
    enum {OFF = 0,ON=!OFF};
    long Reserve;
public:
	short WarmFlag;			//升温标志
	short PreviewOut;       //上次输出的值   
	short du;               //输出         
	unsigned short Kp;		//比例系数
	unsigned short Ki;		//衰减系数1
	unsigned short KI;		//衰减系数2
	struct PoutParam{ 
		int8_t 	iTimerID;		//关断加热定时器
		int8_t	iReserve;
		uint32_t *pPWMPort;		//加热控制端口		   
	}pParam;

	long *gDest;  			//目标温度的AD值
	long *AdValue ;
	long CTEMP;				//开始控制温度 
	long Error[2];			//偏差
	long dErr ;				//偏差变化率	  
	long SumErrLimit; 		//加热初值	
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

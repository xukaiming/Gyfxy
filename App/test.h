#ifndef _TEST_H_
#define _TEST_H_
#include "task.h"

enum REST_STATE {REST_INIT=0,REST_START,REST_SAMPLEDISH,REST_END};
enum IED_STATE{
 IEDS_LOG_BIT        =0,     //1:系统已注册
 IEDS_REST_BIT       ,       //1:系统已复位
 
 IEDS_WEIGHTED_POT_BIT     ,       	//1:全部样品已称坩埚
 IEDS_WEIGHTED_SAMPLE_BIT  ,       	//1:全部样品试验完成
 IEDS_DRYPOT_BIT     ,       		//1:全部称坩埚已烘完
 IEDS_OK_BIT         };       		//1:OK,0:BAD

//试验完成状态
enum SAMPLE_TEST_STATE{
 CS_WEIGHTED_POT_BIT = 0,       	//已称坩埚
 CS_WEIGHTED_SAMPLE_BIT,      		//已称样
 CS_TESTED_WATER_BIT ,       		//水分试验完成
 CS_TESTED_VOL_BIT   ,       		//挥发分试验完成
 CS_WEIGHT_COVERT,BIT,
 CS_TESTED_ASH_BIT   ,       		//灰分试验完成
 CS_DRYPOT_BIT       ,       		//烧坩埚完成
 CS_COMPLETE_BIT     };       		//所有试验完成

//试验内容
enum SAMPLE_TEST_TYPE{
 TEST_WATER_BIT    = 0 ,      //水分试验
 TEST_VOL_BIT   ,       	  //挥发分试验
 TEST_ASH_BIT     };       	   //灰分试验

//试验方法：0-国标法，1-快速法，2-国标法2
enum TESTMODE {TEST_MODE_GB=0,TEST_MODE_QK};

//当前试验状态：0-称坩埚，01-称样，02-水分试验，03,加坩埚盖,04-挥发分试验，05-灰分试验，
enum STATE {S_WEIGHT_POT=0,S_WEIGHT_SAMPLE,S_TEST_WATER,S_WEIGHT_COVER,S_TEST_VOL,S_TEST_ASH};

void TaskTest(void);
void TaskReset(void);	 
void TaskManualKey(void);  
void TaskBurnSample(void);

#define MAX_WEIGHTING_CNT 6			    
class Metage{
	
	
	unsigned char m_cPos;             //称量,试验位置
	unsigned char m_cMDelay;          //称量延时计数器
	unsigned char m_cDelayCnt;		  //延时等待次数;
	unsigned char metagType;		  //称样类型
	unsigned char metagSpeed;		  //称样类型

	static char M_DELAY;             	//s,称量延时
	static char	BALANCE_ZERO;
	static char TARE_DELAY;
	static char METAGE_MAXERR;         	//0.3mg 判称量稳定极差
	static char TARE_MAXERR;         	//0.1mg 判去皮稳定极差
	BYTE* MAX_SAMPLE;					//最大样品数目
	

public:	
	long *m_cSchedule;        //称量任务,试验任务进度
	enum METAGE_STATE {M_INIT=0,M_POS,M_TARE,M_METAGE,M_SAVE,M_NEXTPOS,M_END};
	enum METAGE_TYPE{T_POT=0,T_SAMPLE,T_WATER,T_COVER,T_VOL,T_ASH};	  	//称样类型
	enum METAGE_SPEED{T_NORMAL=0,T_QUICK};
	volatile long* MetagePtr;         	 							//1:称重重量缓冲区
	Metage()
	{ 
		metagSpeed  = T_NORMAL; 
	}
	void TaskMetage(void);
	void StartMetage(METAGE_TYPE type,METAGE_SPEED speed);
    void ClearMetageFlag(void);
}  ;
extern Metage metage; 
void TaskMetage(void);
void ResetSystem(void);
#define BURN_SAMPLE_OFFSET 7L //在样位1时,炉子下的坩埚为6,7 18,19
                              //          1,2    
class Test
{
public:
	enum TEST_STATE {T_INIT=0,
                    T_WARM_WATER,T_TEST_WATER,T_WATERCOOL,T_WEIGHT_WATER,T_WATER_END,
					T_WARM_HIGH1,T_TEST_VOL,T_BURN_VOL,T_WEIGHT_VOL,T_VOL_END,
                    T_WARM_HIGH2,T_TEST_ASH,T_BURN_ASH,T_WEIGHT_ASH,T_ASH_END,	
                    T_END};	
private:
    ////////////////////////////////////////////////////////////////////////                    
    enum BURN_SAMPLE_TYPE{D_WATER=0,D_VOL,D_ASH,D_POT}; //烧样品类型 
    enum BURN_STATE{B_INIT=0,B_CHECK,B_POS,B_BURN,B_COOL,B_END};
    
    uint8_t          m_burnState;    //烧样品状态;
    uint16_t         m_BurnTime;     //灼烧时间
    BURN_SAMPLE_TYPE m_burnType;     //烧样类型
    void BurnSample(BURN_SAMPLE_TYPE sampleType);
    
    int GetWillBeTestedType(void); //得到下一个需要测试的项目
    ////////////////////////////////////////
    uint32_t m_MDelay;
	uint8_t	m_Pos;
	//uint8_t b_Wait;
	signed char m_TestTimer;
public:	
	void TaskTest(void);
	void TaskTestGB(void);
	void TaskTestQK(void);
	void StartTest(void);
	void StopTest(void);
    ///////////////////////////////////////////////////////
    void StartCool(void);
    void StopCool(void);
    ///////////////////////////////////////////////////////
    void TestWater(void);
    void TestVol(void);
    void TestAsh(void);
    void TaskBurnSample(void);
    ///////////////////////////////////////////////////////
    Test(){m_Pos=1;}
};
extern Test test;
#endif

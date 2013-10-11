#ifndef _CTRL_H_
#define _CTRL_H_
#include "task.h" 
#include "FUZZY.H"
#include "balance.h"
#ifndef __cplusplus
	#error "this file must be compiled by C++"
#endif
#define FALSE false
#define TRUE  true
 

class Beep
{
	private:
		uint32_t mBeepTrig;
		int32_t mNumBeep ;
		uint32_t *pBeepPort;
	public: 
		Beep()
		{
			mBeepTrig 	= 0;
			mNumBeep  	= 0;
			pBeepPort	= (uint32_t*)&PC13_O;
		}
		void beep(uint32_t	cNumBeep,uint32_t beepTime)
		{			    
			mNumBeep=cNumBeep;	 		    
		    ActiveTask(TASK_BEEP,beepTime<10?10:beepTime); 
		}
        
		void TaskBeep(void)
		{
		    if(!mBeepTrig)
			{
				*pBeepPort = 1;      
			}
		    else
		    {
				*pBeepPort = 0;						   
				mNumBeep--;
				if(mNumBeep==0)
				{
			    	SuspendTask(TASK_BEEP);
				}
		    }
		    mBeepTrig=!mBeepTrig;
		}	  
} ;
extern Beep beep;
#define BeepMtrErr()    beep.beep(1,50)
#define BeepBalErr()    beep.beep(1,100)
#define BeepOK()        beep.beep(1,1000)


 
#define STOVE_BAR_MID 		0
#define STOVE_BAR_LEFT 		0
#define STOVE_BAR_RIGHT 	1 

template<typename TBase>
class VertStoveBar
{

private:
	bool     bRun;							//运行
	bool     bDirUp;						//运行方向
	
	uint32_t *Top_OptoCoupler;				//到顶光耦
	uint32_t *Bot_OptoCoupler;				//到底光耦
	uint32_t *Motor_UpCtrl;					//向上运行控制
	uint32_t *Motor_DnCtrl;					//向下运行控制
	int32_t  t_RunTimer;            		//样杆运行时间
	uint16_t  *t_ToptoBotTimer;				//顶部到底超时时间	
public:
	uint32_t m_StovePos;				   	//位置
	enum StoveBarPos{POS_TOP=0,POS_UNKNOWN,POS_BOT};
	enum StoveDir{UP=0,DOWN}	;
    bool isRun(){return bRun;}
    uint32_t GetTopOpto(){ return *Top_OptoCoupler;} 
    uint32_t GetBotOpto(){ return *Bot_OptoCoupler;}
	///////////////////////////////////////////////////////////////////
	VertStoveBar()
	{
		bRun				= false;						//运行
	  	bDirUp				= false;						//运行方向
		m_StovePos			= VertStoveBar::POS_UNKNOWN;   	//位置
		t_RunTimer			= 8000;            				//样杆运行时间
		//*t_ToptoBotTimer	= 18000;						//顶部到底超时时间	
	}
	VertStoveBar(										  
		uint32_t *Top_OptoCoupler,				//到顶光耦
		uint32_t *Bot_OptoCoupler,				//到底光耦
		uint32_t *Motor_UpCtrl,					//向上运行控制
		uint32_t *Motor_DnCtrl,					//向下运行控制
		int32_t  t_RunTimer,            		//样杆运行时间
		uint16_t  *t_ToptoBotTimer				//顶部到底超时时间	
	)
	{	
		VertStoveBar();
		InitStoveBar(
			Top_OptoCoupler,				//到顶光耦
			Bot_OptoCoupler,				//到底光耦
			Motor_UpCtrl,					//向上运行控制
			Motor_DnCtrl,					//向下运行控制						  
			t_ToptoBotTimer);				//顶部到底超时时间				
	}
    void SetMaxRunTimer(){t_RunTimer=*t_ToptoBotTimer;}
    void SetMinRunTimer(){t_RunTimer=0;}
    void ReadOutputInfo(void); 
	void StopStoveBar(void)
	{
		*Motor_DnCtrl = 0;
		*Motor_UpCtrl = 0;
        
		bRun = 0;
	}
	void TaskStopBar(void)
	{
		if(bRun)
		{  
			m_StovePos = POS_UNKNOWN;
			if((*Motor_DnCtrl)&&(*Motor_UpCtrl))   //两个都高电平 停止
			{
				StopStoveBar(); 
			}
			if(bDirUp)   //向上 
			{
				t_RunTimer++;
				*Motor_UpCtrl = Top_OptoCoupler?1:0;
					//光电到位  			//超时,停止
				if((*Top_OptoCoupler == 0)||(t_RunTimer>=*t_ToptoBotTimer))
				{ 
					StopStoveBar();					//中间升降 1370    //左右炉	 1270
					__NOP();
				}					   
			}
			else	
			{
				t_RunTimer--;
				*Motor_DnCtrl = Bot_OptoCoupler?1:0;
				if((*Bot_OptoCoupler == 0)||(t_RunTimer<=0 ))//超时,停止
				{
					t_RunTimer	  	= 0;
					StopStoveBar();
					__NOP();
				}
			}  
		}
		else
		{ 
			if(*Top_OptoCoupler==0)//到顶光耦为0
				m_StovePos = POS_TOP;	
			else if((!bDirUp)&&(*Bot_OptoCoupler==0)) //向下运行 而且到底  才是底位
			{
				m_StovePos = POS_BOT;
				t_RunTimer = 0;
			}
			else
				m_StovePos = POS_UNKNOWN;	
		} 
        ReadOutputInfo(); //IedDb.OutputCoil.iCoil16 = GPIOC->ODR;        
	}
	int MoveStoveBar(unsigned int pos);	
	void InitStoveBar(
		uint32_t *Top_Opto,				//到顶光耦
		uint32_t *Bot_Opto,				//到底光耦
		uint32_t *UpCtrl,					//向上运行控制
		uint32_t *DnCtrl,					//向下运行控制
		uint16_t *t_ToptoBotTimer
	)
	{
	
		Top_OptoCoupler = Top_Opto;				//到顶光耦
		Bot_OptoCoupler = Bot_Opto;				//到底光耦
		Motor_UpCtrl	= UpCtrl;				//向上运行控制
		Motor_DnCtrl	= DnCtrl;				//向下运行控制
		
		          		//样杆运行时间
		this->t_ToptoBotTimer		= t_ToptoBotTimer;	
		this->t_RunTimer			= *t_ToptoBotTimer/2;  	
	}
		    
};								//炉子样杆



class LowVertStoveBar :public VertStoveBar<LowVertStoveBar>
{
};
class HighVertStoveBar :public VertStoveBar<HighVertStoveBar>
{
};
 
#define LOW_POS_BOT         LowVertStoveBar::POS_BOT 
#define HIGH_POS_BOT        HighVertStoveBar::POS_BOT
#define LOW_POS_TOP         LowVertStoveBar::POS_TOP 
#define HIGH_POS_TOP        HighVertStoveBar::POS_TOP 

extern  LowVertStoveBar     m_LowTempStoveBar[];
extern  HighVertStoveBar    m_HighTempStoveBar[];
//
#define LowTempStove        m_LowTempStoveBar[0]
#define LeftHighTempStove   m_HighTempStoveBar[0]
#define RightHighTempStove  m_HighTempStoveBar[1]


#define MAX_SAMPLE_CNT  24
//#define DEFAULT_SUBDIV	16
#define DEFAULT_SUBDIV	40
#define DEFAULT_PULSECNT_PERROUND	200  
#define MAX_FREQ		16000
#define INFO_LENTH      32
////////////////////////////////////////////////////////////
class  StepMotorCtrl
{
public:
	enum DIR{FORWARD=1,BACKWARD=!FORWARD};
	enum B_LOCK{LOCK=0,UNLOCK=!LOCK};
	struct StepMotorState 
	{										
		bool bRun;							//运行
		bool bDir;							//方向
		volatile int32_t DestPulseCnt;		//到达目标位置脉冲数
		volatile uint32_t DestSamplePos;	//目标工位

		volatile int32_t CurPulseCnt;		//当前位置脉冲书
		volatile uint32_t CurSamplePos;		//当前工位
																		  
		
		volatile int32_t PulsePerRound;	//每移动一圈需要的脉冲数
		volatile uint32_t *ResetIn;			//复位输入端口
	};
private:
	//StepMotorState MtrState;
	StepMotorState *pMtrState;
	/////////////////////////////////////////////////////////////
	short *PulsePerRoundSubDiv;			//每圈1细分下所需要的脉冲
	BYTE *SubDiv;						//细分数  	
	BYTE *AllSampleCnt;					//最大工位
	////////////////////////////////////////////////////////////////
	TIM_TypeDef* TIMx;					//使用的定时器
	int  Channel;						//定时器通道				
	////////////////////////////////////////////////////////////////	
	uint32_t *pPortFree;				//锁定引脚
	uint32_t* pPortDir;					//方向引脚
private:
	
	void InitTimer(void);
public:
	StepMotorCtrl();	  	
	//int MoveStepMotor(unsigned long step,StepMotorCtrl::DIR direction);	
	void SetFreq(uint16_t Freq);  
	int MoveStepMotor(long DestPulseCnt,StepMotorCtrl::DIR direction);	
	int MoveStepMotor(long DestPos);
	void StopMotor(void);

	void InitCtrl(TIM_TypeDef *TIMx ,int Channel, uint32_t* pPortFree, uint32_t* pPortDir ,uint32_t* pResetIn);
	void InitMotor(
			long AllSampleCnt 			= MAX_SAMPLE_CNT,
			long PulsePerRoundSubDiv	= DEFAULT_PULSECNT_PERROUND,
			long SubDiv					= DEFAULT_SUBDIV);
};
extern StepMotorCtrl StepMotor[2];  

#define  StepMotor0 StepMotor[0]
#define  StepMotor1 StepMotor[1]

class CFuzzy;
///////////////////////////////////////////
typedef struct 
{
	struct DownParam
	{
        
		USHORT			m_sTimePut2weigh;//放样到称样时间(0_1S);
		USHORT			m_sTimePut2HighTemp;//放样到高温时间(0_1S);
		SHORT			m_sPulsePerRound1Spit;//样盘一圈脉冲数;
		BYTE			m_btDriverSpitCnt;//驱动器细分数;
		BYTE			m_btMaxSampleCnt;//样位个数
        BYTE			m_ByteReserve[8];
        
		SHORT			m_fBalanceMaxErr;//称样判值(0.1mg);
		SHORT			m_fVolSampleMax;	  //最大称样重量
		SHORT			m_fVolSampleMin;     //最小称样重量  
            
		SHORT			m_fWaterAshSampleMax;
		SHORT			m_fWaterAshSampleMin;
        
		//SHORT			m_sLowStoveTempSpeed;//低温炉升温速率
		//SHORT			m_sHighStoveTempSpeed;//高温炉升温速率;	
        /*
		SHORT			m_sQuick_WaterTestTime;//快速水分干燥时间(分);
		SHORT			m_sQuick_VolTestTime;//分干燥时间(分);
		SHORT			m_sQuick_AshTestTime;//快速灰分干燥时间(分);

		SHORT			m_sGB_WaterTestTime;//国标水分干燥时间(分);
		SHORT			m_sGB_VolTestTime;//国标挥发分干燥时间(分);
		SHORT			m_sGB_AshTestTime;//国标灰分干燥时间(分);
        */
        
		SHORT			m_sWaterTestTime;//国标水分干燥时间(分);
		SHORT			m_sVolTestTime;//国标挥发分干燥时间(分);
		SHORT			m_sAshTestTime;//国标灰分干燥时间(分);
        
		SHORT			m_sWaterCoolTime;		//水分炉坩埚冷却时间
		SHORT			m_sHighTempCoolTime;	//高温炉坩埚冷却时间
        
		SHORT			m_sBeepTime;	

		LONG			m_sWaterTestTemp;//快速水分试验温度(℃);
		LONG			m_sVolTestTemp;//快速挥发分试验温度(℃);
		LONG			m_sAshTestTemp;//快速灰分试验温度(℃);
        
		SHORT			m_szTestSequence;     //测试顺序
		SHORT			m_szalgorithm;			//测试方法
		enum M_ALG{M_GB=0,M_QK};	
        char            m_szSystemType;
		char            m_cReserve[3];
		long            m_reServe[8]; 
	}DownloadWorkParam;  //下载参数

	    ///////////////////////////////////////////////////////////////////   
	union 			  //输出寄存器
	{
		struct 	
		{	
			UINT BALANCE_MOTOR_UP	:1;		//称样上升
			UINT BALANCE_MOTOR_DN	:1;		//称样下降
			UINT LEFT_MOTOR_UP		:1;		//左炉上升
			UINT LEFT_MOTOR_DN		:1;		//左炉下降

			UINT RIGHT_MOTOR_UP		:1;		//右炉上升
			UINT RIGHT_MOTOR_DN		:1;		//右炉下降	
			UINT Reserve1			:2;		//本来留着准备交流电机控制样盘转动的

			
			UINT OXYGEN		        :1;		//左炉氧气开关
			UINT NITROGEN		    :1;		//右炉氧气开关
            UINT WATER_STOVE_FAN	:1;		//水分炉子交流风扇
			UINT LEFT_STOVE_FAN		:1;		//左炉风扇
			UINT RIGHT_STOVE_FAN	:1;		//右炉风扇
			UINT SPK				:1;		//蜂鸣器
			UINT Reserve2			:2;		// 
		}sCoil16;	
		UCHAR	iCoil8;			 
		USHORT  iCoil16;          
		UINT    cCoil32;                  
	}OutputCoil;                                                      //32BIT
    union 					        								  //此部分已经作废
    {																							   
        struct  input
        {
			UINT    SAMPLE_POS_1			:1;  //样位1
			UINT    SAMPLE_POS_N			:1;  //样位N
			UINT    WATER_STOVE_PUT			:1;  //样位N
			UINT    WATER_STOVE_WEIGH		:1;  //称样区
			UINT	LEFT_STOVE_BOT			:1;  //低位
			UINT	LEFT_STOVE_TOP			:1;  //左高位
			UINT	RIGHT_STOVE_BOT			:1;  //低位
			UINT	RIGHT_STOVE_TOP			:1;  //左高位												  
        };
		uint8_t 	cInput8;
        uint16_t    cInput16;  
		uint32_t    cInput32; 
    }InputStatus;     
	struct
	{
		long ad[3];
		long    adInstTemp;   //主板环境温度
	}ADValue;									 	    
	CFuzzy fuzzy[FUZZY_ARRAY_SIZE];
	Balance::BalanceBuf buf;
	StepMotorCtrl::StepMotorState MtrState;

	//////////////////////////////////////////////////
	short  	ResetState;		//复位状态
    short   ErrorFlag;      //错误标志,如果发现有错误,读取错误状态字
	long 	TestState;		//测试状态
	long    MetageState;	//称量进程
	typedef struct ypd      //样品数据定义
	{
		//enum STATE {S_NULL=0,S_WAIT,S_WEIGHT_POT,S_WEIGHT_SAMPLE,
		//				S_TEST_WATER,S_WEIGHT_COVER,S_TEST_VOL,
		//				S_TEST_ASH};
	 	UCHAR cSta;     //当前需要试验状态, 上位机传下来的 0x03+(cItem<<2)
	 	UCHAR cCSta;    //试验称样完成状态, //需要称量的类型
	 	UCHAR cItem;    //试验内容：b0=1:水分，b1=1:挥发分，b2=1:灰分 
		UCHAR reserve;	
		union{	
			struct{
			long  lMPot;    //坩埚质量(unit=0.1mg)
			/////////////////////////////////////////////////////////////////
		    long  lM0;      //试验前样品质量(unit=0.1mg),含坩埚质量。
		    long  lM1;      //水分试验后样品质量(unit=0.1mg),含坩埚质量。
			/////////////////////////////////////////////////////////////////
			long  lMPotSample2;//挥发份实验前加盖重量
			//
			long  lM2;      //挥发分试验后样品质量(unit=0.1mg),含坩埚质量。
			long  lM3;      //灰分试验后样品质量(unit=0.1mg),含坩埚质量。
			}SQuality;
			long Q[6];
		}UQuality;
		
	}YP_DATA;	
	YP_DATA Sample1[24] ;	 
	
	//////////////////////////////////////////////////
	struct
	{
		UCHAR   CmdType;
        UCHAR   CmdParam1;
        UCHAR   CmdParam2;   
        UCHAR   CmdParam3;   
        LONG    CmdParam;   
	}Cmd;
 
	char InstrumentName[32];//32BIT[]
    char ErrorMsg[INFO_LENTH];  //错误信息
    /////////////////////////////////////////////////////////////////   
	uint8_t   cID;                //IED站号
	uint8_t	  ReserveID;

} IED_DATABASE;
extern  IED_DATABASE  IedDb;
#define ErrBuf IedDb.ErrorMsg
#define ErrFlg IedDb.ErrorFlag
#define MaxSampleCnt IedDb.DownloadWorkParam.m_btMaxSampleCnt
enum enum_IED_type{G_NONE = 0,G5000,G5200,G5500}; 
/////////////////////////////////////////
void TaskBlinLed(void);
void TaskStopBar(void);
void TaskParsaCmd(void);
void TaskBeep(void);
void InitIED(void);
void CtrlOxygenValve(bool bEnable);
void CtrlNitrogenValve(bool bEnable);
void StopAllFan(void);
void EnableLeftFan(bool Enable)	 ;
void EnableRightFan(bool Enable);
void EnableWaterFan(bool Enable) ;

#if defined ( __CC_ARM   )
	void InitStepMotor(void);
	void InitStove(void);
#endif


#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                                                                 
	extern "C"        {
	#endif
#endif
	void TIM4_IRQHandler(void) 	 ;
#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                 
	}
	#endif
#endif

#endif



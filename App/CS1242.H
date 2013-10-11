#ifndef _CS_1242_H_
#define _CS_1242_H_




#define	REG_SETUP		0x00	    //Setup Register
#define	REG_MUX			0x01	    //Multiplexer Control Register
#define	REG_ACR			0x02	    //Analog Control Register
#define	REG_ODAC		0x03	    //Offset DAC
/*
//ADS1242 REGISTER DEFINITIONS
#define	REG_DIO			0x04	    //Data I/O
#define	REG_DIR			0x05	    //Direction	Control	for Data I/O
#define	REG_IOCON		0x06	    //I/O Configuration	Register
*/
#define	REG_OCC0		0x07	    //Offset Calibration Coefficient
#define	REG_OCC1		0x08
#define	REG_OCC2		0x09
#define	REG_GCC0		0x0a	    //Full-Scale Register
#define	REG_GCC1		0x0b
#define	REG_GCC2		0x0c
#define	REG_DOR2		0x0d	    //Data Output Register
#define	REG_DOR1		0x0e
#define	REG_DOR0		0x0f

//ADS1242 COMMAND DEFINITIONS
#define	CMD_RDATA		0x01	    //Read Data
#define	CMD_RDATAC		0x03	    //Read Data	Continuously
#define	CMD_STOPC		0x0f	    //Stop Read	Data Continuously
#define	CMD_RREG		0x10	    //1x Read from REG “rrrr”	0001 r r r r (1xH) xxxx_nnnn (#	of regs-1)
#define	CMD_WREG		0x50	    //5x Write to REG “rrrr” 0101 r r	r r (5xH) xxxx_nnnn (# of regs-1)
#define	CMD_SELFCAL		0xF0	    //Self Cal Offset and Gain
#define	CMD_OSELFCAL	0xF1	    //Self Cal Offset
#define	CMD_GSELFCAL	0xF2	    //Self Cal Gain
#define	CMD_OSYSCAL		0xF3	    //Sys Cal Offset
#define	CMD_GSYSCAL		0xF4	    //Sys Cal Gain
#define	CMD_DSYNC		0xFc	    //Sync DRDY
#define	CMD_SLEEP		0xFd	    //Put in SLEEP Mode
#define	CMD_RESET		0xFe	    //Reset to Power-Up	Values

//#define	CHANNEL1		0x01	    
//#define	CHANNEL2		0x23	    

#define  CHANNEL0 0x03                //AI1=小电极
#define  CHANNEL1 0x13                //AI2=炉温
#define  CHANNEL2 0x23                //AI2=炉温



void 	InitSPI1(void);
void 	InitCS1242(void);   
long    ReadAdData(void);    
void    InitExt1(void);
void    TaskReadAD(void);


union LongData 			//定义一个联合体数据，
{
	long word ;
	unsigned char byte[4];
};
#define VREF 3300L          

#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                                                                 
	extern "C"        {
	#endif
#endif 
//外部中断函数  
void EXTI9_5_IRQHandler(void) ;
#if defined ( __CC_ARM   ) 
	#ifdef __cplusplus                                 
	}
	#endif
#endif


#endif

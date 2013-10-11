#include "MODBUS.H"
#include "CTRL.H"   
#include "task.h"
//#include "DRVUART.H"
//#include "DrvPDMA.h"
#define USINGUART0
#define MAX_BUF_SIZE 512	
#define T15    18	 //18个bit				
uint8_t	    receTimeOut;		//接收超时
uint8_t     TxTimeOut;			//启动发送延时
uint8_t	    sendBuf[MAX_BUF_SIZE],receBuf[MAX_BUF_SIZE]; //发送接收缓冲区	  
uint16_t   	*temp ; 					   	 
//uint8_t	    checkoutError;	// ==2 偶校验错	   
//uint32_t 	dwTickCount;					  
//uint16_t  testRegister;	//用于测试 字址址16
uint8_t	sendCount;		//发送字节个数
uint8_t	receCount;	    //接收到的字节个数
uint8_t	sendPosi;	    //发送位置 

#define  enable()  {__enable_irq();}
#define  disable() {__disable_irq();}
USART_InitTypeDef USART_InitStructure;		 

void beginSend(void);
void checkComm0Modbus(void);
void readCoil(void);
void readRegisters(void);
void forceSingleCoil(void);
void presetSingleRegister(void);
void presetMultipleRegisters(void);
void forceMultipleCoils(void);
uint16_t getRegisterVal(uint16_t addr,uint16_t *tempData);  
uint16_t getInputRegisterVal(uint16_t addr,uint16_t *tempData);
uint16_t getCoilVal(uint16_t addr,uint8_t *tempData);
uint16_t getInputStatusVal(uint16_t addr,uint8_t *tempData);
uint16_t setCoilVal(uint16_t addr,uint8_t tempData);
void Send(void); 
void Uart0PDMASend(void);

void InitModbus(void)
{
	temp = (uint16_t *)&IedDb;    
	InitUART0();	          
}                                                              

void* mymemcpy( void* dest, const void* src, int count )
{
    char* d = (char*)dest;
    const char* s = (const char*)src;
    int n = count>>2;
    switch( count & 3 )
    {
		case 0:  
      	do {  
            *d++ = *s++;
            case 3:      *d++ = *s++;
            case 2:      *d++ = *s++;
            case 1:      *d++ = *s++;
         } while (--n > 0);
   }
   return dest;
}


//字地址 0 - 255 (只取低8位)
//位地址 0 - 255 (只取低8位)

/* CRC 高位字节值表 */
const uint8_t auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
/* CRC低位字节值表*/
const uint8_t auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;
											    
uint16_t crc16(uint8_t *puchMsg, uint16_t usDataLen)
{
	uint8_t uchCRCHi = 0xFF ; /* 高CRC字节初始化 */
	uint8_t uchCRCLo = 0xFF ; /* 低CRC 字节初始化 */
	uint32_t uIndex ; /* CRC循环中的索引 */
	while (usDataLen--) /* 传输消息缓冲区 */
	{
		uIndex = uchCRCHi ^ *puchMsg++ ; /* 计算CRC */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	return (uchCRCHi << 8 | uchCRCLo) ;
}//uint16_t crc16(uint8_t *puchMsg, uint16_t usDataLen)

//读输入寄存器
void ReadInputStatus(void)
{
	uint16_t addr;
	uint16_t tempAddr;
	uint16_t byteCount;
	uint8_t bitCount;
	uint16_t crcData;
	uint8_t position;
	uint8_t i,k;
	uint8_t tempData;
	uint8_t  exit = 0;

	//addr = (receBuf[2]<<8) + receBuf[3];
	//tempAddr = addr & 0xfff;
	addr = receBuf[3];
	tempAddr = addr;

	//bitCount = (receBuf[4]<<8) + receBuf[5];	//读取的位个数
	bitCount = receBuf[5];

	byteCount = bitCount / 8;					//字节个数
	if(bitCount%8 != 0)
		byteCount++;

	for(k=0;k<byteCount;k++)
	{//字节位置
		position = k + 3;
		sendBuf[position] = 0;
		for(i=0;i<8;i++)
		{
			getInputStatusVal(tempAddr,&tempData);

			sendBuf[position] |= tempData << i;
			tempAddr++;
			if(tempAddr >= addr+bitCount)
			{	//读完
				exit = 1;
				break;
			}
		}
		if(exit == 1)
			break;
	}

	sendBuf[0] = IedDb.cID;//localAddr;
	sendBuf[1] = 0x02;
	sendBuf[2] = byteCount;
	byteCount += 3;
	crcData = crc16(sendBuf,byteCount);
	sendBuf[byteCount] = crcData >> 8;
	byteCount++;
	sendBuf[byteCount] = crcData & 0xff;
	sendCount = byteCount + 1;

	beginSend();
}

//读线圈状态
void readCoil(void)
{
	uint8_t addr;
	uint8_t tempAddr;
	uint16_t byteCount;
	uint8_t bitCount;
	uint16_t crcData;
	uint8_t position;
	uint8_t i,k;
	uint8_t tempData;
	uint8_t  exit = 0;

	//addr = (receBuf[2]<<8) + receBuf[3];
	//tempAddr = addr & 0xfff;
	addr = receBuf[3];
	tempAddr = addr;

	//bitCount = (receBuf[4]<<8) + receBuf[5];	//读取的位个数
	bitCount = receBuf[5];

	byteCount = bitCount / 8;					//字节个数
	if(bitCount%8 != 0)
		byteCount++;

	for(k=0;k<byteCount;k++)
	{//字节位置
		position = k + 3;
		sendBuf[position] = 0;
		for(i=0;i<8;i++)
		{
			getCoilVal(tempAddr,&tempData);

			sendBuf[position] |= tempData << i;
			tempAddr++;
			if(tempAddr >= addr+bitCount)
			{	//读完
				exit = 1;
				break;
			}
		}
		if(exit == 1)
			break;
	}

	sendBuf[0] = IedDb.cID;//localAddr;
	sendBuf[1] = 0x01;
	sendBuf[2] = byteCount;
	byteCount += 3;
	crcData = crc16(sendBuf,byteCount);
	sendBuf[byteCount] = crcData >> 8;
	byteCount++;
	sendBuf[byteCount] = crcData & 0xff;
	sendCount = byteCount + 1;

	beginSend();
}//void readCoil(void)
//读输入寄存器
void readInputRegisters(void)
{
	uint16_t addr;
	uint16_t tempAddr;
	uint16_t crcData;
	uint8_t readCount;
	uint16_t byteCount;				 
	uint16_t i;
	uint16_t tempData = 0;

	addr = (receBuf[2]<<8) + receBuf[3];
	tempAddr = addr & 0xfff;                                      
	readCount = receBuf[5];

	byteCount = readCount * 2;
	disable();
	for(i=0;i<byteCount;i+=2,tempAddr++)
	{
		getInputRegisterVal(tempAddr,&tempData);
		sendBuf[i+3] = tempData >> 8;
		sendBuf[i+4] = tempData & 0xff;
	}
	enable();
	sendBuf[0] = IedDb.cID;//localAddr;
	sendBuf[1] = 4;
	sendBuf[2] = byteCount;
	byteCount += 3;
	crcData = crc16(sendBuf,byteCount);
	sendBuf[byteCount] = crcData >> 8;
	byteCount++;
	sendBuf[byteCount] = crcData & 0xff;

	sendCount = byteCount + 1;
	beginSend();
}
//读寄存器
void readRegisters(void)
{
	uint16_t addr;
	uint16_t tempAddr;
	uint16_t crcData;
	uint16_t readCount;
	uint16_t byteCount;				 
	uint16_t i;
	uint16_t tempData = 0;

	addr = (receBuf[2]<<8) + receBuf[3];
	tempAddr = addr & 0xffff;
	//addr = receBuf[3];
	//tempAddr = addr;

	//readCount = (receBuf[4]<<8) + receBuf[5];	//要读的个数
	readCount = receBuf[5];	//要读的个数
	byteCount = readCount <<1;
	disable();
	for(i=0;i<byteCount;i+=2,tempAddr++)
	{
		getRegisterVal(tempAddr,&tempData);
		sendBuf[i+3] = tempData >> 8;
		sendBuf[i+4] = tempData & 0xff;
	}
	enable();
	sendBuf[0] = IedDb.cID;//localAddr;
	sendBuf[1] = 3;
	sendBuf[2] = byteCount;
	byteCount += 3;
	crcData = crc16(sendBuf,byteCount);
	sendBuf[byteCount] = crcData >> 8;
	byteCount++;
	sendBuf[byteCount] = crcData & 0xff;

	sendCount = byteCount +1;
	beginSend();
}//void readRegisters(void)


//强制单个线圈
void forceSingleCoil(void)
{
	uint8_t addr;
	uint8_t tempAddr;
	uint16_t tempData;
	uint8_t  onOff;
	uint8_t i;

	//addr = (receBuf[2]<<8) + receBuf[3];
	//tempAddr = addr & 0xfff;
	addr = receBuf[3];
	tempAddr = addr;

	//onOff = (receBuf[4]<<8) + receBuf[5];
	onOff = receBuf[4];

	//if(onOff == 0xff00)
	if(onOff == 0xff)
	{	//设为ON
		tempData = 1;
	}
	//else if(onOff == 0x0000)
	else if(onOff == 0x00)
	{	//设为OFF
		tempData = 0;
	}

	setCoilVal(tempAddr,tempData);

	for(i=0;i<receCount;i++)
	{
		sendBuf[i] = receBuf[i];
	}
	sendCount = receCount;
	beginSend();
}//void forceSingleCoil(void)


//设置多个寄存器
void presetMultipleRegisters(void)
{
	uint16_t addr;
	uint16_t tempAddr;   
//	uint8_t byteCount;
	uint8_t setCount;
	uint16_t crcData;
	uint16_t tempData;						   
	uint8_t i;

	addr = (receBuf[2]<<8) + receBuf[3];
	tempAddr = addr & 0xfff;
	//addr = receBuf[3];
	//tempAddr = addr & 0xff;

	//setCount = (receBuf[4]<<8) + receBuf[5];
	setCount = receBuf[5];
//	byteCount = receBuf[6];
						   

	for(i=0;i<setCount;i++,tempAddr++)
	{
		tempData = (receBuf[i*2+7]<<8) + receBuf[i*2+8];  

		setRegisterVal(tempAddr,tempData);
	}

	sendBuf[0] = IedDb.cID;//localAddr;
	sendBuf[1] = 16;
	sendBuf[2] = addr >> 8;
	sendBuf[3] = addr & 0xff;
	sendBuf[4] = setCount >> 8;
	sendBuf[5] = setCount & 0xff;
	crcData = crc16(sendBuf,6);
	sendBuf[6] = crcData >> 8;
	sendBuf[7] = crcData & 0xff;
	sendCount = 8;			   
	beginSend();
}//void presetMultipleRegisters(void)

void presetSingleRegister(void)
{
	uint16_t addr;
//	uint8_t tempAddr;	     
	uint16_t crcData;
	uint16_t tempData;		 

	addr = (receBuf[2]<<8) + receBuf[3];
//	tempAddr = addr & 0xfff;

	tempData = (receBuf[4]<<8) + receBuf[5];	   

	setRegisterVal(addr,tempData); 

	sendBuf[0] = IedDb.cID;//localAddr;
	sendBuf[1] = 06;
	sendBuf[2] = addr >> 8;
	sendBuf[3] = addr & 0xff;
	sendBuf[4] = tempData >> 8;
	sendBuf[5] = tempData & 0xff;
	crcData = crc16(sendBuf,6);
	sendBuf[6] = crcData >> 8;
	sendBuf[7] = crcData & 0xff;
	sendCount = 8;			   
	beginSend();
}

//检查uart0数据

void checkComm0Modbus(void)
{
	uint16_t crcData;
	uint16_t tempData;
	if((receBuf[0]!=IedDb.cID)&&(receBuf[0]!=0xFF)) //localAddr
		return ;
	if(receCount > 4)
	{      
		switch(receBuf[1])
		{
			case 1://读取线圈状态(读取点 16位以内)
            case 2://读取输入寄存器
			case 3://读取保持寄存器(一个或多个)    
            case 4:
			case 5://强制单个线圈
			case 6://设置单个寄存器
					
					if(receCount >= 8)
					{//接收完成一组数据
						//应该关闭接收中断
                                                
						{
							crcData = crc16(receBuf,6);
							if(crcData == receBuf[7]+(receBuf[6]<<8))
							{//校验正确
								if(receBuf[1] == 1)
								{//读取线圈状态(读取点 16位以内)
									readCoil();
								}
                                else if(receBuf[1] == 2)
                                {
                                    //读取输入节点
                                    ReadInputStatus();
                                }
								else if(receBuf[1] == 3)
								{//读取保持寄存器(一个或多个)
									readRegisters();
								}
                                else if(receBuf[1] == 4)
								{//读输入寄存器
									readInputRegisters();
								}
								else if(receBuf[1] == 5)
								{//强制单个线圈
									forceSingleCoil();
								}
								else if(receBuf[1] == 6)
								{
									presetSingleRegister();
									//setRegisterVal
								}

							}
						}
						receCount = 0;	         
					}
					break;

			case 15://设置多个线圈
					tempData = receBuf[6];
					tempData += 9;	//数据个数
					if(receCount >= tempData)
					{                              
						{
							crcData = crc16(receBuf,tempData-2);
							if(crcData == (receBuf[tempData-2]<<8)+ receBuf[tempData-1])
							{
								//forceMultipleCoils();
							}
						}
						receCount = 0;       
					}
					break;

			case 16://设置多个寄存器
					tempData = (((unsigned int )receBuf[4])<<8) + receBuf[5];
					tempData = tempData * 2;	//数据个数
					tempData += 9;

																								    
					if(receCount >= tempData)
					{    
				        crcData = crc16(receBuf,tempData-2);
				        if(crcData == (((unsigned int)receBuf[tempData-2])<<8)+ receBuf[tempData-1])
						{
							presetMultipleRegisters();
						}     						
						receCount = 0;       
					}
					break;

			default:
					break;
		}
	}
}//void checkComm0(void)

//取线圈状态 返回0表示成功
uint16_t getCoilVal(uint16_t addr,uint8_t *tempData)
{					   
//	uint16_t result = 0;
	uint16_t tempAddr;
	unsigned char *pCoilPtr = 0;  
	unsigned int CoilAddr 	= 0;
	unsigned int CoilBit  	= 0;   

	tempAddr = addr & 0xFF;

	tempAddr = tempAddr%16;  //只有8位

	CoilAddr = tempAddr/8;
	CoilBit  = tempAddr%8;
																					   
    IedDb.OutputCoil.iCoil16 = GPIOC->ODR;
	pCoilPtr = &IedDb.OutputCoil.iCoil8;
	pCoilPtr+= CoilAddr;

	if(*pCoilPtr&(1<<CoilBit))
		*tempData	  = 1;
	else
		*tempData     = 0;	
	return 1;
}//uint16_t getCoilVal(uint16_t addr,uint16_t *data)


uint16_t getInputStatusVal(uint16_t addr,uint8_t *tempData)
{					   
//	uint16_t result = 0;
	uint16_t tempAddr;
	unsigned char *pRegPtr = 0;  
	unsigned int RegAddr = 0;
	unsigned int RegBit  = 0;   

	tempAddr = addr & 0xFF;

	

	RegAddr = tempAddr/8;
	RegBit  = tempAddr%8;

    IedDb.InputStatus.cInput16 = GPIOD->IDR;																					 
	pRegPtr = &IedDb.InputStatus.cInput8;
	pRegPtr+= RegAddr;

	if(*pRegPtr&(1<<RegBit))
		*tempData	  = 1;
	else
		*tempData     = 0;	
	return 1;
}//uint16_t getCoilVal(uint16_t addr,uint16_t *data)

//设定线圈状态 返回0表示成功
uint16_t setCoilVal(uint16_t addr,uint8_t tempData)
{
//	uint16_t i;
	uint16_t result = 0;
	uint16_t tempAddr;
//	uint16_t tempvar;
	unsigned char *pCoilPtr = 0;

	unsigned int CoilAddr = 0;
	unsigned int CoilBit  = 0;  



	tempAddr = addr & 0xFF;

	tempAddr = tempAddr%16;  //只有16位

	CoilAddr = tempAddr/8;
	CoilBit  = tempAddr%8;
	pCoilPtr = &IedDb.OutputCoil.iCoil8; 
	pCoilPtr+= CoilAddr;
	if	(tempData) //如果为1
	{
		*pCoilPtr|=1<<CoilBit;		
		__NOP();	
	}
	else
	{
		*pCoilPtr&=~(1<<CoilBit);
		__NOP();	
	}
	
//	tempvar = (unsigned short)*pCoilPtr;
	GPIOC->ODR = IedDb.OutputCoil.iCoil16 ;					                     
	return result;
}//uint16_t setCoilVal(uint16_t addr,uint16_t data)




uint16_t getInputRegisterVal(uint16_t addr,uint16_t *tempData) 
{
	uint16_t result = 0;
	uint16_t tempAddr;        
    uint16_t *ptr   =  &IedDb.InputStatus.cInput16 ; 
   	*ptr			=  GPIOD->IDR;
	tempAddr = addr & 0xfff;	   
    *tempData= ptr[tempAddr];
	return result;
}
//取寄存器值 返回0表示成功
uint16_t getRegisterVal(uint16_t addr,uint16_t *tempData)
{

	uint16_t result = 0;
	uint16_t tempAddr; 
	tempAddr = addr & 0xfff;	   
    *tempData=temp[tempAddr];
	return result;
}//uint16_t getRegisterVal(uint16_t addr,uint16_t &data)
 
/******************************							  
modbus RTU 的C51程序
通信波特率 9600 8位数据 1位停止位 偶校验 485通位接口
单片机控制板地址 localAddr(变量)
通信可设置数据的地址：
字地址 0 - 255 (只取16位的低8位)
位地址 0 - 255 (只取16位的低8位)
*******************************/
														    
void TaskCommTimer(void)
{
	if(receTimeOut>0)
	{	
		--receTimeOut;				 			   
		if(receTimeOut==0 && receCount>0)   //判断通讯接收是否超时
		{			  
            checkComm0Modbus(); 							 
			receCount = 0;      //将接收地址偏移寄存器清零       
		}			
	}
	if(TxTimeOut>0)							//延时启动发送任务
	{
		--TxTimeOut;
		if(TxTimeOut==0)
		{
			
    		sendPosi = 0;
			Send();
			//Uart0PDMASend();
		}
	}
	    		
}

//开始发送														  
void beginSend(void)
{
	TxTimeOut = 2;
}//void beginSend(void)

void Send(void)
{ 
	b485Send = ComTx;	//设为发送
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE); //运行发送中断   
}
//设置寄存器值 返回0表示成功
uint16_t setRegisterVal(uint16_t addr,uint16_t tempData)
{
	uint16_t result = 0;
	uint16_t tempAddr;      
	tempAddr = addr & 0xfff;    
    temp[tempAddr]=tempData;   
	return result;
}
 

								 	   
void Uart0PDMASend(void)
{
	b485Send = ComTx;	//设为发送
    sendPosi = 0;
													  
		    
	DMA_InitTypeDef DMA_InitStructure;
																		   
  	DMA_DeInit(USART1_Tx_DMA_Channel);  
  	DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_BASE;
  	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)sendBuf;
  	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  	DMA_InitStructure.DMA_BufferSize = sendCount;
  	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  	DMA_Init(USART1_Tx_DMA_Channel, &DMA_InitStructure);
																 
   														  				    
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE); 
																		    
	DMA_ITConfig(USART1_Tx_DMA_Channel,  DMA_IT_TC|DMA_IT_TE ,ENABLE); 
  	DMA_Cmd(USART1_Tx_DMA_Channel, ENABLE);	    	  
} 



void InitUART0(void)
{
	b485Send = ComRx;	// Receive;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO, ENABLE);		  
	//端口初始化
	// Configure USART1 Tx as alternate function open-drain 
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);  
		
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
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
	USART_Init(USART1, &USART_InitStructure);
	

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //DMA完成.错误中断
	NVIC_Init(&NVIC_InitStructure);	  

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; 
	NVIC_Init(&NVIC_InitStructure);	

	//USART_ITConfig(USART1,USART_IT_TXE, ENABLE);
	//USART_ITConfig(USART1,USART_IT_TC, ENABLE);
	USART_ITConfig(USART1,USART_IT_RXNE, ENABLE);
	
	USART_Cmd(USART1, ENABLE); 		  
}

USART_TypeDef UART1  __at (USART1_BASE);

void USART1_IRQHandler(void)
{			    
		//CTS  LBD  | TXE  TC   RXNE  IDLE   | ORE   NE   FE   PE									 
	  //只要进了中断,状态字就被清除.
	  uint32_t sr = UART1.SR;
	  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	  {
	  		receTimeOut = 5;     //通讯超时值 
			receBuf[receCount++]=  USART1->DR;
	  }
	  	   													    
	  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	  {   	
		if(sendPosi < sendCount)
		{    
			USART1->DR =   sendBuf[sendPosi]; 		           
			sendPosi++;				
		}
        else
        {  								  
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE); 	 
			USART_ITConfig(USART1, USART_IT_TC, ENABLE); 	    
			
        }
	  }
	   
	  if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	  {
	  		USART_ITConfig(USART1, USART_IT_TC, DISABLE); 
			b485Send = ComRx;	// Receive; 
	  }
	  	   
}

								    


void DMA1_Channel1_IRQHandler()
{
	DMA_ClearFlag(DMA1_FLAG_GL1|DMA1_FLAG_TC1);
	b485Send  = ComRx;	// Receive;	
}

void DMA1_Channel2_IRQHandler()
{
}


void TaskIO(void)
{
	IedDb.InputStatus.cInput16 	= 	GPIOD->IDR;
}

void UpdateOutPut(void)
{
	GPIOC->ODR = IedDb.OutputCoil.iCoil16 	;
}
 



#include "MODBUS.H"
#include "CTRL.H"   
#include "task.h"
//#include "DRVUART.H"
//#include "DrvPDMA.h"
#define USINGUART0
#define MAX_BUF_SIZE 512	
#define T15    18	 //18��bit				
uint8_t	    receTimeOut;		//���ճ�ʱ
uint8_t     TxTimeOut;			//����������ʱ
uint8_t	    sendBuf[MAX_BUF_SIZE],receBuf[MAX_BUF_SIZE]; //���ͽ��ջ�����	  
uint16_t   	*temp ; 					   	 
//uint8_t	    checkoutError;	// ==2 żУ���	   
//uint32_t 	dwTickCount;					  
//uint16_t  testRegister;	//���ڲ��� ��ַַ16
uint8_t	sendCount;		//�����ֽڸ���
uint8_t	receCount;	    //���յ����ֽڸ���
uint8_t	sendPosi;	    //����λ�� 

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


//�ֵ�ַ 0 - 255 (ֻȡ��8λ)
//λ��ַ 0 - 255 (ֻȡ��8λ)

/* CRC ��λ�ֽ�ֵ�� */
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
/* CRC��λ�ֽ�ֵ��*/
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
	uint8_t uchCRCHi = 0xFF ; /* ��CRC�ֽڳ�ʼ�� */
	uint8_t uchCRCLo = 0xFF ; /* ��CRC �ֽڳ�ʼ�� */
	uint32_t uIndex ; /* CRCѭ���е����� */
	while (usDataLen--) /* ������Ϣ������ */
	{
		uIndex = uchCRCHi ^ *puchMsg++ ; /* ����CRC */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	return (uchCRCHi << 8 | uchCRCLo) ;
}//uint16_t crc16(uint8_t *puchMsg, uint16_t usDataLen)

//������Ĵ���
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

	//bitCount = (receBuf[4]<<8) + receBuf[5];	//��ȡ��λ����
	bitCount = receBuf[5];

	byteCount = bitCount / 8;					//�ֽڸ���
	if(bitCount%8 != 0)
		byteCount++;

	for(k=0;k<byteCount;k++)
	{//�ֽ�λ��
		position = k + 3;
		sendBuf[position] = 0;
		for(i=0;i<8;i++)
		{
			getInputStatusVal(tempAddr,&tempData);

			sendBuf[position] |= tempData << i;
			tempAddr++;
			if(tempAddr >= addr+bitCount)
			{	//����
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

//����Ȧ״̬
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

	//bitCount = (receBuf[4]<<8) + receBuf[5];	//��ȡ��λ����
	bitCount = receBuf[5];

	byteCount = bitCount / 8;					//�ֽڸ���
	if(bitCount%8 != 0)
		byteCount++;

	for(k=0;k<byteCount;k++)
	{//�ֽ�λ��
		position = k + 3;
		sendBuf[position] = 0;
		for(i=0;i<8;i++)
		{
			getCoilVal(tempAddr,&tempData);

			sendBuf[position] |= tempData << i;
			tempAddr++;
			if(tempAddr >= addr+bitCount)
			{	//����
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
//������Ĵ���
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
//���Ĵ���
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

	//readCount = (receBuf[4]<<8) + receBuf[5];	//Ҫ���ĸ���
	readCount = receBuf[5];	//Ҫ���ĸ���
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


//ǿ�Ƶ�����Ȧ
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
	{	//��ΪON
		tempData = 1;
	}
	//else if(onOff == 0x0000)
	else if(onOff == 0x00)
	{	//��ΪOFF
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


//���ö���Ĵ���
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

//���uart0����

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
			case 1://��ȡ��Ȧ״̬(��ȡ�� 16λ����)
            case 2://��ȡ����Ĵ���
			case 3://��ȡ���ּĴ���(һ������)    
            case 4:
			case 5://ǿ�Ƶ�����Ȧ
			case 6://���õ����Ĵ���
					
					if(receCount >= 8)
					{//�������һ������
						//Ӧ�ùرս����ж�
                                                
						{
							crcData = crc16(receBuf,6);
							if(crcData == receBuf[7]+(receBuf[6]<<8))
							{//У����ȷ
								if(receBuf[1] == 1)
								{//��ȡ��Ȧ״̬(��ȡ�� 16λ����)
									readCoil();
								}
                                else if(receBuf[1] == 2)
                                {
                                    //��ȡ����ڵ�
                                    ReadInputStatus();
                                }
								else if(receBuf[1] == 3)
								{//��ȡ���ּĴ���(һ������)
									readRegisters();
								}
                                else if(receBuf[1] == 4)
								{//������Ĵ���
									readInputRegisters();
								}
								else if(receBuf[1] == 5)
								{//ǿ�Ƶ�����Ȧ
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

			case 15://���ö����Ȧ
					tempData = receBuf[6];
					tempData += 9;	//���ݸ���
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

			case 16://���ö���Ĵ���
					tempData = (((unsigned int )receBuf[4])<<8) + receBuf[5];
					tempData = tempData * 2;	//���ݸ���
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

//ȡ��Ȧ״̬ ����0��ʾ�ɹ�
uint16_t getCoilVal(uint16_t addr,uint8_t *tempData)
{					   
//	uint16_t result = 0;
	uint16_t tempAddr;
	unsigned char *pCoilPtr = 0;  
	unsigned int CoilAddr 	= 0;
	unsigned int CoilBit  	= 0;   

	tempAddr = addr & 0xFF;

	tempAddr = tempAddr%16;  //ֻ��8λ

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

//�趨��Ȧ״̬ ����0��ʾ�ɹ�
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

	tempAddr = tempAddr%16;  //ֻ��16λ

	CoilAddr = tempAddr/8;
	CoilBit  = tempAddr%8;
	pCoilPtr = &IedDb.OutputCoil.iCoil8; 
	pCoilPtr+= CoilAddr;
	if	(tempData) //���Ϊ1
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
//ȡ�Ĵ���ֵ ����0��ʾ�ɹ�
uint16_t getRegisterVal(uint16_t addr,uint16_t *tempData)
{

	uint16_t result = 0;
	uint16_t tempAddr; 
	tempAddr = addr & 0xfff;	   
    *tempData=temp[tempAddr];
	return result;
}//uint16_t getRegisterVal(uint16_t addr,uint16_t &data)
 
/******************************							  
modbus RTU ��C51����
ͨ�Ų����� 9600 8λ���� 1λֹͣλ żУ�� 485ͨλ�ӿ�
��Ƭ�����ư��ַ localAddr(����)
ͨ�ſ��������ݵĵ�ַ��
�ֵ�ַ 0 - 255 (ֻȡ16λ�ĵ�8λ)
λ��ַ 0 - 255 (ֻȡ16λ�ĵ�8λ)
*******************************/
														    
void TaskCommTimer(void)
{
	if(receTimeOut>0)
	{	
		--receTimeOut;				 			   
		if(receTimeOut==0 && receCount>0)   //�ж�ͨѶ�����Ƿ�ʱ
		{			  
            checkComm0Modbus(); 							 
			receCount = 0;      //�����յ�ַƫ�ƼĴ�������       
		}			
	}
	if(TxTimeOut>0)							//��ʱ������������
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

//��ʼ����														  
void beginSend(void)
{
	TxTimeOut = 2;
}//void beginSend(void)

void Send(void)
{ 
	b485Send = ComTx;	//��Ϊ����
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE); //���з����ж�   
}
//���üĴ���ֵ ����0��ʾ�ɹ�
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
	b485Send = ComTx;	//��Ϊ����
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
	//�˿ڳ�ʼ��
	// Configure USART1 Tx as alternate function open-drain 
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);  
		
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//���ڳ�ʼ��
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
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //DMA���.�����ж�
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
	  //ֻҪ�����ж�,״̬�־ͱ����.
	  uint32_t sr = UART1.SR;
	  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	  {
	  		receTimeOut = 5;     //ͨѶ��ʱֵ 
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
 


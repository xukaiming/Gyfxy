#include "EEPROM.H"		  
#include "main.h"
#include "ctrl.h"
//128K 
#define  STARTADDR     (FLASH_BASE+127*SECTOR_SIZE) 	  //0x0807FFFF
unsigned short PrintBuf[SECTOR_SIZE ];     //写入EEPROM缓冲区
u16 EEHalfWord_Read(u32 EEAddr) 
{		  		    
	return *(vu16*)EEAddr;
}

void EESector_Erase(u32 EEAddr)  
{	
	EEAddr=(EEAddr/SECTOR_SIZE)*SECTOR_SIZE	;
	FLASH_ErasePage(EEAddr);  
}

				    								 
void EEHalfWord_Write(u32 EEAddr,u16 EEData)   
{									    
	FLASH_ProgramHalfWord( EEAddr, EEData)  ;  
}

////Addr 写入FLASH的首地址
uint32_t EEWriteStruct(void* pBuf,uint32_t cLen,uint32_t iAddr)	 
{
	uint8_t cResult =0;
	u32 ProtectedPages;		
	u32 Addr = 0;	
	u32 i,j,k=0;
	u32 iBufStart,iBaseAddr,cBufCnt;
	cLen>>=1;
	Addr=STARTADDR;
	Addr=iAddr;
	Addr+=STARTADDR;
	iBaseAddr  		= Addr%(SECTOR_SIZE<<1)+STARTADDR;                  //计算缓冲区首地址	 
	iBufStart  		= iAddr/(SECTOR_SIZE<<1)*(SECTOR_SIZE<<1);
	cBufCnt   	= (iBufStart+cLen)/SECTOR_SIZE+1;   //要写入的缓冲区次数	 

	for(j=0;j<cBufCnt;j++)
	{		
		//全部锁定则退出编程
		Flash_ProtectMode(&ProtectedPages );  //
		if(0==ProtectedPages)
			break;
		//读出一个扇区到保存缓冲区
		for(i=0;i<SECTOR_SIZE;i++)
			PrintBuf[i] = EEHalfWord_Read(iBaseAddr+SECTOR_SIZE*j+(i<<1));
		//将要保存的数据写入到缓冲区   					
		for(i=(j==0?iBufStart:0);i<SECTOR_SIZE;i++,k++) {
			PrintBuf[i] = *((vu16*)pBuf+k);				//
			if(--cLen==0){  
			   //__nop();
			   break;}	 
		}	
				    								    
		//擦除Flash区 
		FLASH_Unlock();
		EESector_Erase(Addr+j*SECTOR_SIZE);								//擦除扇区
		//写入内容;
		for(i=0;i<SECTOR_SIZE;i++)
		{
			 EEHalfWord_Write(iBaseAddr+SECTOR_SIZE*j+(i<<1),PrintBuf[i]);   
		}	   	
		FLASH_Lock();
	}			   		   
	return cResult;
}

void  EEReadStruct(void* pBuf,uint32_t cLen,uint32_t Addr)
{
	uint32_t i;
	
	uint16_t *pcBuf = (uint16_t*)pBuf;
 	cLen>>=1;
	Addr+=STARTADDR;
	for(i=0;i<cLen;i++)
	{
		pcBuf[i]=EEHalfWord_Read(Addr+(i<<1));
	}
}

/**************************************************************************************
    Save Ied Id	to X5043
    address=0,
    1st	byte=0x55--IedID active,
    2nd	byte=IedId,
    3rd	byte=non
**************************************************************************************/
void SaveIdToEE(uint8_t cID)
{						  	
	unsigned short buf[3]={EEPROM_ID_ACTIVE,0,0};
	buf[1] = cID;
	EEWriteStruct(buf,sizeof(buf),EEPROM_ID_ADDR);
	IedDb.cID=cID;; 
}
/**************************************************************************************
    Read Ied Id	from X5043
    address=0,
    1st	byte=0x55-IedID	active,
    2nd	byte=IedId,
    3rd	byte=non
**************************************************************************************/
void ReadIdFromEE(void)
{ 
	unsigned short  buf[3];
	EEReadStruct(buf,sizeof(buf),EEPROM_ID_ADDR);	 	 
	
    if(buf[0]==EEPROM_ID_ACTIVE)
 		IedDb.cID=buf[1];	    
    else
		IedDb.cID=0xFF;	//0xff			
			
}

void SaveEvalCnt(uint32_t cnt)
{					
	unsigned short  buf[3]={EEPROM_ID_ACTIVE,0,0};
	ReadIdFromEE();
	///buf[1] = IED.nId;
	buf[2] = cnt;
	EEWriteStruct(buf,sizeof(buf),EEPROM_ID_ADDR);	 
}

uint32_t ReadEvalCnt()
{					
	unsigned short  buf[3]={EEPROM_ID_ACTIVE,0,0};
	ReadIdFromEE();									    
	return buf[2];
}

void Flash_ProtectMode(u32 *ProtectedPages)
{
	vu32 WRPR_Value;
  	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);

  	/* Get pages write protection status */
  	WRPR_Value = FLASH_GetWriteProtectionOptionByte();
  	*ProtectedPages = WRPR_Value & 0x000000C0;

#ifdef WriteProtection_Disable
  	if (ProtectedPages == 0x00)
  	{// Pages are write protected  

    	// Disable the write protection 
    	FLASHStatus = FLASH_EraseOptionBytes();

    	// Generate System Reset to load the new option byte values  
    	NVIC_GenerateSystemReset();
  	}
#else
 	#ifdef WriteProtection_Enable
  
  	if (ProtectedPages != 0x00)
  	{	// Pages not write protected  
    	#ifdef USE_STM3210B_EVAL
   	 	// Enable the pages write protection  
    	FLASHStatus = FLASH_EnableWriteProtection(FLASH_WRProt_Pages24to27 |FLASH_WRProt_Pages28to31);    
    	#else   
    	// Enable the pages write protection  
    	FLASHStatus = FLASH_EnableWriteProtection(FLASH_WRProt_Pages12to13 |FLASH_WRProt_Pages14to15);
    	#endif
    	// Generate System Reset to load the new option byte values  
    	NVIC_GenerateSystemReset();
  	}
 	#endif
#endif
}

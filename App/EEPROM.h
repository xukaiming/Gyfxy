#ifndef _EEPROM_H_
#define _EEPROM_H_
#include "task.h"
/////////////////////////////////////////////////////////////////////////	 
#define SECTOR_SIZE 512
#define	EEPROM_ID_ACTIVE    0x55    //0x55-ID active	  
#define	EEPROM_ID_ADDR	    0UL 	//save ID start address
/////////////////////////////////////////////////////////////////////////  
#define IAP_DISABLE	  FLASH_Lock
#define IAP_ENABLE	  FLASH_Unlock


  
/////////////////////////////////////////////////////////////////////////
u16 EEHalfWord_Read(u32 EEAddr) ;
void EESector_Erase(u32 EEAddr); 
void EEHalfWord_Write(u32 EEAddr,u16 EEData);  	 //返回0 ,写入失败
uint32_t EEWriteStruct(void* pBuf,uint32_t cLen,uint32_t iAddr); 		 //写入结构体
void  EEReadStruct(void* pBuf,uint32_t cLen,uint32_t Addr);		 //读取结构体
void ReadIdFromEE(void);  
void SaveIdToEE(uint32_t cID) ;
void Flash_ProtectMode(u32 *ProtectedPages);
void SaveEvalCnt(uint32_t cnt);
uint32_t ReadEvalCnt(void);
#endif

#include "key.h" 
#include "Task.H"			 
#include "ctrl.h"
#define KEYPORT (GPIOD->IDR&0xFF) 
#define KEYMASK  0xFF
unsigned char gcOldKey = 0;
unsigned char gcKeyFilter = 0;
unsigned char gcKeyStack[MAX_KEY_STACK] = {"\0"};
unsigned char gcKeySavePtr= 0;
unsigned char gcKetGetPtr = 0;
signed   char gcBLTimer 			  = -1;		     
unsigned char gcKey = 0;								 //键盘键值										    

void TaskScanKey(void)
{		
		unsigned char Key = KEYPORT; 	//按键输入     //去掉LOCK,上下到位  半降全降信号			  
        Key = KEYPORT&KEYMASK;
		if(Key!=0x0)
        {										  
            if(Key!=gcOldKey)    			//digit filter
            {
                gcOldKey=Key;
                gcKeyFilter=0;
				//beep.beep(1,100); 
            }
            else
            {								 //长按不放的情况
                gcKeyFilter++;
				 
                if( gcKeyFilter>MAX_KEY_FILTER)        //save key to stack
                {			   															                     
					if((Key&KEY_ENT)!=KEY_ENT)	   //键值过滤  确认键不允许连击
					{	
						gcKeyFilter=0;	   
                    	gcKeyStack[gcKeySavePtr]=Key;
                    	gcKeySavePtr=(gcKeySavePtr+1)%MAX_KEY_STACK;  
                     	//beep.beep(1,50);           //beep 10ms					 
					}	   
                } 
            }             
        }
		else
		{
			if(gcKeyFilter>2)			//延时去抖动 ,按键松开的情况
			{
                    gcKeyFilter=0;
                    gcKeyStack[gcKeySavePtr]=gcOldKey;
                    gcKeySavePtr=(gcKeySavePtr+1)%MAX_KEY_STACK;
                    //beep.beep(1,100);           //beep 10ms				  
			}	 
		}
		#ifdef _DEBUG								 
		//printf("Key = 0x%X \n",(int)Key);	
		#endif
}
																				  
														   
/**************************************************************************
    get key code,return 0 if no key
***************************************************************************/	  
unsigned char GetKey(void)
{
    UCHAR ret;

    if(gcKetGetPtr!=gcKeySavePtr)
    {
        ret=gcKeyStack[gcKetGetPtr];
        gcKetGetPtr=(gcKetGetPtr+1)%MAX_KEY_STACK;
    }
    else
        ret=0;	
    return ret;
}


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
unsigned char gcKey = 0;								 //���̼�ֵ										    

void TaskScanKey(void)
{		
		unsigned char Key = KEYPORT; 	//��������     //ȥ��LOCK,���µ�λ  �뽵ȫ���ź�			  
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
            {								 //�������ŵ����
                gcKeyFilter++;
				 
                if( gcKeyFilter>MAX_KEY_FILTER)        //save key to stack
                {			   															                     
					if((Key&KEY_ENT)!=KEY_ENT)	   //��ֵ����  ȷ�ϼ�����������
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
			if(gcKeyFilter>2)			//��ʱȥ���� ,�����ɿ������
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

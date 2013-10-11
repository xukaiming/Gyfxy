#ifndef _KEY_H_
#define _KEY_H_
		     
 
 
#define KEY_ENT  		0x01
#define MAX_KEY_STACK	10
#define MAX_KEY_FILTER  4
void TaskScanKey(void);

extern   unsigned char gcKey;	   
unsigned char GetKey(void)	;
#endif

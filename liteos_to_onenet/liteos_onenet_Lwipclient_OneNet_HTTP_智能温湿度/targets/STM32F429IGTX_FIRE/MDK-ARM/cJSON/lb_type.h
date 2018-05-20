#ifndef _LB_TYPE_H_
#define _LB_TYPE_H_

typedef unsigned char  	INT8U;         //�޷���8λ��
typedef signed   char  	INT8S;         //�з���8λ��
typedef unsigned short  INT16U;        //�޷���16λ��
typedef signed   short  INT16S;        //�з���16λ��
typedef unsigned int    INT32U;        //�з���16λ��

typedef INT8U			uchar;
typedef INT16U			uint;

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

#ifndef TRUE
#define TRUE  (1==1)
#endif

#ifndef FALSE
#define FALSE (1==0)
#endif

#ifndef NULL
	#define NULL (void *)0	   /*��ָ��*/
#endif

#define __In_
#define __Out_
#define __Inout_

#endif

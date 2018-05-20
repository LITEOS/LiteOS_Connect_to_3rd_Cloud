#ifndef _LB_TIMER_H_
#define _LB_TIMER_H_

#include "lb_type.h"
#include "CriticalSeg.h"
#ifdef _LB_TIMER_C_
#define LB_TIMER_EXT
#else
#define LB_TIMER_EXT extern
#endif

#ifndef LB_TRUE
	#define LB_TRUE	1
#endif

#ifndef LB_FALSE
	#define LB_FALSE	0
#endif

#ifndef LB_NULL
	#define LB_NULL (void *)0	   /*��ָ��*/
#endif

#define XRAM 			 /*�ⲿRAM����*/

#define LB_MAX_TIMER	20	   //��ʱ��������

typedef void *TIMER_CB(void *msg);	 	//�ص���������
typedef struct LB_TMRCB LB_TMRCB;	//��ʱ�����ƿ�ṹ�����Ͷ���

//��ʱ�����ƿ�ṹ��
struct LB_TMRCB{
	INT16U		interval;	 //ʱ����
	INT16U		cnt;		 //������
	INT8U		enable;		 //ʹ�ܿ���λ
	INT8U		ok;			 //ʱ�䵽���
	TIMER_CB	*fCBack;	 //�ص�����
	TIMER_CB	*fCBackInt;
        void            *msg;
	LB_TMRCB	*pNext;		 //����ָ��
};

/*�ں������ڴ棬���޸�*/
#ifdef _LB_TIMER_C_
static XRAM LB_TMRCB LB_TMRCBTbl[LB_MAX_TIMER];	//��ʱ����
#endif

LB_TIMER_EXT void InitLB_TMR(void);	 //��ʼ����ʱ��
LB_TIMER_EXT LB_TMRCB *LB_TMRCreate(INT16U interval,INT8U enable,TIMER_CB *TMRCallBack,TIMER_CB *TMRCallBackInt,void *msg);	//������ʱ��
LB_TIMER_EXT void LB_TMRTick(void);	 //��ʱ�����
LB_TIMER_EXT void LB_ExcTMR(void);	 //ִ�ж�ʱ��
LB_TIMER_EXT void SetLB_TMR(INT16U interval,LB_TMRCB *pTMRCB);	 //���ö�ʱ�����
LB_TIMER_EXT void EnLB_TMR(LB_TMRCB *pTMRCB);					 //ʹ�ܶ�ʱ��
LB_TIMER_EXT void DisLB_TMR(LB_TMRCB *pTMRCB);					 //��ֹ��ʱ��
LB_TIMER_EXT void ReCntLB_TMR(LB_TMRCB *pTMRCB); 				 //���¼���
LB_TIMER_EXT INT8U GetTimerNum(void);

#endif

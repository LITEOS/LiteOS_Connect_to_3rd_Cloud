#define _WDT_C_
#include "sys1.h"

void IwdgConfiguration(void)
{
 /* д��0x5555,�����������Ĵ���д�빦�� */
 IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
 
 /* ����ʱ�ӷ�Ƶ,40K/256=156HZ(6.4ms)*/
 IWDG_SetPrescaler(IWDG_Prescaler_256);
 
 /* ι��ʱ�� 640MS .ע�ⲻ�ܴ���0xfff*/
 IWDG_SetReload(100);
 
 /* ι��*/
 IWDG_ReloadCounter();
 
 /* ʹ�ܹ���*/
 IWDG_Enable();
}

void WdtClr(void)
{
  IWDG_ReloadCounter();
}

/*ģ��ṹ���ʼ��*/
void WdtStructInit(void)
{

}

/*ģ���ʼ��*/
void WdtInit(void)
{

	WdtStructInit();
	IwdgConfiguration();

}


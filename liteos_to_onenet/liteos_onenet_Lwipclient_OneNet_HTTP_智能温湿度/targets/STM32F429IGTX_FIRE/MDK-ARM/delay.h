#ifndef __DELAY_H
#define __DELAY_H 			   
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

/*
*���ܣ�STM32F407VET6��̫�����İ���Դ��룬�Զ���ȡIP������������������ͨ������1��ӡ��ǰ������Ϣ
*��������������(www.zkaifa.com)
*��̳��www.zkaifa.com/bbs
*���ߣ�liubinkaixin
*ʱ�䣺2015-05-30
*��������ǰ�汾���ṩ���İ���ԣ����ṩ�о�ѧϰʹ�ã�����Ϊ��ҵ��;�����κδ����ṩ�����е��κ�����
*/

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif






























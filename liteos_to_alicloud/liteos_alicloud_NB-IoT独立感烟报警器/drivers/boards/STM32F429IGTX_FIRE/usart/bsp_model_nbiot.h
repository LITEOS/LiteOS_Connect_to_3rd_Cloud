#ifndef __BSP_MODEL_IOT_H
#define __BSP_MODEL_IOT_H

#include "stm32f4xx.h"
#include "bsp_model_usart.h"
#include "los_typedef.h"



typedef enum{
    MODEL_TRUE,
    MODEL_FALSE,
    
}model_res_e;


#define     MODEL_TX(cmd)                 MODEL_USART_printf("%s", cmd)
#define     MODEL_DELAY(time)             LOS_TaskDelay(time)                 //��ʱ

extern  UINT8 model_cmd(char *cmd, char *reply, UINT32 waittime);
extern  UINT8 model_cmd_check(char *reply, UINT32 waittime);

/*************************** ���� ���� ***************************/
UINT8 model_init(void);
UINT8 model_iot_init(void);                                                             //GPRS��ʼ������
INT8 model_iot_udp_create(char localport);               //UDP����
INT8 model_iot_udp_send(int fd, char * serverip, UINT16 serverport, unsigned char * str, unsigned int datalen);//��������
UINT8 model_iot_udp_close(int fd);
UINT16 model_iot_udp_recv(int fd, void *pRet, unsigned int maxLen, unsigned char *pRetAddr, int *pRetPort);
void nbiot_tran_send(void);
void nbiot_tran_recv(void);

#endif


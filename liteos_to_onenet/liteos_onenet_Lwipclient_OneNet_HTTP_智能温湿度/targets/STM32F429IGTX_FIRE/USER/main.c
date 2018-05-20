/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   TCP Client����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����  STM32 F429 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
//#include "stm32f4xx.h"
#include "bsp_led.h" 
#include "./Bsp/usart/bsp_debug_usart.h"
#include "./Bsp/systick/bsp_SysTick.h"
#include "bsp_key.h"
#include "lwip/tcp.h"
#include "netconf.h"
#include "LAN8742A.h"
#include "tcp_echoclient.h"
#include "bsp_dht11.h"

#include "stdlib.h"
#include "string.h"
#include "los_base.h"
#include "los_config.h"
#include "los_typedef.h"
#include "los_hwi.h"
#include "los_task.ph"
#include "los_sem.h"
#include "los_event.h"
#include "los_memory.h"
#include "los_queue.ph"
#include "cmsis_os.h"
#include <stdio.h>
#include "ds18b20.h"
#include "stm32f4xx_conf.h"
#include "dwt.h"
#include "cjson.h"
#include "malloc.h"
#include "dwt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
KEY Key1,Key2;
UINT32 g_TskHandle;
 cJSON *json;
extern __IO uint8_t EthLinkStatus;
extern __IO uint32_t LocalTime; /* this variable is used to create a time reference incremented by 10ms */
/* Private function prototypes -----------------------------------------------*/
static void TIM3_Config(uint16_t period,uint16_t prescaler);
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
void TIM3_IRQHandler(void);
void hieth_hw_init()
{

}
extern void HAL_ETH_IRQHandler(void);
void hardware_init(void)
{
	LED_GPIO_Config();
	Key1_GPIO_Config();
	Key2_GPIO_Config();
	KeyCreate(&Key1,GetPinStateOfKey1);
	KeyCreate(&Key2,GetPinStateOfKey2);
	Debug_USART_Config();
	DS18B20_GPIO_Config();
	DelayInit(SystemCoreClock);
	LOS_HwiCreate(TIM3_IRQn, 0,0,TIM3_IRQHandler,NULL);
	LOS_HwiCreate(ETH_IRQn, 1,0,HAL_ETH_IRQHandler,0);
	TIM3_Config(999,899);
	printf("Sysclock is %d\r\n",SystemCoreClock);
		
	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
  ETH_BSP_Config();	
  printf("LAN8720A BSP INIT AND COMFIGURE SUCCESS\n");
	DHT11_GPIO_Config();
  
}	
	
VOID task1()
{
	int count = 0;
	while(1)
	{
		count++;
		printf("------This is task1,count is %d\r\n",count);
		LOS_TaskDelay(2000);
	}
}


UINT32 creat_task1()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "task1";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task1;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}
UINT32 mainDHT11task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "mainDHT11";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)mainDHT11;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}
VOID task2()
{
	int count = 0;
	while(1)
	{
		count++;
		printf("++++++++This is task2,count is %d\r\n",count);
		LOS_TaskDelay(1000);
	}
}


UINT32 creat_task2()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "task2";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task2;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}
const char text1[]="{\n\"Project\": \"HuaweiLiteOS oneNET\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";	

VOID task3()
{
	int count = 0;
	int count1 = 0;
	int count2 = 0;
	int sum=0;
	unsigned int count3 = 0;
	uint8_t flag=0;
	cJSON *tmp;
	printf("LAN8720A Ethernet Demo\n");
  printf("    KEY1: ����TCP����\n");
  printf("    KEY2: �Ͽ�TCP����\n");
  //IP��ַ�Ͷ˿ڿ���netconf.h�ļ��޸ģ�����ʹ��DHCP�����Զ���ȡIP(��Ҫ·����֧��)
  printf("����IP�Ͷ˿�: %d.%d.%d.%d\n",IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  printf("Զ��IP�Ͷ˿�: %d.%d.%d.%d:%d\n",DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3,DEST_PORT);
	/* Initilaize the LwIP stack */
  LwIP_Init();	
	json=cJSON_Parse(text1);
	if (json != NULL)
	{
		
		//tmp = cJSON_GetObjectItem(json,"format");
		tmp = cJSON_GetObjectItem(json,"Project");
		if(tmp != NULL)
		{			
			printf("cJSON:Project=%s\n",tmp->valuestring);	
			//cJSON_Delete(tmp);
		}
	}

   tcp_echoclient_connect();
	int flag1=0;
	int flag2=1;
	while(1)
	{
		Key_RefreshState(&Key1);//ˢ�°���״̬
		Key_RefreshState(&Key2);//ˢ�°���״̬
		if(count1++>30500)	
		{		
			mainDHT11();count1=0;
		}
		if(count++>30500&&flag2==1)
		{
			GETdata();//��ȡ����
			count=0;		
			flag1=1;
			flag2=0;	
		}
		if(count2++>30500&&flag1==1)
		{		
			MCU_to_TCP();//�ϴ�����
		  flag1=0;
			flag2=1;
			count2=0;			
		}
		recDataAnalyze();//�������յ�������   
   if((Key_Scan(KEY1_GPIO_PORT,KEY1_PIN)==KEY_ON) )
		 {		 
		   	Delay10ms(100);
				 MCU_to_TCP();
		 }
    if(flag==0)//(Key_Scan(KEY1_GPIO_PORT,KEY1_PIN)==KEY_ON) && 
		{
			LED33=~LED33;
			if (EthLinkStatus == 0)
			{
        printf("connect to tcp server\n");
				/*connect to tcp server */ 
        tcp_echoclient_connect();//���ӷ�����
				flag=1;
			}
		}	
		 if(count3++>500000)//��������������ֹ�������ݳ���ʱ�Ͽ�����
			 {
				 sum++;
				 
				 printf("RESET tcp server:%d\n",sum);
			  tcp_echoclient_disconnect();//�Ͽ�������
			  LOS_TaskDelay(2);
		 /*connect to tcp server */ 
        tcp_echoclient_connect();//���ӷ�����
			  LOS_TaskDelay(2);
				 count3=0;
		 }
		if((Key_Scan(KEY2_GPIO_PORT,KEY2_PIN)==KEY_ON) && flag==1)
		{
			LED33=~LED33;
			tcp_echoclient_disconnect();
			flag=0;
		}	
		/* check if any packet received */
    if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(LocalTime);
	}
}



UINT32 creat_task3()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "task3";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task3;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}
UINT32 creat_task_18b20()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "maindsp18b20";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)maindsp18b20;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}
/*UINT32 creat_MCU_TO_TCP()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "GETdata";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)GETdata;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}*/




int main(void)
{
    UINT32 uwRet = LOS_OK;
    LOS_KernelInit();//?????	
    hardware_init();//?????
   // uwRet = creat_task1();
    //if(uwRet != LOS_OK)
    //{
     //   return uwRet;
    //}
		
		/*uwRet = creat_task_18b20();
    if(uwRet != LOS_OK)
    {
       return uwRet;
    }*/
		
		
		uwRet = creat_task3();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
	/*	uwRet = mainDHT11task();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }*/
		/*uwRet = creat_MCU_TO_TCP();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }*/
		
    LOS_Start();//??LiteOS
}


/**
  * @brief  ͨ�ö�ʱ��3�жϳ�ʼ��
  * @param  period : �Զ���װֵ��
  * @param  prescaler : ʱ��Ԥ��Ƶ��
  * @retval ��
  * @note   ��ʱ�����ʱ����㷽��:Tout=((period+1)*(prescaler+1))/Ft us.
  *          Ft=��ʱ������Ƶ��,ΪSystemCoreClock/2=90,��λ:Mhz
  */
static void TIM3_Config(uint16_t period,uint16_t prescaler)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=prescaler;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=period;   //�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  ��ʱ��3�жϷ�����
  * @param  ��
  * @retval ��
  */
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		LocalTime+=10;//10ms����
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

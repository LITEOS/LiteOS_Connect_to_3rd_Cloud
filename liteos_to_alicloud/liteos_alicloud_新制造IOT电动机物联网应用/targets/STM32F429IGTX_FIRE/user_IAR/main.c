/* Includes LiteOS------------------------------------------------------------------*/
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
//#include "cmsis_os.h"  /** TY ����ʹ�� IAR �Դ��� CMSIS, ����ʹ�� $PROJ_DIR$\kernel\compat\cmsis */
#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "bsp_led.h" 
#include "bsp_debug_usart.h"
#include "dwt.h"
#include "bsp_key.h"
#include "LAN8742A.h"
#include "netconf.h"
#include "modbus.h"

/* extern variables ----------------------------------------------------------*/
extern __IO UINT8 EthLinkStatus;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Extern variables ---------------------------------------------------------*/
extern TBuffModbus stBufModbus;

/* Private variables ---------------------------------------------------------*/
KEY Key1,Key2;
UINT32 g_TskHandle;              /** TY Record the Task's ID, when create Task...*/
__IO uint32_t LocalTime = 0;     /** this variable is used to create a time reference incremented by 10ms */
/** Aliyun MQTT */
__IO u8_t uDeviceStateDHCP = 0;  /** Ϊ1ʱ��ʾͨ�� DHCP get�� ip */
__IO u8_t uDeviceStateDNS = 0;   /** Ϊ1ʱ��ʾ�Ѿ��õ� Aliyun MQTT server �� IP */

extern int mqtt_client(void);

/* Private function prototypes -----------------------------------------------*/
static void TIM3_Config(uint16_t period,uint16_t prescaler);
/* Private functions ---------------------------------------------------------*/
void TIM3_IRQHandler(void);
static void hardware_init(void);
static void software_init(void);

/* Function ------------------------------------------------------------------*/
static void software_init(void)
{
    /** Initilaize the LwIP stack */
    LwIP_Init();
    printf("Initilaize the LwIP stack SUCCESS.\r\n");

    /** OS tick ����֮���ӹ� ģ��osʱ�� TDateTime stNowTime, �����ȳ�ʼ��һ�� */
    
}

static void hardware_init(void)
{
	LED_GPIO_Config();
	
	/* ��ʼ�����Դ��ڣ�һ��Ϊ����1; ��ʼ�� USART2-RS485 */
	Debug_USART_Config();
    RS485_USART2_Config();
	DelayInit(SystemCoreClock);
    /** @2018-05-17 ���ڲ���Ҫ TIM3,�������ﲻ���� tim3 */
//	LOS_HwiCreate(TIM3_IRQn, 0,0,TIM3_IRQHandler,NULL);
//	TIM3_Config(999,899);
//	printf("Sysclock is %d\r\n",SystemCoreClock);

    /** TY Configure ethernet, such as, ETH_GPIO,  ETH_MACDMA... */ 
    ETH_BSP_Config();
    printf("LAN8720A BSP INIT AND COMFIGURE SUCCESS.\r\n");
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
		LocalTime+=10; //10ms����		
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}

/***/
VOID modbus_task()
{
    memset(&stBufModbus, 0, sizeof(TBuffModbus));
    /** modbus �̻��Բ��豸��ַ  */ 
    stBufModbus.address= MODBUS_ADDRESS_SLAVE;    
    /** ���� modbus ʱ,�� enable ���ڽ��� */
    RS485_RX_ON_TX_OFF;

	while(1)
	{	           	    
    	LED3_OFF;        
    	LED2_OFF;
    	LED1_ON;
        /** RS485 - Modbus Cycle */
        ModbusProc_InTask(&stBufModbus);
                
		LOS_TaskDelay(2000); // Printf Per 2s 
	}
}

/**
  * ����: 1.)  ͨ�� rs485 ���մӻ����͵� '�綯��' ״̬��Ϣ
  *       1.1) �������״̬: ͣ��, ���� 2 Byte 
  */
UINT32 create_Modbus_Task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 5;
    task_init_param.pcName = "Modbus_Task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)modbus_task;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;        
}

UINT32 create_MQTT_client_task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 4;
    task_init_param.pcName = "mqtt_client_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)mqtt_client;
    task_init_param.uwStackSize = 0x1400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}

/** Crate DHCP Task, Call Lwip's netconf.c */
UINT32 create_DHCP_task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 3;
    task_init_param.pcName = "DHCP_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)LwIP_DHCP_task;
    task_init_param.uwStackSize = 0x640;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}

/* Entry - main() */
int main(void)
{    
    UINT32 uwRet = LOS_OK;
    LOS_KernelInit(); //�ں˳�ʼ��	
    hardware_init();  //Ӳ����ʼ��
    software_init();

    uwRet = create_DHCP_task(); /** 1. first created DHCP task */
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
   
    uwRet = create_Modbus_Task();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
		
	uwRet = create_MQTT_client_task();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }

    LOS_Start();    //����LiteOS
}

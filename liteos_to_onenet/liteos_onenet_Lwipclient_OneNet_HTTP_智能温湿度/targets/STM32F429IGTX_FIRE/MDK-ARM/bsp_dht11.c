/**
  ******************************************************************************
  * @file    bsp_dht11.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   ��ʪ�ȴ�����Ӧ�ú����ӿ�
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����  STM32 F429 ������  
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
#include "bsp_dht11.h"
#include "utils.h"
#include "stm32f4xx.h"
#include "bsp_led.h" 
#include "./Bsp/usart/bsp_debug_usart.h"
#include "./Bsp/systick/bsp_SysTick.h"
#include "dwt.h"
/*
 * ��������DHT11_GPIO_Config
 * ����  ������DHT11�õ���I/O��
 * ����  ����
 * ���  ����
 */
void DHT11_GPIO_Config(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����DHT11_PORT������ʱ��*/
	RCC_AHB1PeriphClockCmd(DHT11_CLK, ENABLE); 

	/*ѡ��Ҫ���Ƶ�DHT11_PORT����*/															   
  	GPIO_InitStructure.GPIO_Pin = DHT11_PIN;	

	/*��������ģʽΪͨ���������*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
  
  /*�������ŵ��������Ϊ�������*/
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  
  /*��������Ϊ����ģʽ*/
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
	/*������������Ϊ50MHz */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*���ÿ⺯������ʼ��DHT11_PORT*/
  GPIO_Init(DHT11_PORT, &GPIO_InitStructure); 
}

/*
 * ��������DHT11_Mode_IPU
 * ����  ��ʹDHT11-DATA���ű�Ϊ��������ģʽ
 * ����  ����
 * ���  ����
 */
static void DHT11_Mode_IPU(void)
{
 	  GPIO_InitTypeDef GPIO_InitStructure;

	  	/*ѡ��Ҫ���Ƶ�DHT11_PORT����*/	
	  GPIO_InitStructure.GPIO_Pin = DHT11_PIN;

	   /*��������ģʽΪ��������ģʽ*/ 
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN ; 
  
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  	/*������������Ϊ50MHz */   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  
	  /*���ÿ⺯������ʼ��DHT11_PORT*/
	  GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 
}

/*
 * ��������DHT11_Mode_Out_PP
 * ����  ��ʹDHT11-DATA���ű�Ϊ�������ģʽ
 * ����  ����
 * ���  ����
 */
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;

	/*ѡ��Ҫ���Ƶ�DHT11_PORT����*/															   
  GPIO_InitStructure.GPIO_Pin = DHT11_PIN;	

	/*��������ģʽΪͨ���������*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
  
  /*�������ŵ��������Ϊ�������*/
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  
  /*��������Ϊ����ģʽ*/
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
	/*������������Ϊ50MHz */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*���ÿ⺯������ʼ��DHT11_PORT*/
  	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 	 
}

/* 
 * ��DHT11��ȡһ���ֽڣ�MSB����
 */
static uint8_t Read_Byte(void)
{
	uint8_t i, temp=0;

	for(i=0;i<8;i++)    
	{	 
		/*ÿbit��50us�͵�ƽ���ÿ�ʼ����ѯֱ���ӻ����� ��50us �͵�ƽ ����*/  
		while(DHT11_DATA_IN()==Bit_RESET);

		/*DHT11 ��26~28us�ĸߵ�ƽ��ʾ��0������70us�ߵ�ƽ��ʾ��1����
		 *ͨ����� x us��ĵ�ƽ��������������״ ��x ���������ʱ 
		 */
		Delayus(40); //��ʱx us �����ʱ��Ҫ��������0������ʱ�伴��	   	  

		if(DHT11_DATA_IN()==Bit_SET)/* x us����Ϊ�ߵ�ƽ��ʾ���ݡ�1�� */
		{
			/* �ȴ�����1�ĸߵ�ƽ���� */
			while(DHT11_DATA_IN()==Bit_SET);

			temp|=(uint8_t)(0x01<<(7-i));  //�ѵ�7-iλ��1��MSB���� 
		}
		else	 // x us��Ϊ�͵�ƽ��ʾ���ݡ�0��
		{			   
			temp&=(uint8_t)~(0x01<<(7-i)); //�ѵ�7-iλ��0��MSB����
		}
	}
	return temp;
}
/*
 * һ�����������ݴ���Ϊ40bit����λ�ȳ�
 * 8bit ʪ������ + 8bit ʪ��С�� + 8bit �¶����� + 8bit �¶�С�� + 8bit У��� 
 */
uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{  
  uint16_t count;
	/*���ģʽ*/
	DHT11_Mode_Out_PP();
	/*��������*/
	DHT11_DATA_OUT(DHT11_LOW);
	/*��ʱ18ms*/
	Delayus(20000);

	/*�������� ������ʱ30us*/
	DHT11_DATA_OUT(DHT11_HIGH); 

	Delayus(30);   //��ʱ30us

	/*������Ϊ���� �жϴӻ���Ӧ�ź�*/ 
	DHT11_Mode_IPU();

	/*�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������*/   
	if(DHT11_DATA_IN()==Bit_RESET)     
	{
    count=0;
		/*��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���*/  
		while(DHT11_DATA_IN()==Bit_RESET)
    {
      count++;
      if(count>1000)  return 0;
      Delayus(10); 
    }    
    
    count=0;
		/*��ѯֱ���ӻ������� 80us �ߵ�ƽ �����źŽ���*/
		while(DHT11_DATA_IN()==Bit_SET)
    {
      count++;
      if(count>1000)  return 0;
      Delayus(10); 
    }  
		/*��ʼ��������*/   
		DHT11_Data->humi_int= Read_Byte();

		DHT11_Data->humi_deci= Read_Byte();

		DHT11_Data->temp_int= Read_Byte();

		DHT11_Data->temp_deci= Read_Byte();

		DHT11_Data->check_sum= Read_Byte();

		/*��ȡ���������Ÿ�Ϊ���ģʽ*/
		DHT11_Mode_Out_PP();
		/*��������*/
		DHT11_DATA_OUT(DHT11_HIGH);

		/*����ȡ�������Ƿ���ȷ*/
		if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
			return 1;
		else 
			return 0;
	}
	else
	{		
		return 0;
	}   
}




/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
DHT11_Data_TypeDef DHT11_Data1;
extern int temperature1;
extern int humidity1;
char post_data1[255]={0};
extern char post_data2[255];//��tcp_echoclient.c��MCU_TO_TCP()�����е���
int count=0;
//�޸��Լ����豸ID��APIkey
char post_head[]={"POST /devices/29702274/datapoints?type=3 HTTP/1.1\r\napi-key: TnWg0VdW6kIhU9=6I9AewR=TxKQ=\r\nHost: api.heclouds.com\r\nContent-Length:"};
void mainDHT11(void)
{	
		/*����DHT11_Read_TempAndHumidity��ȡ��ʪ�ȣ����ɹ����������Ϣ*/
		if( Read_DHT11 ( & DHT11_Data1 ) == SUCCESS)
		{
			LED33=~LED33;
			temperature1=DHT11_Data1.temp_int;
			humidity1=DHT11_Data1.humi_int;
			sprintf(post_data1,"{\"temperature\":%d,\"humidity\":%d,\"Count\":%d}",temperature1,humidity1,count++);//,,\"LED1\":%d(int)LED33);
			sprintf(post_data2,"%s%d\r\n\r\n%s",post_head,strlen(post_data1),post_data1);
			printf("\r\n��ȡDHT11�ɹ�!\r\n\r\n%s,len:[%d]",post_data1,strlen(post_data1));
			if(count==100000)count=0;//
		}		
		else
		{			
		  printf("Read DHT11 ERROR!\r\n");
	  
		}
}






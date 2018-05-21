/**
 * Init USART2 for Modbus RS485
 */

#ifndef __RS485_USART_H
#define	__RS485_USART_H

/** std lib */
#include "stm32f4xx.h"
#include <stdio.h>

/** USART2 HW. */
//#define USART1_DR_Base  0x40013804		// 0x40013800 + 0x04 = 0x40013804
//#define SENDBUFF_SIZE   5000
#define RS485_USART                             USART2
#define RS485_USART_CLK                         RCC_APB1Periph_USART2
#define RS485_USART_BAUDRATE                    115200

#define RS485_USART_RX_GPIO_PORT                GPIOD
#define RS485_USART_RX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define RS485_USART_RX_PIN                      GPIO_Pin_6
#define RS485_USART_RX_AF                       GPIO_AF_USART2
#define RS485_USART_RX_SOURCE                   GPIO_PinSource6

#define RS485_USART_TX_GPIO_PORT                GPIOD
#define RS485_USART_TX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define RS485_USART_TX_PIN                      GPIO_Pin_5
#define RS485_USART_TX_AF                       GPIO_AF_USART2
#define RS485_USART_TX_SOURCE                   GPIO_PinSource5

//�շ��ܽ�
#define RS485_GPIO_PIN                          GPIO_Pin_11                 
#define RS485_GPIO_PORT                         GPIOD                     
#define RS485_GPIO_CLK                          RCC_AHB1Periph_GPIOD

#define	digitalHi(p,i)			                {p->BSRRL=i;}		//����Ϊ�ߵ�ƽ	-- reference bsp_led.h
#define digitalLo(p,i)			                {p->BSRRH=i;}		//����͵�ƽ
#define RS485_TX_ON_RX_OFF    	                digitalHi(RS485_GPIO_PORT,RS485_GPIO_PIN)
#define RS485_RX_ON_TX_OFF		                digitalLo(RS485_GPIO_PORT,RS485_GPIO_PIN)


/** RS485 */
#define NUM_TX_BUFF	128			//���巢�ͻ�������С
#define NUM_RX_BUFF	128			//������ܻ�������С

enum{
	MODBUS_03		= 0x03,		//03�� ���Ĵ���
};

enum{
	MODBUS_03_SUB_06= 0x06,		//03�� ���Ĵ���������� - �綯������״̬
	MODBUS_03_SUB_07= 0x07      //	                    - �綯����ģ����
};


/** bool */
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/** modbus �̻��Բ��豸��ַ  */
#define MODBUS_ADDRESS_SLAVE 0x01

/** Modbus Struct */
typedef struct {	
	uint16_t address;               //ͨѶ��ַ	0~255
	
	/**����**/
	uint8_t RxBuff[NUM_RX_BUFF];	//���ܻ�����
	uint16_t RxLen;					//���ܳ���
	uint16_t RxCount;				//���ܼ���
	
	/**����**/
	uint8_t TxBuff[NUM_TX_BUFF];	//���ͻ�����
	uint16_t TxLen;					//���ͳ���
	uint16_t TxCount;				//���ͼ���
	
	/**�Ƿ����±���δ�����־**/
	uint8_t bNew;					//TRUE: ���µ�һ��������Ҫ����
	/**�Ƿ����������ڷ��ͱ�־**/
	uint8_t bSend;					//TRUE: ��������
 	
// 	volatile uint32 RxTimer;		//����ʱ�������
//	volatile uint32 TxTimer;		//���ͳ�ʱ������
//	volatile uint32 OpTimer;		//������ʱ��ʱ��
//	volatile uint32 ExTimer;		//��չ��ʱ��������ĳЩ���⹦��
	
	//modbus ˽���������� - ����
	uint16_t modbusRxCRC;			//ModbusCRCУ��
	uint8_t  modbusCode;			//����������	
}TBuffModbus;

//�綯��״̬
enum{
	STATUS_STOP			=0x00,	//ֹ̬ͣ
	STATUS_STARTING,			//����̬
	STATUS_RUN					//����̬
};

//�綯��ת�� ����/����
enum{
	STATUS_POSITIVE,			//����
	STATUS_NEGATIVE 			//����
};

typedef struct{
	uint8_t motor_status_now:3;	// �����ǰ״̬ - ÿ�� modbus ���͵ĵ綯�� state ���Ķ���������ֵ,
	uint8_t motor_status_last:3;// �������һ��״̬ - mqtt�л�Ƚ� _now �� _last �Ƿ�ͬ, ���ǲ�ͬ������ mqtt ���״̬�����仯
	uint8_t motor_dir:2;		// �綯��ת�� ����/����
}S_Status;

// �綯��״̬ - Modbus ������ 0x03  ������ 0x06;
typedef union{
	uint8_t ui;
	S_Status s;
}TStatus;

//�綯������ֵ - Modbus ������ 0x03  ������ 0x07;
typedef struct{
	/***������� - ������***/
	uint16_t Meas_Ia;		
}TMeasure;

typedef struct {    
	TStatus Status;
    TMeasure Measure;
}TMotor;

extern TMotor stMotor; /** mqtt ����ʱ��Ҫ�� global ���� */

/*------------------------------------------------------------------*/
/** ����һЩ����ά�� OS ʱ��ı���, ��Щ����ʵ�ֺ���� ���ں��� HAL_UptimeMs() �� */
/** ͬʱ����Ҫ��� OS tick �л��жϺ���, ��Ϊ�ú�������ÿ 1ms �ж� 1 �� */
/**
 *	����ʱ�䣬��ȷ������
 *	��ı�ʾ��ʽΪ��λ������2007�꣬�ͱ�ʾΪy=7
 */
typedef struct {
	uint8_t y;	    //��
	uint8_t mon;	//��
	uint8_t d;	    //��
	uint8_t h;	    //ʱ
	uint8_t min;	//��
	uint8_t s;	    //��
	uint16_t ms;	//����
}TDateTime;

extern TDateTime stNowTime;

uint8_t CheckOSTime(TDateTime * pTime);
void InitOSTime(TDateTime * pTime); 
inline void OSTimeAddSecond(TDateTime * pTime);

/*------------------------------------------------------------------*/
/** Function API - Motor State */
/** ��ȡ/���õ��״̬ */
#define GetMotorStatusNow() (stMotor.Status.s.motor_status_now)
#define SetMotorStatusNow(status) (stMotor.Status.s.motor_status_now=(status))
#define GetMotorStatusLast() (stMotor.Status.s.motor_status_last)
#define SetMotorStatusLast(status) (stMotor.Status.s.motor_status_last=(status))

/** Function API - Modbus */
void RS485_USART2_Config(void);
void Modbus_OnSend(TBuffModbus *pBuff);
void Moubus_OnRecvive(TBuffModbus *pBuff);
void ModbusProc_InTask(TBuffModbus *pBuff);

#endif /* __USART2_H */

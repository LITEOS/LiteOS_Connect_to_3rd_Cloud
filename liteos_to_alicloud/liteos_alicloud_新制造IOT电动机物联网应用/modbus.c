/**
 * @file    modbus.c
 */

// Std Lib
#include <string.h>
// LiteOS
#include "los_hwi.h"
// Personal
#include "modbus.h"
#include "bsp_led.h"

/* extern define ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#ifndef Min
#define Min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((uint16_t)((a&0x00FF) | ((uint16_t)(b&0x00FF)) << 8))
#endif

#define Modbus_Crc16(pBuff, c) crc16_ex(&(c), 1, &(pBuff)->modbusRxCRC)

/* Private variables ---------------------------------------------------------*/
uint8_t uBuffRecv[NUM_RX_BUFF];
uint16_t uRecvLen=0;

/* public variables ---------------------------------------------------------*/
TBuffModbus stBufModbus;
TMotor stMotor;      /** �綯��״̬��Ϣ */
TDateTime stNowTime; /** ���ģ���һ�� os ���� + ʱ��, ��ȷ�� ms */

/* Private function prototypes -----------------------------------------------*/
static void Modbus_OnReceiveOneByte(TBuffModbus *pBuff, uint8_t uChar);

/* Function ------------------------------------------------------------------*/
void USART2_IRQHandler(void);

/** For Calc CRC */
const uint16_t crc16_xtab[256] =
{
   0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
   0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
   0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
   0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
   0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
   0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
   0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
   0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
   0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
   0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
   0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
   0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
   0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
   0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
   0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
   0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
   0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
   0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
   0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
   0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
   0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
   0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
   0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
   0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
   0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
   0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
   0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
   0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
   0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
   0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
   0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
   0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

/** Calc CRC  */
void crc16_ex(uint8_t * buf, uint32_t len, uint16_t * crc)
{
	while (len--)
    {
		*crc = ((*crc) >> 8) ^ crc16_xtab[  ((*crc) ^ (*buf&0x00ff))&0x00ff ];
        buf++;
	}
}

/**
  * usart2 for RS485
  */
void RS485_USART2_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;    
	NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd( RS485_USART_RX_GPIO_CLK|RS485_USART_TX_GPIO_CLK, ENABLE);

    /* Enable UART2 clock */
    RCC_APB1PeriphClockCmd(RS485_USART_CLK, ENABLE);

    /* Connect PXx to USARTx_Tx*/
    GPIO_PinAFConfig(RS485_USART_RX_GPIO_PORT, RS485_USART_RX_SOURCE, RS485_USART_RX_AF);

    /* Connect PXx to USARTx_Rx*/
    GPIO_PinAFConfig(RS485_USART_TX_GPIO_PORT, RS485_USART_TX_SOURCE, RS485_USART_TX_AF);

    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

    GPIO_InitStructure.GPIO_Pin = RS485_USART_TX_PIN  ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RS485_USART_TX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = RS485_USART_RX_PIN;
    GPIO_Init(RS485_USART_RX_GPIO_PORT, &GPIO_InitStructure);
    	
    /* USART2 mode config */
    USART_InitStructure.USART_BaudRate = RS485_USART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(RS485_USART, &USART_InitStructure); 
    USART_Cmd(RS485_USART, ENABLE);

    USART_ClearFlag(RS485_USART, USART_FLAG_TC);

    /** ע�� USART2 �ж� */
    LOS_HwiCreate(USART2_IRQn, 0, 0, USART2_IRQHandler, NULL);

    /** ���� USART2 �Ľ����ж� - USART2_IRQn */   
    NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;             //�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01;  //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;         //�����ȼ� 0
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
   	USART_ITConfig(RS485_USART, USART_IT_RXNE, ENABLE);         //Receive Data register not empty interrupt
}  

/**
 * 1. usart2 �жϽ���ʹ��, ֻ��Ҫ���� LPC ���͵�rs485 ���� 
 * 2. USART2 ÿ���ж� �� ����� 1 �� byte ������,  ����ÿ�ν����ж� uRecvLen++ ����
 *    ���ܷ�Ӧ�������յ���һ���ַ������ܳ�����.
 */
void USART2_IRQHandler(void)
{   
    /** Clear Int Flag */
    /** RXNE pending bit can be also cleared by a read to the USART_DR register (USART_ReceiveData()) */
    if(USART_GetITStatus(RS485_USART, USART_IT_RXNE) != RESET)
    {       
        //uBuffRecv[uCountRecv++] = USART_ReceiveData(RS485_USART);        
        uBuffRecv[uRecvLen++] = USART_ReceiveData(RS485_USART);
        //uCountRecv &= 0x7F; /** ���������±�Ĵ���, ����ÿһ�ֽ��ղ�������ַ���֮��, uBuffRecv[] ���±� index �����µ����� 0 �±�, ���Բ��ÿ���д���鳬��һȦ�����  */        
        uRecvLen &= 0x7F;
    }

    if(USART_GetFlagStatus(RS485_USART, USART_FLAG_ORE) == SET ) // if ���
    {
        USART_ClearFlag(RS485_USART, USART_FLAG_ORE);
        USART_ReceiveData(RS485_USART);
    }    
}

/**
 * Receive - Step1.
 * 1. @param uint8_t *pRecv , �����жϽ��ջ��������׵�ַ
 *    @parma uint16_t uLen , ǰһ�������Ĵ����жϽ��յ��� byte ���ݳ���, ��Ӧ uBuffRecv
 */
void Moubus_OnRecvive(TBuffModbus *pBuff)
{
    uint16_t i=0;
    
    /** 
     * 1. ÿ�ν����� uBuffRecv[] �����жϻ�������֮��, ��� uRecvLen / uCountRecv ���������,
     *    �� uCountRecv �����������ʾ uBuffRecv[] �±�ָ��� 0 ��ʼ,  
     *    ͬʱҪ�� uBuffRecv[] ȫ������, �Ա��´δ����жϽ�������    
     */
//    if(0 == uLen)
    if(0 == uRecvLen)
    {
        RS485_RX_ON_TX_OFF; 
        return; /** ��ʾ�����ж����µ��������, ��ȷ�� ���ڽ���ʹ�� */
    }

    /** ����ǰһ�δ��ڽӵ���ȫ������, ������ disable ���ڽ���, ��������Ϻ��� enable... */ 
    RS485_TX_ON_RX_OFF; 

    /** ��ʼ����ǰһ�εĴ����жϽ�������ȫ������,  */    
    /** 1. ���� uLen ���� == uRecvLen */
    for(i=0; i< uRecvLen; i++)
    {        
        Modbus_OnReceiveOneByte(pBuff, uBuffRecv[i]);
    }
    uRecvLen = 0;
    memset(uBuffRecv, 0, sizeof(uBuffRecv));    
    RS485_RX_ON_TX_OFF;

    /** ǰһ�δ�������ȫ��ÿ�� byte �������, �����Ƿ���һ����Ч���� packet ������һ�� */    
    /** 1. ����һ֡�����ı���, ����� CRC У�� ok */               
    
}

/** 
 * Receive - Step2.
 * 1.�����ֽڽ������յ��ı��� 
 */
static void Modbus_OnReceiveOneByte(TBuffModbus *pBuff, uint8_t uChar)
{
    uint16_t uCRC;   
    
    if(pBuff->bSend==FALSE && pBuff->bNew==FALSE && pBuff->RxCount<NUM_RX_BUFF)
    {
        /** ���ڷ���״̬, ���ҵ�ǰû���µı���packet */
        /** �峬ʱ���� */
        //pBuff->RxTimer = 0;
        pBuff->RxBuff[pBuff->RxCount++] = uChar;

        if(0x01 == pBuff->RxCount) /** 1. �ӻ� modbus ��ַ */
        {
            if(MODBUS_ADDRESS_SLAVE == uChar)
            {
                pBuff->modbusRxCRC = 0xFFFF;
                Modbus_Crc16(pBuff, uChar);
            }
            else
            {
                pBuff->RxCount = 0x00;
            }                
        }
        else if(0x02 == pBuff->RxCount) /** 2. ������ */
        {
            if(MODBUS_03 == uChar)
            {
                pBuff->modbusCode = uChar;                
                Modbus_Crc16(pBuff, uChar);
            }
            else
            {
                pBuff->RxCount = 0x00;
            }
        }
        else if(0x03 == pBuff->RxCount) /** ���ݳ��� bytes */
        {            
			/** ���ݹ����벻ͬ, ����ʽ������ͬ */
            switch(pBuff->modbusCode)
            {
                case MODBUS_03:
                {
                    pBuff->RxLen = 0x06;
                    break;
                }
                default:
                    break;
            }            
			Modbus_Crc16(pBuff, uChar);
        }
        else if(pBuff->RxCount>=4 && pBuff->RxCount<=pBuff->RxLen) /** ���� DU �� */
        {            
            Modbus_Crc16(pBuff, uChar);
        }
        else if(pBuff->RxCount == (pBuff->RxLen+2)) /** CRC У�� 2 bytes */
        {
            uCRC = MAKEWORD(pBuff->RxBuff[pBuff->RxCount-2], uChar);
            if(pBuff->modbusRxCRC == uCRC)
            {
                pBuff->bNew = TRUE; /** CRC У�����֮��, �������µ� ���� */
            }
            else
            {
                pBuff->RxCount = 0x00;
            }
        }        
    }//if(pBuff->bSend==FALSE ...)
}

/***/
void Modbus_OnSendByFIFO(uint8_t *pSendBuf, uint16_t uSendLen)
{
    uint16_t uSeq;
    
    RS485_TX_ON_RX_OFF; /** Enable Send Mode */

    for(uSeq = 0; uSeq < uSendLen; uSeq++)
    {        
        USART_SendData(RS485_USART, pSendBuf[uSeq]);
        while( USART_GetFlagStatus(RS485_USART, USART_FLAG_TXE) == RESET );
    }
}

/**
 * @2018-05-19
 * 1. stm32 send ����ʱ���Բ����Ƿ��� fifo �����, ���������, ȫ�������͵����ݶ��� 1 �������� Modbus_OnSendByFIFO() 
 *    ������ȫ���������, Ȼ��Ž�����������.
 */
void Modbus_OnSend(TBuffModbus *pBuff)
{
    uint16_t uSendPerLeft=0;

    if(TRUE == pBuff->bSend) /** ǰ��ĺ����Ѿ���֯��� Modbus ����,��׼���������ݵ�״̬ */
    {
        /** ���� send FIFO ����֮����� Modbus_OnSendByFIFO() */
        if(pBuff->TxCount < pBuff->TxLen)
        {
            uSendPerLeft = pBuff->TxLen - pBuff->TxCount;   /** ��ǰʣ�¶������ݴ����� */
            Modbus_OnSendByFIFO(&pBuff->TxBuff[pBuff->TxCount], uSendPerLeft);
            pBuff->TxCount += uSendPerLeft;
        }
        else if(pBuff->TxCount >= pBuff->TxLen)
        {
            // ��֧ : �����跢�͵����ݶ��Ѿ��������
            if(USART_GetFlagStatus(RS485_USART, USART_FLAG_TXE) == SET)
            {
                pBuff->bSend = FALSE;
                pBuff->TxCount = 0;
                RS485_RX_ON_TX_OFF; /** Enable RS485 receive  */                
            }
        }// if(TRUE == pBuff->bSend)
    }    
}

/** LiteOS �� ������ ��֯���ظ��� Modbus �������� */
void Mobus_ParserMsg(TBuffModbus *pBuff)
{   
    uint8_t uClassID = 0;
    uint16_t uDataLen = 0;
    
    if(NULL == pBuff)
    {
        return; 
    }

    /** ���� Ӧ�Ա��� */
    if(MODBUS_03 == pBuff->modbusCode)
    {
        // ���ݳ��� - �ӻ�Ӧ��� DU �򳤶�
        uDataLen = pBuff->RxBuff[2];            
        // �Ĵ������ - �ӻ�Ӧ��ı����� DU ��� 1 ���ֽ��� �������� �ļĴ������
        uClassID = pBuff->RxBuff[3]; 
        switch(uClassID){
            case MODBUS_03_SUB_06:{ // Motor State
                SetMotorStatusNow((pBuff->RxBuff[5] & 0x07)); /* �綯��״ֻ̬�뱨����� Byte �еĵ� 3 bit�й� */                
                break;
            }
            case MODBUS_03_SUB_07:{ // Motor Measure
                stMotor.Measure.Meas_Ia = pBuff->RxBuff[4];
                break;
            }
            default:{
                return;
            }
        }
    }
    else
    {
        return; //��֧�ֵĹ�����
    }
       
    /** ������� Modbus ����֮��,�ſ����� 0 �� FALSE, Ϊ�´ν��ձ�����׼��... */
    pBuff->RxCount = 0;
    pBuff->bNew = FALSE;
}

/***/
void ModbusProc_InTask(TBuffModbus *pBuff)
{    
    if(NULL == pBuff)
    {
        return; 
    }
    
    Moubus_OnRecvive(&stBufModbus);

    if(TRUE == pBuff->bNew) /** received one new packet */
    {     
        LED1_OFF;
        LED2_ON;        
        LED3_OFF;
        Mobus_ParserMsg(&stBufModbus);        
    }    
}

/**
 * 1. ���ʱ���Ƿ�Ϸ�
 */
// One-based array of days in year at month start
static const uint16_t _MonthDays[13] =	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

//�ж��Ƿ�������
/*
��������������������Ϊyear����

��1����year��400��������ʱΪ���꣬������ʾΪ��

year%400= =0

��2����year��4����������������100��������ʱΪ���꣬����Ϊ��

    year%4= =0 && year%100 != 0
*/
#define IsLeapYear(nYear) ((((nYear) & 3) == 0) && (((nYear) % 100) != 0 || ((nYear) % 400) == 0))

//����ĳ��ĳ���ж�����
#define GetDaysInMonth(nYear, nMon) (_MonthDays[nMon] - _MonthDays[(nMon)-1] + (((nMon) == 2 && IsLeapYear(nYear)) ? 1 : 0))

uint8_t CheckOSTime(TDateTime * pTime)
{
    // Validate year and month (ignore day of week and milliseconds)
    if (pTime->y > 99 || pTime->mon < 1 || pTime->mon > 12)
        return FALSE;
    
    // Finish validating the date
    if (pTime->d < 1 || pTime->d > GetDaysInMonth(pTime->y, pTime->mon) ||
        pTime->h > 23 || pTime->min > 59 ||
        pTime->s > 59 || pTime->ms>999)
    {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * 1.ϵͳʱ�ӳ�ʼ��
 */
void InitOSTime(TDateTime * pTime)
{    
    stNowTime.y = 18;
    stNowTime.mon = 1;
    stNowTime.d = 1;
    stNowTime.h = 0;
    stNowTime.min = 0;
    stNowTime.s = 0;
    stNowTime.ms = 0;
    CheckOSTime(&stNowTime);
}

/**
 * 1.����Ϊ��λʱ�����, ���� los_tick.c �е���
 */ 
void OSTimeAddSecond(TDateTime * pTime)
{
    uint16_t nDaysInMonth;
    pTime->s++;
    if(pTime->s >= 60)
    {
        pTime->s = 0;
        pTime->min++;
        if(pTime->min >= 60)
        {
            pTime->min = 0;
            pTime->h++;
            if(pTime->h >= 24)
            {
                pTime->h = 0;
                pTime->d++;
                
                //���㵱ǰ���ж�����
                nDaysInMonth =GetDaysInMonth(pTime->y, pTime->mon);
                
                if(pTime->d > nDaysInMonth)  //����һ����
                {
                    if(pTime->mon==12)
                    {
                        //��һ��
                        pTime->d = 1;
                        pTime->mon = 1;
                        pTime->y++;
                    }
                    else
                    {
                        pTime->d = 1;
                        pTime->mon++;
                    }
                }
            }
        }
    }       
}



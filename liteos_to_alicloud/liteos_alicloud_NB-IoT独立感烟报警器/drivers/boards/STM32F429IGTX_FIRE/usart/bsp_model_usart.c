#include <stdarg.h>
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
#include "bsp_model_usart.h"

#define RXBUFSIZE    2048

char uartrecBuf[RXBUFSIZE];
volatile UINT16 uartrecCnt = 0;
UINT32 g_uwModelMsg;
volatile UINT64  uwTickCount;


 /**
  * @brief  ����Ƕ�������жϿ�����NVIC
  * @param  ��
  * @retval ��
  */
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* �����ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = MODEL_USART_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
}

 /**
  * @brief  MODEL_USART GPIO ����,����ģʽ���á�115200 8-N-1 ���жϽ���ģʽ
  * @param  ��
  * @retval ��
  */
void MODEL_USART_Config(UINT32 baudrate)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  //USART_DeInit(MODEL_USART);
  USART_Cmd(MODEL_USART, DISABLE);
  USART_ITConfig(MODEL_USART,USART_IT_RXNE,DISABLE);
  RCC_AHB1PeriphClockCmd( MODEL_USART_RX_GPIO_CLK|MODEL_USART_TX_GPIO_CLK, ENABLE);

  /* ʹ�� UART ʱ�� */
  RCC_APB1PeriphClockCmd(MODEL_USART_CLK, ENABLE);
  
  /* ���� PXx �� USARTx_Tx*/
  GPIO_PinAFConfig(MODEL_USART_RX_GPIO_PORT,MODEL_USART_RX_SOURCE, MODEL_USART_RX_AF);

  /*  ���� PXx �� USARTx__Rx*/
  GPIO_PinAFConfig(MODEL_USART_TX_GPIO_PORT,MODEL_USART_TX_SOURCE,MODEL_USART_TX_AF);

  /* ����Tx����Ϊ���ù���  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = MODEL_USART_TX_PIN  ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(MODEL_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  /* ����Rx����Ϊ���ù��� */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = MODEL_USART_RX_PIN;
  GPIO_Init(MODEL_USART_RX_GPIO_PORT, &GPIO_InitStructure);
            
  /* ���ô���MODEL_USART ģʽ */
  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(MODEL_USART, &USART_InitStructure); 
  
  LOS_HwiCreate(MODEL_USART_IRQ, 0,0,MODEL_USART_IRQHandler,NULL);
  
  //uartrecBuf = (char *)LOS_MemAlloc(OS_SYS_MEM_ADDR, RXBUFSIZE);

  NVIC_Configuration();
  /*���ô��ڽ����ж�*/
  USART_ITConfig(MODEL_USART, USART_IT_RXNE, ENABLE);
    
  USART_Cmd(MODEL_USART, ENABLE);
}

void MODEL_RESET_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd( MODEL_RESET_GPIO_CLK, ENABLE);

  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

  GPIO_InitStructure.GPIO_Pin = MODEL_RESET_PIN  ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(MODEL_RESET_GPIO_PORT, &GPIO_InitStructure);
}


/*****************  ����һ���ַ� **********************/
static void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch )
{
    /* ����һ���ֽ����ݵ�USART1 */
    USART_SendData(pUSARTx,ch);
        
    /* �ȴ�������� */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);  
}
/*****************  ָ�����ȵķ����ַ��� **********************/
void Usart_SendStr_length( USART_TypeDef * pUSARTx, uint8_t *str,uint32_t strlen )
{
    unsigned int k=0;
    do 
    {
        Usart_SendByte( pUSARTx, *(str + k) );
        k++;
    } while(k < strlen);
}

/*****************  �����ַ��� **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, uint8_t *str)
{
    unsigned int k=0;
    do 
    {
        Usart_SendByte( pUSARTx, *(str + k) );
        k++;
    } while(*(str + k)!='\0');
}
  
static char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

void MODEL_USART_printf(char *Data,...)
{
  const char *s;
  int d;   
  char buf[16];

  va_list ap;
  va_start(ap, Data);

    while ( *Data != 0)     // �ж��Ƿ񵽴��ַ���������
    {                                         
        if ( *Data == 0x5c )  //'\'
        {                                     
            switch ( *++Data )
            {
                case 'r':                                     //�س���
                    USART_SendData(MODEL_USART, 0x0d);
                    Data ++;
                    break;

                case 'n':                                     //���з�
                    USART_SendData(MODEL_USART, 0x0a);   
                    Data ++;
                    break;
                
                default:
                    Data ++;
                    break;
            }            
        }
        else if ( *Data == '%')
        {                                     //
            switch ( *++Data )
            {               
                case 's':                                         //�ַ���
                    s = va_arg(ap, const char *);
          for ( ; *s; s++) 
                    {
                        USART_SendData(MODEL_USART,*s);
                        while( USART_GetFlagStatus(MODEL_USART, USART_FLAG_TXE) == RESET );
          }
                    Data++;
          break;

        case 'd':                                       //ʮ����
          d = va_arg(ap, int);
          itoa(d, buf, 10);
          for (s = buf; *s; s++) 
                    {
                        USART_SendData(MODEL_USART,*s);
                        while( USART_GetFlagStatus(MODEL_USART, USART_FLAG_TXE) == RESET );
          }
                    Data++;
          break;
                 default:
                        Data++;
                    break;
            }        
        } /* end of else if */
        else USART_SendData(MODEL_USART, *Data++);
        while( USART_GetFlagStatus(MODEL_USART, USART_FLAG_TXE) == RESET );
    }
}

void MODEL_USART_IRQHandler(void)
{
    UINT32 uwRet = 0;
    UINT8 msg[1];
    if (USART_GetITStatus(MODEL_USART, USART_IT_RXNE) != RESET)
    {  
        uartrecBuf[uartrecCnt++] = USART_ReceiveData(MODEL_USART);
        if (uartrecCnt >= RXBUFSIZE) {
            uartrecCnt = 0;
        }
        if (g_uwModelMsg) {
            uwRet = LOS_QueueWrite(g_uwModelMsg, msg, 1, 0);
            if(uwRet != LOS_OK)
            {
                //printf("create model msg failure!,error:%x\n", uwRet);
            }
        }
        //uwTickCount = LOS_TickCountGet();
    }
}

int SysTimeOut(UINT32 start, UINT32 end, UINT32 cur)
{
    return (((cur-start) >= (end-start)) ? 1 : 0);  
}
void MODEL_MSG_QueueCreate(void)
{
    UINT32 uwRet = 0;
    uwRet = LOS_QueueCreate("model msg", 5, &g_uwModelMsg, 0, 24);
    if(uwRet != LOS_OK)
    {
        printf("create model msg failure!,error:%x\n", uwRet);
    }

}

UINT32 MODEL_MSG_QueueRead(char *rx_buff, UINT32 TimeOut)
{
    int len = 0;
    UINT32 uwReadbuf;
    UINT32 uwRet = LOS_OK;
    MODEL_MSG_QueueCreate();
    //uwTickCount = LOS_TickCountGet();
    memset(uartrecBuf, 0, RXBUFSIZE);
    uartrecCnt = 0;

    while (1)
    {   
        uwRet = LOS_QueueRead(g_uwModelMsg, &uwReadbuf, 24, TimeOut);
        if(uwRet == LOS_ERRNO_QUEUE_TIMEOUT) { 
        //if (SysTimeOut(uwTickCount,uwTickCount+TimeOut,LOS_TickCountGet())) {
            if (uartrecCnt) {
                memcpy(rx_buff, uartrecBuf, uartrecCnt);
                rx_buff[uartrecCnt] = 0;
                len = uartrecCnt;
                uartrecCnt = 0;
            }
            break;
        }
    }
    LOS_QueueDelete(g_uwModelMsg);
    return len;
}

void MODEL_USART_TEST(void)
{
    char *redata;
    
    redata = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 255);  

    MODEL_USART_Config(MODEL_USART_BAUDRATE);                 //��ʼ����?    
    while (1) {
        if (MODEL_MSG_QueueRead(redata, LOS_WAIT_FOREVER)) {
            MODEL_USART_printf(redata);
        }
    }
     
}

/*********************************************END OF FILE**********************/

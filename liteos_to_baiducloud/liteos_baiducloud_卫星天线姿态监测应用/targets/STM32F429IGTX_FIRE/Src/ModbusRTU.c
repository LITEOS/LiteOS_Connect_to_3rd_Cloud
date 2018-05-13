/*
 * Copyright (C) 2018-2019 �����@Ҷ���Ƽ�   ΢�ţ�yefanqiu
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ModbusRTU.h"
#include "los_task.h"

//****************************************************************
//RS485�ӿ���Ҫ���⴦��һ��  �����RS485��ֵλ1�������������0
//****************************************************************
#define MODBUSRTU_RS485  1  

#define MODBUSRTU_UART_BUFF_SIZE             512
uint8_t MODBUSRTU_UART_BUFF[MODBUSRTU_UART_BUFF_SIZE];
volatile uint16_t MODBUSRTU_UART_BytesToRead = 0;
void MODBUSRTU_USART_IRQHandler(void);
//=========================================================================================
// ����
//=========================================================================================
//���ڳ�ʼ��
void MODBUSRTU_UART_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_AHB1PeriphClockCmd( MODBUSRTU_USART_RX_GPIO_CLK|MODBUSRTU_USART_TX_GPIO_CLK, ENABLE);

  /* Enable UART clock */
  MODBUSRTU_USART_CLKCMD(MODBUSRTU_USART_CLK, ENABLE);
  
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(MODBUSRTU_USART_RX_GPIO_PORT,MODBUSRTU_USART_RX_SOURCE, MODBUSRTU_USART_RX_AF);

  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(MODBUSRTU_USART_TX_GPIO_PORT,MODBUSRTU_USART_TX_SOURCE,MODBUSRTU_USART_TX_AF);

  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = MODBUSRTU_USART_TX_PIN  ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(MODBUSRTU_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Pin = MODBUSRTU_USART_RX_PIN;
  GPIO_Init(MODBUSRTU_USART_RX_GPIO_PORT, &GPIO_InitStructure);
			
  /* USART1 mode config */
  USART_InitStructure.USART_BaudRate = MODBUSRTU_USART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(MODBUSRTU_USART, &USART_InitStructure); 
  
  USART_Cmd(MODBUSRTU_USART, ENABLE);	
	USART_ClearFlag(MODBUSRTU_USART, USART_FLAG_TC);  
  
  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = MODBUSRTU_USART_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
	//ע���ж�
	LOS_HwiCreate(MODBUSRTU_USART_IRQ, 0,0,MODBUSRTU_USART_IRQHandler,0);
	
  /* ʹ�ܴ���2�����ж� */
	USART_ITConfig(MODBUSRTU_USART, USART_IT_RXNE, ENABLE);
}

//�жϻ��洮������
void MODBUSRTU_USART_IRQHandler(void)
{
  if(MODBUSRTU_UART_BytesToRead<MODBUSRTU_UART_BUFF_SIZE)
  {
    if(USART_GetITStatus(MODBUSRTU_USART, USART_IT_RXNE) != RESET)
    {			 
        MODBUSRTU_UART_BUFF[MODBUSRTU_UART_BytesToRead++] = (uint8_t)USART_ReceiveData(MODBUSRTU_USART);			
			  //printf("%d=%x\r\n",MODBUSRTU_UART_BytesToRead,MODBUSRTU_UART_BUFF[MODBUSRTU_UART_BytesToRead-1]);			 
    }
  }
}

//�������
void MODBUSRTU_UART_DiscardInBuffer(void)
{
  MODBUSRTU_UART_BytesToRead = 0;
	for(int i=0;i<MODBUSRTU_UART_BUFF_SIZE;i++)
	{
    MODBUSRTU_UART_BUFF[i]=0;
  }
}

void MODBUSRTU_UART_Write(uint8_t *DataArray,uint16_t size)
{
#if MODBUSRTU_RS485
	//RS485����ģʽ
	RS485_GPIO_ON;	
#endif
	
	for(int i=0;i<size;i++)
	{
		 //printf("[s]%d=%x\r\n",i,DataArray[i]);
		 USART_SendData(MODBUSRTU_USART,DataArray[i]);
	 	 while( USART_GetFlagStatus(MODBUSRTU_USART, USART_FLAG_TXE) == RESET );
	}	
	
#if MODBUSRTU_RS485
	//�෢һ������
	USART_SendData(MODBUSRTU_USART,0);
	while( USART_GetFlagStatus(MODBUSRTU_USART, USART_FLAG_TXE) == RESET);
	//RS485����ģʽ
	RS485_GPIO_OFF;	
#endif
}


//=========================================================================================
// Modbus
//=========================================================================================
//CRC16У��
uint16_t GetCheckCode(uint8_t * buf,int nEnd)
{
	uint16_t crc=(uint16_t)0xffff;
	int i,j;
	for(i = 0; i < nEnd; i++)
	{
		crc^=(uint16_t)buf[i];
		for(j = 0; j < 8; j++)
		{
			if(crc&1)
			{
				crc>>=1;
				crc^=0xA001;
			}
			else
				crc>>=1;
		}
	}
	return crc;
}

//��������
int SendCommand(unsigned int intSendNum,unsigned char *byrSendData,unsigned int intInceptNum,uint8_t *bytInceptData)
{
	   //��ս��պͷ��ͻ�����
	   MODBUSRTU_UART_DiscardInBuffer();	
	   //��������
     MODBUSRTU_UART_Write(byrSendData,intSendNum);	
	   /*
	   for (int t = 0; t < intInceptNum * 2 + 30; t++)
     {
         if (MODBUSRTU_UART_BytesToRead >= intInceptNum) break;
         LOS_TaskDelay(10);		
     }*/	   
		 
     //-----------------------------
		 LOS_TaskDelay(200);		
	   
		 if (MODBUSRTU_UART_BytesToRead >= intInceptNum)
		 { 
 #if MODBUSRTU_RS485
			  memcpy(bytInceptData,&MODBUSRTU_UART_BUFF[1], intInceptNum);
 #else
		  	memcpy(bytInceptData,MODBUSRTU_UART_BUFF, intInceptNum);
 #endif		

        //for(int i=0;i<intInceptNum;i++) printf("%X ",bytInceptData[i]);
			  //printf("\r\n");
			 
			  MODBUSRTU_UART_DiscardInBuffer();	
				return 0;               //���ݽ��ճɹ�
		 }
		 else
		 {
		 		return -1;              //���ݽ���ʧ��
		 }
}

//�����շ�
int RtuData(uint8_t Addr, uint8_t Mode, uint16_t DataStart, uint8_t *DataArray,uint16_t DataNum)
{
    uint8_t bytSendArray[255];          
    uint8_t bytReceiveArray[255];          
	  uint16_t intCRC16;
    int i;
    int intOffSet;
    int intSendNum;
    int intGetDataLen;                  

    if (DataNum>64 || DataNum<1) return 3;
  
    bytSendArray[0] = Addr;                       //�豸��ַ
    bytSendArray[1] = Mode;                       //����ģʽ
    bytSendArray[2] = DataStart / 256;            //��ַ��λ
    bytSendArray[3] = DataStart & 0xFF;           //��ַ��λ

    bytSendArray[4] = DataNum / 256;              //���ݸ�����λ
    bytSendArray[5] = DataNum & 0xFF;             //���ݸ�����λ

    if (Mode==MODBUSRTU_WriteData)
    {
	     bytSendArray[6] = DataNum * 2;              //���ݵ��ֽڸ���
	     for(i = 1;i<DataNum * 2+1;i++)
	     bytSendArray[6+i] = DataArray[i-1];
	     intOffSet = 7 + DataNum * 2;
    }
    else
    {
	      intOffSet = 6;
    }
    intCRC16=GetCheckCode(bytSendArray,intOffSet);
    bytSendArray[intOffSet] = intCRC16 & 0xFF;                    //CRCУ���λ
    bytSendArray[intOffSet + 1] = (intCRC16>>8) &0xff;            //CRCУ���λ
    
    intSendNum=intOffSet+2;
    if (Mode==MODBUSRTU_WriteData)  intGetDataLen = 8;
    else   intGetDataLen = 5 + DataNum * 2;
    
    //���ͺͽ�������
    if (SendCommand(intSendNum,bytSendArray,intGetDataLen,bytReceiveArray)!=0)
    {
	      return 1;
    }

    //��Ϣ����
    intCRC16=GetCheckCode(bytReceiveArray,intGetDataLen-2);

    //CRC16У��
    if (bytReceiveArray[intGetDataLen - 2]==(intCRC16 & 0xFF) && bytReceiveArray[intGetDataLen - 1]==((intCRC16>>8) &0xff))
    {
				//֡�����Ƿ���ȷ
				if (bytReceiveArray[0] == bytSendArray[0] && bytReceiveArray[1] == bytSendArray[1])
				{
					 if( Mode==MODBUSRTU_WriteData)
					 {
							 //
					 }
					 else
					 {
							 for(i=0;i<bytReceiveArray[2];i++)
							 DataArray[i] = bytReceiveArray[3 + i];
					 }
					 return 0;
				}
    }
   return 2;
}

//=========================================================================================================//

#define _TCP_CLIENT_DEMO_C_

#include "tcp_client_demo.h"
#include "./Bsp/systick/bsp_SysTick.h"
#include "./Bsp/usart/bsp_debug_usart.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#include "lb_timer.h"
#include "./Bsp/led/bsp_led.h"   
#include "Wdt.h"



/*
*���ܣ�STM32F407VET6��̫�����İ���Դ��룬�Զ���ȡIP������������������ͨ��ITM�Ĵ��ڴ�ӡ��ǰ������Ϣ
*			 1.����keil5��ΰ�װ����Ƶ�̡̳���01��03 Keil5��װ��ע�����    �뵽������ַ��ȡ���ص�ַ��http://www.zkaifa.com/bbs/thread-12-1-1.html
*			 2.����ITM���ʹ��  ����Ƶ�̡̳���01��04 ��COS�������ؼ������������뵽������ַ��ȡ���ص�ַ��http://www.zkaifa.com/bbs/thread-12-1-1.html
*��������������(www.zkaifa.com)
*��̳��www.zkaifa.com/bbs
*�Ա���http://shop68304486.taobao.com/
*���ߣ�liubinkaixin
*ʱ�䣺2015-05-30
*��������ǰ�汾���ṩ���İ���ԣ����ṩ�о�ѧϰʹ�ã�����Ϊ��ҵ��;�����κδ����ṩ�����е��κ�����
*��л������ο�������ԭ��@ALIENTEK���ṩ��Դ������б�д���ڴ˷ǳ���л���ǵĹ���ʦ��
*/


typedef struct{
	INT8U ucMsgBuf[MAX_BUFFER_LEN];
	INT32U dwLen;
}MSG_BUF_STRUCT;

//TCP Client�������ݻ�����
MSG_BUF_STRUCT tcp_client_recvbuf;
//TCP������������������
MSG_BUF_STRUCT tcp_client_sendbuf;

LB_TMRCB *TimerTcpPeriod;
LB_TMRCB *TimerTcpTimout;
LB_TMRCB *TimerSendTimout;

BOOL bSendBusy = FALSE;
BOOL bReConnFlag = FALSE;

const char get_requst[]="GET /devices/284289/datapoints HTTP/1.1\r\napi-key: UxvFGrtEFZMnfhwF6pa6zV5h9ZwA\r\nHost: api.heclouds.com\r\n\r\n";

//u8 uRemoteIp[4] = {121,40,173,197};
//u8 uRemoteIp[4] = {192,168,5,99};
//u8 uRemoteIp[4] = {192,168,11,196};
//const u8 *tcp_client_sendbuf="Explorer STM32F407 TCP Client send data\r\n";

//TCP Client ����ȫ��״̬��Ǳ���
//bit7:0,û������Ҫ����;1,������Ҫ����
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û�������Ϸ�����;1,�����Ϸ�������.
//bit4~0:����
u8 tcp_client_flag;
u8 connflag=0;		//���ӱ��

//����Զ��IP��ַ
void tcp_client_set_remoteip(void)
{
    u8 *tbuf;
		//u8 uRemoteIp[4] = {192,168,5,99};
    uart_dbg_printf((u8 *)"��������\n");
    tbuf=mymalloc(SRAMIN,100);	//�����ڴ�
    if(tbuf==NULL)return;
    //ǰ����IP���ֺ�DHCP�õ���IPһ��
    lwipdev.remoteip[0]=uRemoteIp[0];
    lwipdev.remoteip[1]=uRemoteIp[1];
    lwipdev.remoteip[2]=uRemoteIp[2];
    lwipdev.remoteip[3]=uRemoteIp[3];
    sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//Զ��IP
    uart_dbg_printf(tbuf);

    myfree(SRAMIN,tbuf);
}

//�жϷ����Ƿ����
BOOL IsSendBusy(void)
{
	return bSendBusy;
}

void SendTimout(void)
{
	bSendBusy = FALSE;
	DisLB_TMR(TimerSendTimout);//�رշ��ͳ�ʱ
	printf("ִ��TcpDataSend��ʱ��\n");
}

//��������
BOOL TcpDataSend(BYTE *buf,WORD len)
{
	int i;
	if(bSendBusy == FALSE)
	{
		printf("ִ��TcpDataSend\n");
		for(i = 0;i < len;i ++)
		{
			tcp_client_sendbuf.ucMsgBuf[i] = buf[i];
		}
		tcp_client_sendbuf.dwLen = len;
		bSendBusy = TRUE;
		tcp_client_flag|=1<<7;//���Ҫ��������
		EnLB_TMR(TimerSendTimout);
		return TRUE;
	}

	return FALSE;
}

void key_check(void)
{
	if(0 ==GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8))
	{
		printf("key fst down!\n");
		//ע������Ҫ��1����Ϊ�ַ������һ��'\0'Ҳ���ȥ�ˣ��ᵼ��OneNet��������ʶ��
		TcpDataSend((BYTE *)get_requst,sizeof(get_requst)-1);
	}
}

void TimerTcpPeroidCallBack(void)
{
	static int t = 0;
	t++;
	if(t==200)	//200ms
	{
		t = 0;
		bReConnFlag = TRUE;
	}
	if(0 == (t%100))//100ms
	{
		key_check();
	}
	//�����lwip�ں���صģ����붨ʱ����
	lwip_periodic_handle();
}

void TcpTimout(void)
{
	connflag=0;
	tcp_client_flag=0;
}

//TCP Client ����
void tcp_client_test(void)
{
	struct tcp_pcb *tcppcb;  	//����һ��TCP���������ƿ�
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
	u8 *tbuf;
	
		
	TimerTcpPeriod = LB_TMRCreate(1,TRUE,(TIMER_CB *)TimerTcpPeroidCallBack,(TIMER_CB *)0,(void *)0);
	

	tcp_client_set_remoteip();//��ѡ��IP

	uart_dbg_printf((u8 *)"www.zkaifa.com\n");
	tbuf=mymalloc(SRAMIN,200);	//�����ڴ�
	if(tbuf==NULL)return ;		//�ڴ�����ʧ����,ֱ���˳�
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//������IP
	uart_dbg_printf(tbuf);
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//Զ��IP
	uart_dbg_printf(tbuf);
	sprintf((char*)tbuf,"Remote Port:%d\n",TCP_CLIENT_PORT);//�ͻ��˶˿ں�
	uart_dbg_printf(tbuf);

	uart_dbg_printf((u8 *)"STATUS:Disconnected\n");
	tcppcb=tcp_new();	//����һ���µ�pcb
	if(tcppcb)			//�����ɹ�
	{
			IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
			tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
	}

	TimerTcpTimout = LB_TMRCreate(2000,TRUE,(TIMER_CB *)TcpTimout,(TIMER_CB *)0,(void *)0);
	TimerSendTimout = LB_TMRCreate(1500,FALSE,(TIMER_CB *)SendTimout,(TIMER_CB *)0,(void *)0);

	while(1)
	{
		
		if(tcp_client_flag&1<<6)//�Ƿ��յ�����?
		{
			printf("���յ�%d�ֽ�����:\n%s\n",tcp_client_recvbuf.dwLen,tcp_client_recvbuf.ucMsgBuf);
			//��������
			
			tcp_client_flag&=~(1<<6);//��������Ѿ���������.
		}
		if(tcp_client_flag&1<<5)//�Ƿ�������?
		{
			if(connflag==0)
			{
				uart_dbg_printf((u8 *)"STATUS:Connected   \n");//��ʾ��Ϣ
				connflag=1;//���������
			}
		}
		else if(connflag)
		{
			uart_dbg_printf((u8 *)"STATUS:Disconnected\n");
			connflag=0;	//������ӶϿ���
		}
				
		//400ms��������Ƿ�Ҫ����
		if(bReConnFlag == TRUE)
		{
			bReConnFlag = FALSE; 
			if(connflag==0&&(tcp_client_flag&1<<5)==0)//δ������,��������
			{
				ReCntLB_TMR(TimerTcpTimout);
				tcp_client_connection_close(tcppcb,0);//�ر�����
				tcppcb=tcp_new();	//����һ���µ�pcb
				if(tcppcb)				//�����ɹ�
				{
						tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);//���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
				}
			}
		}
		//��ʱ�������õģ������������
		LB_ExcTMR();
	}
}
//lwIP TCP���ӽ�������ûص�����
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    struct tcp_client_struct *es=NULL;
    if(err==ERR_OK)
    {
        es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //�����ڴ�
        if(es) //�ڴ�����ɹ�
        {
            es->state=ES_TCPCLIENT_CONNECTED;//״̬Ϊ���ӳɹ�
            es->pcb=tpcb;
            es->p=NULL;
            tcp_arg(tpcb,es);        			//ʹ��es����tpcb��callback_arg
            tcp_recv(tpcb,tcp_client_recv);  	//��ʼ��LwIP��tcp_recv�ص�����
            tcp_err(tpcb,tcp_client_error); 	//��ʼ��tcp_err()�ص�����
            tcp_sent(tpcb,tcp_client_sent);		//��ʼ��LwIP��tcp_sent�ص�����
            tcp_poll(tpcb,tcp_client_poll,1); 	//��ʼ��LwIP��tcp_poll�ص�����
            tcp_client_flag|=1<<5; 				//������ӵ���������
            err=ERR_OK;
        }
        else
        {
            tcp_client_connection_close(tpcb,es);//�ر�����
            err=ERR_MEM;	//�����ڴ�������
        }
    }
    else
    {
        tcp_client_connection_close(tpcb,0);//�ر�����
    }
    return err;
}
//lwIP tcp_recv()�����Ļص�����
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{
    u32 data_len=0;
    struct pbuf *q;
    struct tcp_client_struct *es;
    err_t ret_err;
    LWIP_ASSERT("arg != NULL",arg != NULL);
    es=(struct tcp_client_struct *)arg;
    if(p==NULL)//����ӷ��������յ��յ�����֡�͹ر�����
    {
        es->state=ES_TCPCLIENT_CLOSING;//��Ҫ�ر�TCP ������
        es->p=p;
        ret_err=ERR_OK;
    }
    else if(err!= ERR_OK) //�����յ�һ���ǿյ�����֡,����err!=ERR_OK
    {
        if(p)pbuf_free(p);//�ͷŽ���pbuf
        ret_err=err;
    }
    else if(es->state==ES_TCPCLIENT_CONNECTED)	//����������״̬ʱ
    {
        if(p!=NULL)//����������״̬���ҽ��յ������ݲ�Ϊ��ʱ
        {
            memset(tcp_client_recvbuf.ucMsgBuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
            for(q=p; q!=NULL; q=q->next) //����������pbuf����
            {
                //�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
                //�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
                if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf.ucMsgBuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
                else memcpy(tcp_client_recvbuf.ucMsgBuf+data_len,q->payload,q->len);
                data_len += q->len;
                if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����
            }
						tcp_client_recvbuf.dwLen = data_len;
            tcp_client_flag|=1<<6;		//��ǽ��յ�������
            tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
            pbuf_free(p);  	//�ͷ��ڴ�
            ret_err=ERR_OK;
        }
    }
    else   //���յ����ݵ��������Ѿ��ر�,
    {
        tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
        es->p=NULL;
        pbuf_free(p); //�ͷ��ڴ�
        ret_err=ERR_OK;
    }
    return ret_err;
}
//lwIP tcp_err�����Ļص�����
void tcp_client_error(void *arg,err_t err)
{
    //�������ǲ����κδ���
}
//lwIP tcp_poll�Ļص�����
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct tcp_client_struct *es;
    es=(struct tcp_client_struct*)arg;
    if(es!=NULL)  //���Ӵ��ڿ��п��Է�������
    {
			ReCntLB_TMR(TimerTcpTimout);
			if(tcp_client_flag&(1<<7))	//�ж��Ƿ�������Ҫ����
			{
				es->p=pbuf_alloc(PBUF_TRANSPORT, tcp_client_sendbuf.dwLen,PBUF_POOL);	//�����ڴ�
				pbuf_take(es->p,(char*)tcp_client_sendbuf.ucMsgBuf,tcp_client_sendbuf.dwLen);	//��tcp_client_sentbuf[]�е����ݿ�����es->p_tx��
				tcp_client_senddata(tpcb,es);//��tcp_client_sentbuf[]���渴�Ƹ�pbuf�����ݷ��ͳ�ȥ
				tcp_client_flag&=~(1<<7);	//������ݷ��ͱ�־
				bSendBusy = FALSE;
				
				DisLB_TMR(TimerSendTimout);//�رշ��ͳ�ʱ
				if(es->p)pbuf_free(es->p);	//�ͷ��ڴ�
			}
			else if(es->state==ES_TCPCLIENT_CLOSING)
			{
				tcp_client_connection_close(tpcb,es);//�ر�TCP����
			}
			ret_err=ERR_OK;
    }
    else
    {
			tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
			ret_err=ERR_ABRT;
    }
    return ret_err;
}
//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct tcp_client_struct *es;
    LWIP_UNUSED_ARG(len);
    es=(struct tcp_client_struct*)arg;
    if(es->p)tcp_client_senddata(tpcb,es);//��������
    return ERR_OK;
}
//�˺���������������
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
    struct pbuf *ptr;
    err_t wr_err=ERR_OK;
    while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))) //��Ҫ���͵����ݼ��뵽���ͻ��������
    {
        ptr=es->p;
        wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
        if(wr_err==ERR_OK)
        {
            es->p=ptr->next;			//ָ����һ��pbuf
            if(es->p)pbuf_ref(es->p);	//pbuf��ref��һ
            pbuf_free(ptr);				//�ͷ�ptr
        }
        else if(wr_err==ERR_MEM)es->p=ptr;
        tcp_output(tpcb);		//�����ͻ�������е������������ͳ�ȥ
    }
}
//�ر��������������
void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
    //�Ƴ��ص�
    tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
    tcp_arg(tpcb,NULL);
    tcp_recv(tpcb,NULL);
    tcp_sent(tpcb,NULL);
    tcp_err(tpcb,NULL);
    tcp_poll(tpcb,NULL,0);
    if(es)mem_free(es);
    tcp_client_flag&=~(1<<5);//������ӶϿ���
}























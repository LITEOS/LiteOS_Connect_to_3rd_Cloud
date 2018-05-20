/*
***********************************************************************************
*			
*���ܣ�
*
*
*
*˵����1.
*      2.
*      3.
*
*
*By			:Liubing�����ľͺã�
*Contact	:371007204@qq.com
*History	:2015/7/1 16:13:05
***********************************************************************************
*/

#ifndef _TCP_CLIENT_DEMO_H_
#define _TCP_CLIENT_DEMO_H_

#include "sys1.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip_comm.h"
#include "lb_type.h"

#ifdef _TCP_CLIENT_DEMO_C_
#define TCP_CLIENT_DEMO_EXT
#else 
#define TCP_CLIENT_DEMO_EXT extern
#endif 

#ifdef _TCP_CLIENT_DEMO_C_
#endif 

#define MAX_BUFFER_LEN 1024
#define TCP_CLIENT_RX_BUFSIZE	MAX_BUFFER_LEN		//����tcp client���������ݳ���
#define	TCP_CLIENT_PORT			80	//����tcp clientҪ���ӵ�Զ�˶˿�

//tcp����������״̬
enum tcp_client_states
{
	ES_TCPCLIENT_NONE = 0,		//û������
	ES_TCPCLIENT_CONNECTED,		//���ӵ��������� 
	ES_TCPCLIENT_CLOSING,		//�ر�����
}; 
//LWIP�ص�����ʹ�õĽṹ��
struct tcp_client_struct
{
	u8 state;               //��ǰ����״
	struct tcp_pcb *pcb;    //ָ��ǰ��pcb
	struct pbuf *p;         //ָ�����/�����pbuf
};  

TCP_CLIENT_DEMO_EXT u8 uRemoteIp[4];
TCP_CLIENT_DEMO_EXT BOOL bDnsOk;

TCP_CLIENT_DEMO_EXT void tcp_client_set_remoteip(void);
TCP_CLIENT_DEMO_EXT void tcp_client_test(void);//TCP Client���Ժ���
TCP_CLIENT_DEMO_EXT err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
TCP_CLIENT_DEMO_EXT err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err);
TCP_CLIENT_DEMO_EXT void tcp_client_error(void *arg,err_t err);
TCP_CLIENT_DEMO_EXT err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb);
TCP_CLIENT_DEMO_EXT err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
TCP_CLIENT_DEMO_EXT void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es);
TCP_CLIENT_DEMO_EXT void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es );
TCP_CLIENT_DEMO_EXT BOOL TcpDataSend(BYTE *buf,WORD len);
TCP_CLIENT_DEMO_EXT BOOL IsSendBusy(void);
TCP_CLIENT_DEMO_EXT void TcpTimout(void);//��ʹtcp����
#endif
























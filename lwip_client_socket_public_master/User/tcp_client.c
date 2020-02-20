/*
*********************************************************************************************************
*
*	ģ������ : tcp_client
*	�ļ����� : tcp_client.c
*	��    �� : V1.0
*	˵    �� : TCP �ͻ��� �������Ӻ�̨������
*
*	�޸ļ�¼ :
*						�汾��    ����        ����       ˵��
*						V1.0  2019��04��04��  suozhang   �״η���
*
*********************************************************************************************************
*/


#include "tcp_client.h"

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "app.h"
/**
 * Log default configuration for EasyLogger.
 * NOTE: Must defined before including the <elog.h>
 */
#if !defined(LOG_TAG)
#define LOG_TAG                    "tcp_client_tag:"
#endif
#undef LOG_LVL
#if defined(XX_LOG_LVL)
    #define LOG_LVL                    XX_LOG_LVL
#endif

//#include "elog.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"

#include "lwip/tcp.h"
#include "lwip/sockets.h"
#include "lwip/ip.h"
#include "app.h"
#include "lwip/dhcp.h"
uint8_t RxdTelBuf[20];
uint8_t connect_md = 0;//����ѡ��ʱ���õ��м�������55ָ�����ٸ�ֵ
uint8_t command_adjust_4_20ma(void);
void command_set_connect_method(void);
void command_reply_connect_method(void);
uint8_t command_esp32_statue(void);
extern uint8_t  RxdBuf[RxdBufLine];
extern TaskHandle_t xHandleTaskLwip;
extern TaskHandle_t xHandleTaskSendbuf;
extern TaskHandle_t xHandleEsp32ProcessFunction;

uint8_t iap_data_in_sdram[800000] __attribute__((at(0xC1000000))); //������������ݣ�����

extern TaskHandle_t xHandleTaskLED;

#define TCP_SERVER_IP   "192.168.0.88"
#define TCP_SERVER_PORT 8712
//#define TCP_SERVER_IP "218.91.149.114"
//#define TCP_SERVER_PORT 9999


uint8_t command_adjust_temp(void);
void reply_channelkind(void);
void set_channelkind(void);
#define TCP_SERVER_IP_1   "192.168.0.98"
#define TCP_SERVER_PORT_1 8712

#define sw_version "china-yec1224"
/* ������ͨ��ʹ��Ȩ�ź��� */

uint32_t NetReceiveBufHeadIndex=0;
uint32_t NetReceiveBufTailIndex=0;

uint8_t    RXDCmdBuf[10];
uint8_t    CmdBuf[TX_BUFFER_SIZE];  //

volatile uint16_t RxdBytes=0;
uint16_t CmdBufLength=0;

struct netconn *tcp_client_server_conn=NULL;

void RxdByte(uint8_t c);
extern CONFIG config;
struct JoinNet joinnetstatue;

extern struct netif gnetif;
uint16_t Join_IP_Count = 0;
#define UDP_BUF_SIZE 50
uint8_t udp_server_recvbuf[UDP_BUF_SIZE];	//UDP�������ݻ�����
uint8_t udp_server_recvbuf_index[UDP_BUF_SIZE][UDP_BUF_SIZE];
uint8_t udp_server_recvbuf_index_tail = 0;
uint8_t udp_server_recvbuf_index_head = 0;
ip_addr_t dest_ipaddr;
uint16_t dest_port;
uint16_t dest_port_buf[50];
extern void Analytic_Function(uint8_t *rec_buf);
void vTaskLwip(void *pvParameters)
{
		uint32_t data_len;
  static struct netbuf *buf;
  err_t err;
//	struct pbuf *q;
//	ip_addr_t server_ip;

	u16_t server_port;

	TCPIP_Init();

	tcp_client_server_conn = netconn_new( NETCONN_UDP );
	netconn_bind(tcp_client_server_conn, IP_ADDR_ANY, 8089);
				 
				 while(1)
				 {
				 err = netconn_recv(tcp_client_server_conn, &buf);				
					 if (err == ERR_OK) {
						dest_ipaddr.addr =  buf->addr.addr;
						dest_port =  buf->port;
//						memset(udp_server_recvbuf,0,UDP_BUF_SIZE);  //���ݽ��ջ���������
//						memcpy(udp_server_recvbuf,buf->p->payload,buf->p->len);//�յ���buffer
						 
						memset(&udp_server_recvbuf_index[udp_server_recvbuf_index_head],0,UDP_BUF_SIZE);  //���ݽ��ջ���������
						memcpy(&udp_server_recvbuf_index[udp_server_recvbuf_index_head],buf->p->payload,buf->p->len);//�յ���buffer
						dest_port_buf[udp_server_recvbuf_index_head] = dest_port;
						udp_server_recvbuf_index_head++;
						if(udp_server_recvbuf_index_head == 50)
						{
							udp_server_recvbuf_index_head = 0;
						}
						
						
						
//						for(q=buf->p;q!=NULL;q=q->next){
//							if(q->len > (100-data_len)) {
//							;
//							}else {
//								memcpy(udp_server_recvbuf+data_len,q->payload,q->len);
//								data_len += q->len;
//							}
//						}
//						err = netconn_send(tcp_client_server_conn, buf);
//						if(err != ERR_OK) {
//						printf("============================");
//						}
//						 err = netconn_sendto(tcp_client_server_conn,buf,&dest_ipaddr,dest_port);
						netbuf_delete(buf);
//						Increase(TxdBufTailIndex);
//						Analytic_Function(udp_server_recvbuf);
					 }
				 }
		
	
}




uint16_t command_Register(void)
{ 	

//	 {
//		CmdBuf[0]=0x7e;
//		CmdBuf[1]=0x02;
//		CmdBuf[2]=0x08;
//		CmdBuf[3]=0x00;
//		CmdBuf[11]=config.SNnumber;
//		CmdBuf[10]=config.SNnumber>>8;
//		CmdBuf[9]=config.SNnumber>>16;
//		CmdBuf[8]=config.SNnumber>>24;
//		CmdBuf[7]=config.SNnumber>>32;
//		CmdBuf[6]=config.SNnumber>>40;
//		CmdBuf[5]=config.SNnumber>>48;
//		CmdBuf[4]=config.SNnumber>>56;
//		CmdBuf[12]=0;
//		for(uint16_t i=1;i<12;i++)
//		CmdBuf[12]+=CmdBuf[i];
//		CmdBuf[13]=0x7e;
//	  CmdBufLength=14;
//		 for(uint8_t i = 0; i < 14; i++)
//		 {
//			TXDBUF[TxdBufTailIndex][i] =  CmdBuf[i];
//		 }
//		netconn_write( tcp_client_server_conn, TXDBUF[TxdBufTailIndex], 14, NETCONN_COPY);
////    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
//	 }
//	 return 1;
}

uint64_t KeyEmuBySN(uint64_t sn)
{
	uint64_t key=0;
	for(uint32_t i=0;i<8;i++)
		key+=(uint8_t)(config.SNnumber>>(i*8));
	return key;
	
}
void vTaskRegister(void *pvParameters)
{
//		while(1)
//		{		
//			vTaskDelay(1000);	
//			while(!(xSemaphoreTake(xServerCommunicationLockSemaphore, portMAX_DELAY) == pdTRUE))
//			
////			while(xServerCommunicationLockSemaphore==NULL)
//			{};//
//			if(REGISTER_FLAG == 0x00)
//			{
////					vTaskDelay(1000);				
//					command_Register();								
//					vTaskDelay(1000);
//					xSemaphoreGive( xServerCommunicationLockSemaphore ); /* �ͷŷ�����ͨ��ʹ��Ȩ */		
//			}
//			else
//			{
//				if(Parameter.DeviceKey!=KeyEmuBySN(config.SNnumber))
//				{
//						vTaskSuspendAll();     //У����󣬿�������������ֹ�����̣߳�������Ч
//						while(1)
//						{
//							vTaskDelay(1000); //����ѭ��
//						}
//						
//				}
//			
//			}
//		}
}


uint8_t ReadRxdBuf(void)
{ 
	uint8_t  c;
	c=RxdBuf[RxdBufTailIndex];
	
	return (c);
}



void receive_server_data_task( void )
{
	for(;;)
	{

			if(!isRxdBufEmpty()) //������ܵ����ģ�����͸��ģʽ
			{
				RxdByte(ReadRxdBuf());	
				IncreaseRxdBufNum(RxdBufTailIndex);		
			}
			else 
			{
				vTaskDelay(2);
			}
		}
}

//extern void AD7606_EnterAutoMode(uint32_t _ulFreq);

uint8_t command_start()
{ 
	//EnableIEPEPower();
//	 for(uint32_t i=0;i<add_data;i++)
//		xx[i]+=i;
	
	 {
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
	 }
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
//	 AD7606_StartRecord(config.ADfrequence);
	 return 1;
}

uint8_t command_stop()
{ 

	 {
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
	 }
	  WriteDataToTXDBUF(CmdBuf,CmdBufLength);
//	 AD7606_StopRecord();
	 return 1;
}

uint16_t command_id(void)
{ 
	
	 {
		CmdBuf[0]=0x7e;
		CmdBuf[1]=0x02;
		CmdBuf[2]=0x08;
		CmdBuf[3]=0x00;
		CmdBuf[11]=config.SNnumber;
		CmdBuf[10]=config.SNnumber>>8;
		CmdBuf[9]=config.SNnumber>>16;
		CmdBuf[8]=config.SNnumber>>24;
		CmdBuf[7]=config.SNnumber>>32;
		CmdBuf[6]=config.SNnumber>>40;
		CmdBuf[5]=config.SNnumber>>48;
		CmdBuf[4]=config.SNnumber>>56;
		CmdBuf[12]=0;
		for(uint16_t i=1;i<12;i++)
		CmdBuf[12]+=CmdBuf[i];
		CmdBuf[13]=0x7e;
	  CmdBufLength=14;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	 return 1;
}



uint8_t command_receive_beacon()
{ 
	//EnableIEPEPower();
	
	 {
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
	 }
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 ReceiveBeaconMessage();
	 return 1;
}


uint16_t command_channelkind(void)
{
//		 {
//		CmdBuf[0]=0x7e;
//		CmdBuf[1]=0x03;
//		CmdBuf[2]=0x08;
//		CmdBuf[3]=0x00;
//		CmdBuf[4]=config.Vibrate_Cnt+config.Temp_Cnt+config.Rotate_Speed_Cnt; //������һ���¶�һ�����ٶ�
//		CmdBuf[5]=0x02;  //��һ�飬ÿ�ִ������ֹ����
//		CmdBuf[6]=0x01;
//		CmdBuf[7]=config.Vibrate_Cnt;
//		CmdBuf[8]=0x02;
//		CmdBuf[9]=config.Temp_Cnt;
//		CmdBuf[10]=0x03;
//		CmdBuf[11]=config.Rotate_Speed_Cnt;	 
//		CmdBuf[12]=0x00;	 
//		for(uint16_t i=1;i<12;i++)
//		CmdBuf[12]+=CmdBuf[i];
//		 
//    CmdBuf[13]=0x7e;
//	  CmdBufLength=14;
//    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
//	 }
	 {
		CmdBuf[0]=0x7e;
		CmdBuf[1]=0x03;
		CmdBuf[2]=0x06;
		CmdBuf[3]=0x00;
		CmdBuf[4]=Acceleration_ADCHS+Temp_ADCHS; //������һ���¶�һ�����ٶ�
		CmdBuf[5]=0x02;  //��һ�飬ÿ�ִ������ֹ����
		CmdBuf[6]=0x01;
		CmdBuf[7]=Acceleration_ADCHS;
		CmdBuf[8]=0x02;
		CmdBuf[9]=Temp_ADCHS;
		CmdBuf[10]=0;
		for(uint16_t i=1;i<10;i++)
		CmdBuf[10]+=CmdBuf[i];
		 
    CmdBuf[11]=0x7e;
	  CmdBufLength=12;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	 return 1;
}
uint8_t command_setip()  //����IP��ַ  //����DHCP
{

//	{
//		for(uint8_t i=0;i<CmdBufLength;i++)
//		CmdBuf[i]=RXDCmdBuf[i];
//	  CmdBufLength=CmdBufLength;
//	 }
////	 CmdBuf[1]=0xe4; //
////	 for(uint32_t i=1;i<(CmdBufLength-2);i++)
////	 CmdBuf[CmdBufLength-2]+=RXDCmdBuf[i];
////	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
//	 
//	 
//	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
//	 for(uint8_t i=0;i<CmdBuf[5];i++)
//	 config.LocalIP[i]=CmdBuf[6+i];//��AP�ַ�����ֵ��config
//	 config.LocalIP[CmdBuf[5]]=0;
//	 for(uint8_t i=0;i<CmdBuf[7+CmdBuf[5]];i++)
//	 config.LocalMASK[i]=CmdBuf[8+CmdBuf[5]+i];//��AP�ַ�����ֵ��config
//	 config.LocalMASK[CmdBuf[7+CmdBuf[5]]]=0;
//	 for(uint8_t i=0;i<CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]];i++)
//	 config.LocalGATEWAY[i]=CmdBuf[10+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]+i];//��AP�ַ�����ֵ��config
//	 config.LocalGATEWAY[CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]]]=0;
//	 for(uint8_t i=0;i<CmdBuf[11+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]];i++)
//	 config.DHCP=CmdBuf[12+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]+CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]]];//��AP�ַ�����ֵ��config
//	 //�������ӷ�ʽ�洢������
//	 config.Connection_Method = CmdBuf[15+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]+CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]]];

//	 saveConfig();
//	 software_reset();

	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
	 }
//	 CmdBuf[1]=0xe4; //
//	 for(uint32_t i=1;i<(CmdBufLength-2);i++)
//	 CmdBuf[CmdBufLength-2]+=RXDCmdBuf[i];
//	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);	 
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 
	 for(uint8_t i=0;i<CmdBuf[5];i++)
	 config.LocalIP[i]=CmdBuf[6+i];//��AP�ַ�����ֵ��config
	 config.LocalIP[CmdBuf[5]]=0;
	 for(uint8_t i=0;i<CmdBuf[7+CmdBuf[5]];i++)
	 config.LocalMASK[i]=CmdBuf[8+CmdBuf[5]+i];//��AP�ַ�����ֵ��config
	 config.LocalMASK[CmdBuf[7+CmdBuf[5]]]=0;
	 for(uint8_t i=0;i<CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]];i++)
	 config.LocalGATEWAY[i]=CmdBuf[10+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]+i];//��AP�ַ�����ֵ��config
	 config.LocalGATEWAY[CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]]]=0;
	 //for(uint8_t i=0;i<CmdBuf[11+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]];i++)
	 config.DHCP=CmdBuf[12+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]+CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]]];//��AP�ַ�����ֵ��config
	 
	  saveConfig();
	 return 1;
}
uint8_t command_replyip()  //����IP��ַ
{
//	 uint8_t localIPlength=strlen(config.LocalIP);
//	 uint8_t LocalMASKlength=strlen(config.LocalMASK);
//	 uint8_t LocalGATEWAYlength=strlen(config.LocalGATEWAY);
//	 CmdBufLength=15+localIPlength+LocalMASKlength+LocalGATEWAYlength;
//	 CmdBuf[0]=0X7E;
//	 CmdBuf[1]=0X04;   //T
//	 CmdBuf[2]=CmdBufLength-6;
//	 CmdBuf[3]=(CmdBufLength-6)>>8;   //L
//	 CmdBuf[4]=0X00;   //V_IP
//	 CmdBuf[5]=localIPlength;
//	{
//		for(uint8_t i=0;i<localIPlength;i++)
//		CmdBuf[6+i]=config.LocalIP[i];
//	 
//	 }
//   CmdBuf[6+localIPlength]=0X01;   //V_MASK
//	 CmdBuf[7+localIPlength]=LocalMASKlength;
//	{
//		for(uint8_t i=0;i<LocalMASKlength;i++)
//		CmdBuf[8+localIPlength+i]=config.LocalMASK[i];
//	 
//	 }
//	 CmdBuf[8+localIPlength+LocalMASKlength]=0X02;   //V_GATEWAY
//	 CmdBuf[9+localIPlength+LocalMASKlength]=LocalGATEWAYlength;
//	{
//		for(uint8_t i=0;i<LocalGATEWAYlength;i++)
//		CmdBuf[10+localIPlength+LocalMASKlength+i]=config.LocalGATEWAY[i];
//	 
//	 }
//	 CmdBuf[10+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X03;
//	 CmdBuf[11+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X01;
//	 CmdBuf[12+localIPlength+LocalMASKlength+LocalGATEWAYlength]=config.DHCP;
//	 
//	 CmdBuf[13+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X04;
//	 CmdBuf[14+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X01;
//	 CmdBuf[15+localIPlength+LocalMASKlength+LocalGATEWAYlength]=config.Connection_Method;
//	 
//	 CmdBuf[16+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0;
//	 for(uint8_t i=1;i<(16+localIPlength+LocalMASKlength+LocalGATEWAYlength);i++)
//	 CmdBuf[16+localIPlength+LocalMASKlength+LocalGATEWAYlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
//	 CmdBuf[17+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0x7e;
//	 CmdBufLength=18+localIPlength+LocalMASKlength+LocalGATEWAYlength;	
//	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 uint8_t localIPlength=strlen(config.LocalIP);
	 uint8_t LocalMASKlength=strlen(config.LocalMASK);
	 uint8_t LocalGATEWAYlength=strlen(config.LocalGATEWAY);
	 CmdBufLength=15+localIPlength+LocalMASKlength+LocalGATEWAYlength;
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=0X04;   //T
	 CmdBuf[2]=CmdBufLength-6;
	 CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	 CmdBuf[4]=0X00;   //V_IP
	 CmdBuf[5]=localIPlength;
	{
		for(uint8_t i=0;i<localIPlength;i++)
		CmdBuf[6+i]=config.LocalIP[i];
	 
	 }
   CmdBuf[6+localIPlength]=0X01;   //V_MASK
	 CmdBuf[7+localIPlength]=LocalMASKlength;
	{
		for(uint8_t i=0;i<LocalMASKlength;i++)
		CmdBuf[8+localIPlength+i]=config.LocalMASK[i];
	 
	 }
	 CmdBuf[8+localIPlength+LocalMASKlength]=0X02;   //V_GATEWAY
	 CmdBuf[9+localIPlength+LocalMASKlength]=LocalGATEWAYlength;
	{
		for(uint8_t i=0;i<LocalGATEWAYlength;i++)
		CmdBuf[10+localIPlength+LocalMASKlength+i]=config.LocalGATEWAY[i];
	 
	 }
	 CmdBuf[10+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X03;
	 CmdBuf[11+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X01;
	 CmdBuf[12+localIPlength+LocalMASKlength+LocalGATEWAYlength]=config.DHCP;
	 CmdBuf[13+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0;
	 for(uint8_t i=1;i<(13+localIPlength+LocalMASKlength+LocalGATEWAYlength);i++)
	 CmdBuf[13+localIPlength+LocalMASKlength+LocalGATEWAYlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	 CmdBuf[14+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0x7e;
	 CmdBufLength=15+localIPlength+LocalMASKlength+LocalGATEWAYlength;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 return 1;
}

uint8_t command_replyap()  //����IP��ַ
{  
	//���Ҫ�ж��Ƿ񹤳�����
	 uint8_t APSSIDlength=strlen(config.APssid);
	 uint8_t APPASSWORDlength=strlen(config.APpassword);
	 CmdBufLength=10+APSSIDlength+APPASSWORDlength;
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=0X53;   //T
	 CmdBuf[2]=CmdBufLength-6;
	 CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	 CmdBuf[4]=0X00;   //V_IP
	 CmdBuf[5]=APSSIDlength;
	{
		for(uint8_t i=0;i<APSSIDlength;i++)
		CmdBuf[6+i]=config.APssid[i];
	 
	 }
   CmdBuf[6+APSSIDlength]=0X01;   //V_MASK
	 CmdBuf[7+APSSIDlength]=APPASSWORDlength;
	{
		for(uint8_t i=0;i<APPASSWORDlength;i++)
		CmdBuf[8+APSSIDlength+i]=config.APpassword[i];
	 
	 }
	 CmdBuf[8+APSSIDlength+APPASSWORDlength]=0X00;   //У���
	 
	 for(uint8_t i=1;i<(8+APSSIDlength+APPASSWORDlength);i++)
	 CmdBuf[8+APSSIDlength+APPASSWORDlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	 CmdBuf[9+APSSIDlength+APPASSWORDlength]=0x7e;
	 CmdBufLength=10+APSSIDlength+APPASSWORDlength;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 return 1;
}



float interscale=0;
extern void emu_sprase_index();
uint8_t command_reply_scale()
{  
	 unsigned char *floatCdata;
	 uint16_t channelnum=Acceleration_ADCHS;  //�������ͨ������
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=0X08;   
	 CmdBuf[2]=channelnum*17;
	 CmdBuf[3]=(channelnum*17)>>8;   //L
	 for(uint8_t i=0;i<channelnum;i++)
	 {
	 CmdBuf[4+17*i]=i;   //V_IP
	
	 CmdBuf[5+17*i]=config.channel_freq[i];
	 CmdBuf[6+17*i]=config.channel_freq[i]>>8; 
	 CmdBuf[7+17*i]=config.channel_freq[i]>>16; 
	 CmdBuf[8+17*i]=config.channel_freq[i]>>24; 		 
   interscale=1.0f/config.floatscale[i];  

	 floatCdata=(unsigned char *)&interscale;
	 CmdBuf[9+17*i]=*floatCdata;
	 CmdBuf[10+17*i]=*(floatCdata+1);;
	 CmdBuf[11+17*i]=*(floatCdata+2);
	 CmdBuf[12+17*i]=*(floatCdata+3);
	 floatCdata=(unsigned char *)&config.alarmgate[i];
	 CmdBuf[13+17*i]=*floatCdata;
	 CmdBuf[14+17*i]=*(floatCdata+1);;
	 CmdBuf[15+17*i]=*(floatCdata+2);
	 CmdBuf[16+17*i]=*(floatCdata+3);
	 floatCdata=(unsigned char *)&config.floatrange[i];
	 CmdBuf[17+17*i]=*floatCdata;
	 CmdBuf[18+17*i]=*(floatCdata+1);;
	 CmdBuf[19+17*i]=*(floatCdata+2);
	 CmdBuf[20+17*i]=*(floatCdata+3);
		 

	
	 }

	 CmdBuf[channelnum*17+4]=0;
	 for(uint16_t j=1;j<(channelnum*17+4);j++)
	 {
		CmdBuf[channelnum*17+4]+=CmdBuf[j];
	 }
	 CmdBuf[channelnum*17+5]=0x7e;
	 CmdBufLength=channelnum*17+6;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 
	 emu_sprase_index();//���²�����ϡ�����
	 
   return 1;
}



uint8_t command_adjust_adc()
{ 
	 {
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
	 }
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);


	 if(CmdBuf[4]==1){
	 config.floatscale[0]=1;
   config.floatscale[1]=1;
	 config.floatscale[2]=1;
	 config.floatadc[0]=1;
   config.floatadc[1]=1;
	 config.floatadc[2]=1;
	 return 1;
	 }
	 if(CmdBuf[5]==1){
	 config.floatadc[0]=100.0f/Parameter.EffectiveValue[0];	 		 
	 return 1;
	 }
	 if(CmdBuf[6]==1){
	 config.floatadc[1]=100.0f/Parameter.EffectiveValue[1];	 		 
	 return 1;
	 }
	 if(CmdBuf[7]==1){
	 config.floatadc[2]=100.0f/Parameter.EffectiveValue[2];	
	 return 1; 
	 } 		 
	 if(CmdBuf[8]==1){
	 config.floatadc[3]=100.0f/Parameter.EffectiveValue[3];	 		 
	 return 1;
	 }
	 if(CmdBuf[9]==1){
	 config.floatadc[4]=100.0f/Parameter.EffectiveValue[4];	 		 
	 return 1;
	 }
	 if(CmdBuf[10]==1){
	 config.floatadc[5]=100.0f/Parameter.EffectiveValue[5];
	 return 1; 
	 }		 
	 if(CmdBuf[11]==1){
	 config.floatadc[6]=100.0f/Parameter.EffectiveValue[6];	 		 
	 return 1;
	 }
	 if(CmdBuf[12]==1){
	 config.floatadc[7]=100.0f/Parameter.EffectiveValue[7];	 		 
	 return 1;
	 }
	 saveConfig();
   return 1;

	 
 


}
	 


typedef union  
{  
    float fdata;  
    unsigned long ldata;  
}FloatLongType; 


uint8_t command_set_scale()
{ 
	 {
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;

	 }
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 uint8_t channelnum=(((uint32_t)CmdBuf[3]<<8)+CmdBuf[2])/17;
	 for(uint16_t j=0;j<Acceleration_ADCHS;j++)//for(uint16_t j=0;j<channelnum;j++) //ֻ�Լ��ٶȽ�������
	 {
		FloatLongType fl;  
		fl.ldata=0;  
    config.channel_freq[j]=(((uint32_t)CmdBuf[8+17*j]<<24)+((uint32_t)CmdBuf[7+17*j]<<16)
															+((uint32_t)CmdBuf[6+17*j]<<8)+CmdBuf[5+17*j]); 
   
		fl.ldata=0;  
    fl.ldata=CmdBuf[12+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[11+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[10+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[9+17*j];  
		config.floatscale[j]=1.0f/fl.fdata;  
    
		fl.ldata=0;  
    fl.ldata=CmdBuf[16+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[15+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[14+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[13+17*j];  
		
		config.alarmgate[j]=fl.fdata; 
		fl.ldata=0; 
		fl.ldata=CmdBuf[20+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[19+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[18+17*j];  
    fl.ldata=(fl.ldata<<8)|CmdBuf[17+17*j];  
		config.floatrange[j]=fl.fdata; 
   
		Parameter.ReciprocalofRange[j]=32768/config.floatrange[j];
	 }
	 saveConfig();
   
   return 1;
}

uint8_t command_set_ap(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
#ifdef ProgramESP32ByMyself
	 CmdBuf[1]=0xe5; //
	 for(uint32_t i=1;i<(CmdBufLength-2);i++)
	 CmdBuf[CmdBufLength-2]+=CmdBuf[i];
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
#endif
	 for(uint8_t i=0;i<CmdBuf[5];i++)
	 config.APssid[i]=CmdBuf[6+i];//��AP�ַ�����ֵ��config
	 config.APssid[CmdBuf[5]]=0;
	 for(uint8_t i=0;i<CmdBuf[7+CmdBuf[5]];i++)
	 config.APpassword[i]=CmdBuf[8+CmdBuf[5]+i];//��AP�ַ�����ֵ��config
	 config.APpassword[CmdBuf[7+CmdBuf[5]]]=0;
	 return (1);
}

uint8_t command_reply_runmode(void)   //��ѯ����ģʽ
{
	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=0x56;
		CmdBuf[2]=0x01;
		CmdBuf[3]=0x00;
		CmdBuf[4]=config.DataToBoardMode; //
		
		CmdBuf[5]=0;
		for(uint16_t i=1;i<5;i++)
		CmdBuf[5]+=CmdBuf[i];		 
    CmdBuf[6]=0x7e;
	  CmdBufLength=7;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	 return 1;
}

uint16_t command_receive_active_beacon(void)
{
	ReceiveActiveBeaconMessage();
	return 1;
}


uint8_t command_setrunmode(void)   //���ù���ģʽ
{
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	 switch(CmdBuf[4]){
		 case 1:
			
		   //vTaskDelay (pdMS_TO_TICKS(10));
		   
			 config.DataToBoardMode=PARAMETERMODE;
//		   EnableTempTransmission();
		  // TPM_StartSHAOTimer(TPM1, kTPM_SystemClock); //ȥ��ͨЧ�������ڲ�����ô�ɣ�Ҫֱ�Ӽ�ȥһ��ֵ
		 break;
		 case 2:
		
			 config.DataToBoardMode=WAVEMODE;
//		   EnableTempTransmission();
	
		 break;
		 case 3:
			
			 config.DataToBoardMode=FFTWAVEMODE;
//		   EnableTempTransmission();
		 break;
		 case 4:
			
			 config.DataToBoardMode=FFTPARAMETERMODE;
//		   EnableTempTransmission();
		 break;
		 case 5:
			 config.DataToBoardMode=IDLEMODE;
//		   DisableTempTransmission();
		  
		 break;
		 case 6:
			 config.DataToBoardMode=LITEWAVEMODE;
//		   DisableTempTransmission();
		
		 break;
		 default:
		 break;
	 } 

 return 1;	
}  
uint8_t command_set_snnumber(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }

	 config.SNnumber=(uint64_t)CmdBuf[4]+((uint64_t)CmdBuf[5]<<8)+((uint64_t)CmdBuf[6]<<16)+((uint64_t)CmdBuf[7]<<24)+((uint64_t)CmdBuf[8]<<32)+((uint64_t)CmdBuf[9]<<40)+((uint64_t)CmdBuf[10]<<48)+((uint64_t)CmdBuf[11]<<56);//��AP�ַ�����ֵ��config
	 saveConfig();
	 return (1);
}
uint8_t command_device_key(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	 Parameter.DeviceKey=(uint64_t)CmdBuf[4]+((uint64_t)CmdBuf[5]<<8)+((uint64_t)CmdBuf[6]<<16)+((uint64_t)CmdBuf[7]<<24)+((uint64_t)CmdBuf[8]<<32)+((uint64_t)CmdBuf[9]<<40)+((uint64_t)CmdBuf[10]<<48)+((uint64_t)CmdBuf[11]<<56);//��AP�ַ�����ֵ��config
//	 REGISTER_FLAG = 0x01;

	 return (1);
}
uint8_t command_set_channel_condition(void)   //����ͨ����
{
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }

	 config.DataToSendChannel=((uint16_t)RXDCmdBuf[5]<<8)+RXDCmdBuf[4];;//��AP�ַ�����ֵ��config
	 //saveConfig();
	 return (1);
}

//extern void EarseFlash(uint32_t address,uint32_t length);

volatile uint32_t receive_iap_data_length=0;
volatile uint8_t receive_iap_pack_index=0;
uint8_t reply_shut_iap(void)
{
	CmdBuf[0]=0x7e;	
	CmdBuf[1]=COMMAND_IAP_STATUE;	
	CmdBuf[2]=0x05;	
	CmdBuf[3]=0x00;	
	CmdBuf[4]=0x04;	
	CmdBuf[5]=0x00;	
	CmdBuf[6]=0x00;	
	CmdBuf[7]=0x00;	
	CmdBuf[8]=0x00;	
	CmdBuf[9]=0x00;
	CmdBuf[9]=COMMAND_IAP_STATUE+0X09;	
	CmdBuf[10]=0x7e;
	CmdBufLength=11;
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	return 1;
}

uint8_t reply_complete_iap(void)
{
	CmdBuf[0]=0x7e;	
	CmdBuf[1]=COMMAND_IAP_STATUE;	
	CmdBuf[2]=0x05;	
	CmdBuf[3]=0x00;	
	CmdBuf[4]=0x03;	
	CmdBuf[5]=0x00;	
	CmdBuf[6]=0x00;	
	CmdBuf[7]=0x00;	
	CmdBuf[8]=0x00;	
	CmdBuf[9]=0x00;	
	CmdBuf[9]=COMMAND_IAP_STATUE+0X08;	
	CmdBuf[10]=0x7e;
	CmdBufLength=11;
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	return 1;
}

uint8_t reply_requie_contioue_iap(void)
{
	CmdBuf[0]=0x7e;	
	CmdBuf[1]=COMMAND_IAP_STATUE;	
	CmdBuf[2]=0x05;	
	CmdBuf[3]=0x00;	
	CmdBuf[4]=0x02;	
	CmdBuf[5]=0x00;	
	CmdBuf[6]=0x00;	
	CmdBuf[7]=0x00;	
	CmdBuf[8]=0x00;	
	CmdBuf[9]=0x00;	
	CmdBuf[9]=COMMAND_IAP_STATUE+0X07;	
	CmdBuf[10]=0x7e;
	CmdBufLength=11;
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	return 1;
}

uint8_t reply_requie_contioue_iap_data(void)
{
	CmdBuf[0]=0x7e;	
	CmdBuf[1]=COMMAND_IAP_DATA;	
	CmdBuf[2]=0x01;	
	CmdBuf[3]=0x00;	
	CmdBuf[4]=0x01;	
	CmdBuf[5]=0x00;	
	CmdBuf[5]=COMMAND_IAP_DATA+0X02;	
	CmdBuf[6]=0x7e;
	CmdBufLength=7;
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	return 1;
}

uint8_t reply_stop_iap_data(void)
{
	CmdBuf[0]=0x7e;	
	CmdBuf[1]=COMMAND_IAP_DATA;	
	CmdBuf[2]=0x01;	
	CmdBuf[3]=0x00;	
	CmdBuf[4]=0x00;	
	CmdBuf[5]=0x00;	
	CmdBuf[5]=COMMAND_IAP_DATA+0X01;	
	CmdBuf[6]=0x7e;
	CmdBufLength=7;
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	return 1;
}

uint8_t command_reply_sw_version(void)
{
	CmdBuf[0]=0x7e;	
	CmdBuf[1]=COMMAND_REPLY_SW_VERSION;	
	CmdBuf[2]=0x10;
	CmdBuf[3]=0x00;
	for(uint32_t i=0;i<16;i++) //strcpy
	{
		CmdBuf[4+i]=sw_version[i];
	}
	CmdBuf[20]=0;
	for(uint32_t i=1;i<20;i++) //strcpy
	{
		CmdBuf[20]+=CmdBuf[i];
	}
	CmdBuf[21]=0x7e;
	CmdBufLength=22;
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	return 1;
}

volatile uint32_t erase_data_length=0;
uint8_t command_iap_statue(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
		if(RXDCmdBuf[4]==0x01)
	{
	 
//	 stop_ad7606_samp();
		Parameter.IapDataLength=(uint32_t)RXDCmdBuf[5]+((uint32_t)RXDCmdBuf[6]<<8)+((uint32_t)RXDCmdBuf[7]<<16)+((uint32_t)RXDCmdBuf[8]<<24);
		if((Parameter.IapDataLength%64)!=0)
			erase_data_length=(Parameter.IapDataLength/64+1)*64;
		else 
		erase_data_length=Parameter.IapDataLength;
//	EarseFlash(IAP_ADDRESS,erase_data_length);
//		Flash_Erase(IAP_ADDRESS,erase_data_length);
	receive_iap_data_length=0;	
	receive_iap_pack_index=0;
	reply_requie_contioue_iap();
	}else if(RXDCmdBuf[4]==0x03)
	{
	receive_iap_data_length=0;	
	receive_iap_pack_index=0;
	
	reply_shut_iap();
	}
	
	 return (1);
	
//	if(RXDCmdBuf[4]==0x01)
//	{
//	 
////	 stop_ad7606_samp();
//		Parameter.IapDataLength=(uint32_t)RXDCmdBuf[5]+((uint32_t)RXDCmdBuf[6]<<8)+((uint32_t)RXDCmdBuf[7]<<16)+((uint32_t)RXDCmdBuf[8]<<24);
//		if((Parameter.IapDataLength%512)!=0)
//			erase_data_length=(Parameter.IapDataLength/512+1)*512;
//		else 
//		erase_data_length=Parameter.IapDataLength;
////	EarseFlash(IAP_ADDRESS,erase_data_length);
//	receive_iap_data_length=0;	
//	receive_iap_pack_index=0;
//	reply_requie_contioue_iap();
//	}else if(RXDCmdBuf[4]==0x03)
//	{
//	receive_iap_data_length=0;	
//	receive_iap_pack_index=0;
//	
//	reply_shut_iap();
//	}
//	
//	 return (1);
}
extern void WRBYTES(uint32_t address,uint32_t num, uint8_t *Data);
extern uint8_t WRBYTES_iap_data(uint32_t address,uint32_t num, uint8_t *Data);
uint8_t rece_iap_data[128];
uint8_t error_iap;

uint32_t iap_num=0;

void WRBYTES_iap_data_sdram(uint32_t addr,uint8_t *data,uint32_t length)
{
	uint32_t i=0;
	for(i=0;i<length;i++)
//		iap_data_in_sdram[iap_num++] = data[i];
		iap_data_in_sdram[addr+i]=data[i];
}
uint8_t command_iap_data(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
		uint8_t write_32_Byte[64]={0};
	if(receive_iap_pack_index==RXDCmdBuf[7])
	{
	receive_iap_pack_index++;
	uint32_t current_iap_pack_length=RXDCmdBuf[2]+((uint16_t)RXDCmdBuf[3]<<8)-4;
		if(Parameter.IapDataLength>=(receive_iap_data_length+current_iap_pack_length))
		{
		
		if(Parameter.IapDataLength==(receive_iap_data_length+current_iap_pack_length))
		{
			for(uint32_t i=0;i<64;i++ )
			write_32_Byte[i]=0;
			for(uint32_t i=0;i<current_iap_pack_length;i++ )
			{
			iap_data_in_sdram[iap_num++] = RXDCmdBuf[8+i];
			}
		}
		else
		{
			for(uint32_t i=0;i<64;i++ )
			{
				write_32_Byte[i]=0;
			}
			for(uint32_t i=0;i<current_iap_pack_length;i++ )
			{
				iap_data_in_sdram[iap_num++] = RXDCmdBuf[8+i];
			}
		
		}
		receive_iap_data_length+=current_iap_pack_length;
			if(Parameter.IapDataLength==receive_iap_data_length)
			{
					STMFLASH_Write(IAP_ADDRESS,(uint32_t *)iap_data_in_sdram,(erase_data_length/4));
//				WRBYTES_iap_data(IAP_ADDRESS,receive_iap_data_length,iap_data_in_sdram);
					reply_complete_iap();
				  config.Iap_datalength=erase_data_length;
				  config.Iap_flag=0x01;
				  saveConfig();
				  software_reset();
			}
			else 
				reply_requie_contioue_iap_data();
		}
		else
		{
		receive_iap_data_length=0;	
		receive_iap_pack_index=0;	
		reply_stop_iap_data();
		}
	}else
	{
	reply_stop_iap_data();	
	}
	 return (1);
	
//	uint8_t write_32_Byte[513]={0};
//	if(receive_iap_pack_index==RXDCmdBuf[7])
//	{
//		receive_iap_pack_index++;
//		uint32_t current_iap_pack_length=RXDCmdBuf[2]+((uint16_t)RXDCmdBuf[3]<<8)-4;
//		if(Parameter.IapDataLength>=(receive_iap_data_length+current_iap_pack_length))
//		{
//		
//		if(Parameter.IapDataLength==(receive_iap_data_length+current_iap_pack_length))
//		{
//			for(uint32_t i=0;i<512;i++ )
//			write_32_Byte[i]=0;
//			for(uint32_t i=0;i<current_iap_pack_length;i++ )
//			write_32_Byte[i]=RXDCmdBuf[8+i];
//			WRBYTES_iap_data_sdram(receive_iap_data_length,write_32_Byte,512);
//			
////				WRBYTES(IAP_ADDRESS+receive_iap_data_length,current_iap_pack_length,(uint8_t*)&iap_data_in_sdram);
////								STMFLASH_Write(IAP_ADDRESS+receive_iap_data_length,(uint32_t *)iap_data_in_sdram,current_iap_pack_length);
//		}
//		else{
//			WRBYTES_iap_data_sdram(receive_iap_data_length,(uint8_t *)RXDCmdBuf+8,current_iap_pack_length);
////				WRBYTES(IAP_ADDRESS+receive_iap_data_length,current_iap_pack_length,(uint8_t*)&iap_data_in_sdram);
////				STMFLASH_Write(IAP_ADDRESS+receive_iap_data_length,(uint32_t *)iap_data_in_sdram,current_iap_pack_length);
//		}
//		receive_iap_data_length+=current_iap_pack_length;
//			if(Parameter.IapDataLength==receive_iap_data_length)
//			{
////					__disable_irq();
////					WRBYTES(IAP_ADDRESS,current_iap_pack_length,(uint8_t*)&iap_data_in_sdram);
////					STMFLASH_Write(IAP_ADDRESS,(uint32_t *)iap_data_in_sdram,(erase_data_length/4));
////					__enable_irq();
//					//дflash
////				WRBYTES(IAP_ADDRESS,receive_iap_data_length,(uint8_t*)&iap_data_in_sdram);
//				
//					__disable_irq();
//					STMFLASH_Write(IAP_ADDRESS,(uint32_t *)iap_data_in_sdram,(erase_data_length/4));
//					__enable_irq();
//				
//				
//					reply_complete_iap();
//				  config.Iap_datalength=erase_data_length;
//				  config.Iap_flag=0x01;
//				  saveConfig();
//				  software_reset();
//			}
//			else 
//				reply_requie_contioue_iap_data();
//		}
//		else
//		{
//		receive_iap_data_length=0;	
//		receive_iap_pack_index=0;	
//		reply_stop_iap_data();
//		}
//	}else
//	{
//		reply_stop_iap_data();	
//	}
//	 //saveConfig();
//	 return (1);
}



uint16_t command_set_factory_parameter(void)
{
	return 1;
}

//extern void BoardPeroidWave(void); 

RTC_T Requirdperiodwave_tRTC;
uint8_t command_require_periodwave(void)
{ 
	{
		Requirdperiodwave_tRTC.Year=g_tRTC.Year;
		Requirdperiodwave_tRTC.Mon=g_tRTC.Mon;
		Requirdperiodwave_tRTC.Day=g_tRTC.Day; //ʱ����32λ��ʾ
		Requirdperiodwave_tRTC.Hour=g_tRTC.Hour;
		Requirdperiodwave_tRTC.Min=g_tRTC.Min;
		Requirdperiodwave_tRTC.Sec=g_tRTC.Sec; //ʱ����32λ��ʾ		
		
//		for(uint8_t i=0;i<CmdBufLength;i++)
//		CmdBuf[i]=RXDCmdBuf[i];
//	  CmdBufLength=CmdBufLength;
//    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	}
	config.RequirePeriodChannel=((uint16_t)RXDCmdBuf[5]<<8)+RXDCmdBuf[4];

//	if(config.DataToBoardMode==PARAMETERMODE)
	{
		EnablePeroidWaveTransmission();
		Parameter.PeroidWaveTransmissionCounter=((uint16_t)RXDCmdBuf[7]<<8)+RXDCmdBuf[6];
	}
	return 1;
}
uint8_t command_reply_channel_condition(void)   //����ͨ����
{
	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=0x57;
		CmdBuf[2]=0x02;
		CmdBuf[3]=0x00;
		CmdBuf[4]=config.DataToSendChannel; //������һ���¶�һ�����ٶ�
		CmdBuf[5]=config.DataToSendChannel>>8; //������һ���¶�һ�����ٶ�
		CmdBuf[6]=0;
		for(uint16_t i=1;i<6;i++)
		CmdBuf[6]+=CmdBuf[i];
		 
    CmdBuf[7]=0x7e;
	  CmdBufLength=8;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	 return 1;
}

uint8_t command_reply_beacon_parameter(void)   //������������ʽ
{
	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=COMMAND_REPLY_BEACON_PARAMETER;
		CmdBuf[2]=0x02;
		CmdBuf[3]=0x00;
		CmdBuf[4]=config.Enable_active_beacon; //
		CmdBuf[5]=config.BeaconInterval;
		CmdBuf[6]=0;
		for(uint16_t i=1;i<6;i++)
		CmdBuf[6]+=CmdBuf[i];
		 
    CmdBuf[7]=0x7e;
	  CmdBufLength=8;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }

	 return 1;
}

uint8_t command_set_beacon_parameter(void)   //������������ʽ
{
 {
	for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	CmdBufLength=CmdBufLength;
 }
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	{
	config.Enable_active_beacon=CmdBuf[4]; //
	config.BeaconInterval=CmdBuf[5];	
	}
	saveConfig();
	 return 1;
}

uint8_t command_reply_SampleParameter(void)   //���ò�������
{


	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=COMMAND_REPLY_RATE;
		CmdBuf[2]=0x0B;
		CmdBuf[3]=0x00;
		CmdBuf[4]=config.ADfrequence; //������һ���¶�һ�����ٶ�
		
		CmdBuf[5]=config.ADfrequence>>8;
		CmdBuf[6]=config.ADfrequence>>16; //������һ���¶�һ�����ٶ�
		
		CmdBuf[7]=config.ADfrequence>>24;
    CmdBuf[8]=config.ADtime;
		CmdBuf[9]=config.workcycleseconds;
		CmdBuf[10]=config.workcycleseconds>>8;
		CmdBuf[11]=(uint8_t)config.PeriodTransimissonStatus;
//		CmdBuf[12]=(config.PeriodTransimissonCounter*config.workcycleseconds);
//		CmdBuf[13]=(config.PeriodTransimissonCounter*config.workcycleseconds)>>8;
		CmdBuf[12]=config.PeriodTransimissonCounter;
		CmdBuf[13]=config.PeriodTransimissonCounter>>8;
		CmdBuf[14]=(uint8_t)config.ParameterTransimissonStatus;
		CmdBuf[15]=0;
		for(uint16_t i=1;i<15;i++)
		CmdBuf[15]+=CmdBuf[i];
		 
    CmdBuf[16]=0x7e;
	  CmdBufLength=17;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }

	 return 1;
}


uint8_t command_set_SampleParameter(void)   //���ò�������
{
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
 
	 
	 config.ADfrequence=  (uint32_t)CmdBuf[4]+((uint32_t)CmdBuf[5]<<8)+((uint32_t)CmdBuf[6]<<16)+((uint32_t)CmdBuf[7]<<24);//���ò�������
//	 config.ADtime= 1;//CmdBuf[8]; //AD����ʱ��
	 if(config.ADtime>2) config.ADtime=2;   //����AD����ʱ��Ϊ2S
	 config.workcycleseconds =(uint16_t)CmdBuf[9]+((uint16_t)CmdBuf[10]<<8);

	 config.PeriodTransimissonStatus=(bool)CmdBuf[11]; //�Ƿ�ʹ���Զ�����
	 config.PeriodTransimissonCounter=(uint16_t)CmdBuf[12]+((uint16_t)CmdBuf[13]<<8);	 
//	 uint32_t counter=(uint16_t)CmdBuf[12]+((uint16_t)CmdBuf[13]<<8);	  //�Ƿ�ʹ���Զ�����
//	 config.PeriodTransimissonCounter=(uint32_t)(counter/config.workcycleseconds); //�Զ���������
	 config.ParameterTransimissonStatus=(bool)CmdBuf[14];

	 
	// TPM_SetTimerPeriod(TPM1, ((CLOCK_GetFreq(kCLOCK_PllFllSelClk)/1)/(config.ADfrequence*ADCHS)-1));
//	 Parameter.ReciprocalofADfrequence=1.0f/((float)config.ADfrequence); //����Ƶ�ʵ���;
//	 Parameter.ReciprocalofEMUnumber=1.0f/((float)config.ADfrequence*config.ADtime); //������ʱ���й�	 
	 saveConfig();
	 return (1);
}


uint8_t command_set_time(void)   //����ʱ��
{ 
	
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	RTC_WriteClock(((uint16_t)CmdBuf[5]<<8)+CmdBuf[4],CmdBuf[6],CmdBuf[7],CmdBuf[8],CmdBuf[9],CmdBuf[10]);
//	 date.year = ((uint16_t)CmdBuf[5]<<8)+CmdBuf[4];
//	date.month = CmdBuf[6];
//	date.day = CmdBuf[7];
//	date.hour = CmdBuf[8];
//	date.minute = CmdBuf[9];
//	date.second = CmdBuf[10];
	 return (1);
}
uint8_t command_replydns()  //����IP��ַ
{
	 uint8_t DNS_SERVERIPlength=strlen(config.DNS_SERVERIP);
	 CmdBufLength=8+DNS_SERVERIPlength;
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=COMMAND_REPLY_DNS;   //T
	 CmdBuf[2]=CmdBufLength-6;
	 CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	 CmdBuf[4]=0X00;   //V_IP
	 CmdBuf[5]=DNS_SERVERIPlength;
	 {
		for(uint8_t i=0;i<DNS_SERVERIPlength;i++)
		CmdBuf[6+i]=config.DNS_SERVERIP[i];	 
	 }
	 CmdBuf[6+DNS_SERVERIPlength]=0X00;   //У���
	 
	 for(uint8_t i=1;i<(6+DNS_SERVERIPlength);i++)
	 CmdBuf[6+DNS_SERVERIPlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	 CmdBuf[7+DNS_SERVERIPlength]=0x7e;
	 CmdBufLength=8+DNS_SERVERIPlength;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 return 1;
}

uint8_t command_set_dns(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	 for(uint8_t i=0;i<CmdBuf[5];i++)
	 config.DNS_SERVERIP[i]=CmdBuf[6+i];//��AP�ַ�����ֵ��config
	 config.DNS_SERVERIP[CmdBuf[5]]=0;
	 
	 return (1);
}


uint8_t command_replytcpserve()  //����IP��ַ
{
	 uint8_t TcpServer_IPlength=strlen(config.TcpServer_IP);
	 uint8_t TcpServer_Portlength=strlen(config.TcpServer_Port);
	 uint8_t server_addresslength=strlen(config.server_address);
	 CmdBufLength=12+TcpServer_Portlength+TcpServer_IPlength+server_addresslength;
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=COMMAND_REPLYTCPSERVE;   //T
	 CmdBuf[2]=CmdBufLength-6;
	 CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	 CmdBuf[4]=0X00;   //V_IP
	 CmdBuf[5]=TcpServer_IPlength;
	 {
		for(uint8_t i=0;i<TcpServer_IPlength;i++)
		CmdBuf[6+i]=config.TcpServer_IP[i];	 
	 }
   CmdBuf[6+TcpServer_IPlength]=0X01;   //V_MASK
	 CmdBuf[7+TcpServer_IPlength]=TcpServer_Portlength;
	 {
		for(uint8_t i=0;i<TcpServer_Portlength;i++)
		CmdBuf[8+TcpServer_IPlength+i]=config.TcpServer_Port[i];	 
	 }
	 CmdBuf[8+TcpServer_IPlength+TcpServer_Portlength]=0X02;   //V_MASK
	 CmdBuf[9+TcpServer_IPlength+TcpServer_Portlength]=server_addresslength;
	 {
		for(uint8_t i=0;i<server_addresslength;i++)
		CmdBuf[10+TcpServer_IPlength+TcpServer_Portlength+i]=config.server_address[i];	 
	 }
	 CmdBuf[10+TcpServer_IPlength+TcpServer_Portlength+server_addresslength]=0X00;   //У���
	 
	 for(uint8_t i=1;i<(10+TcpServer_IPlength+TcpServer_Portlength+server_addresslength);i++)
	 CmdBuf[10+TcpServer_IPlength+TcpServer_Portlength+server_addresslength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	 CmdBuf[11+TcpServer_IPlength+TcpServer_Portlength+server_addresslength]=0x7e;
	 CmdBufLength=12+TcpServer_IPlength+TcpServer_Portlength+server_addresslength;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 return 1;
}

uint8_t command_set_tcpserve(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
	{
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
    WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 }
	
//	 	 CmdBuf[1]=0xe7; //
//	 for(uint32_t i=1;i<(CmdBufLength-2);i++)
//	 CmdBuf[CmdBufLength-2]+=RXDCmdBuf[i];
//	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 
	 
	 for(uint8_t i=0;i<CmdBuf[5];i++)
	 config.TcpServer_IP[i]=CmdBuf[6+i];//��AP�ַ�����ֵ��config
	 config.TcpServer_IP[CmdBuf[5]]=0;
	 for(uint8_t i=0;i<CmdBuf[7+CmdBuf[5]];i++)
	 config.TcpServer_Port[i]=CmdBuf[8+CmdBuf[5]+i];//��AP�ַ�����ֵ��config
	 config.TcpServer_Port[CmdBuf[7+CmdBuf[5]]]=0;
	 for(uint8_t i=0;i<CmdBuf[9+CmdBuf[7+CmdBuf[5]]+CmdBuf[5]];i++)
	 config.server_address[i]=CmdBuf[10+CmdBuf[7+CmdBuf[5]]+CmdBuf[5]+i];//��AP�ַ�����ֵ��config
	 config.server_address[CmdBuf[CmdBuf[9+CmdBuf[7+CmdBuf[5]]+CmdBuf[5]]]]=0;
	 
	 
	   saveConfig();
	 return (1);
}

uint8_t command_applynetset()
{ 
	{
	for(uint8_t i=0;i<CmdBufLength;i++)
	CmdBuf[i]=RXDCmdBuf[i];
	CmdBufLength=CmdBufLength;
	
 }

//	CmdBuf[0]=0x7e;
//	CmdBuf[1]=0xff;
//	CmdBuf[2]=0x00;
//	CmdBuf[3]=0x00;
//	CmdBuf[4]=0xff;
//	CmdBuf[5]=0x7e;
//  CmdBufLength=6;
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
 	config.Connection_Method = connect_md;
  saveConfig();
	HAL_Delay(200);
	software_reset();
	return 1;
	
}

uint8_t command_reset_system(void)
{
	software_reset();
	return 1;
}

uint8_t AnalyCmd(uint16_t length)
{ 

	switch( RXDCmdBuf[1] ){     //��������
		case COMMAND_ID:     //0x02
			   command_id();
			break;
		case COMMAND_CHANNELKIND:     //0x03 ͨ����������
			   command_channelkind();
			break;
		case COMMAND_REPLYIP:     //0x03 ͨ����������
			   command_replyip();
			break;
		case COMMAND_STOP:
			   command_stop();
			break;
		case COMMAND_RECEIVE_BEACON:
				command_receive_beacon();
			break;
		case COMMAND_SETIP:   //����IP��ַ
			   command_setip();
			break;
		case COMMAND_SET_RUNMODE:   //����IP��ַ
			   command_setrunmode();
			break;
		case COMMAND_REPLY_RUNMODE:
			   command_reply_runmode();
		  break;
		case COMMAND_START:
			   command_start();
			break;
		case COMMAND_REQUIRE_PERIODWAVE:
			   command_require_periodwave();
			break;
		case COMMAND_RECEIVE_ACTIVE_BEACON:
					command_receive_active_beacon();
			break;
		case COMMAND_SET_FACTORY_PARAMETER:
					command_set_factory_parameter();
			break;
		case COMMAND_SET_BEACON_PARAMETER:
			   command_set_beacon_parameter();
			break;
		case COMMAND_REPLY_BEACON_PARAMETER:
			   command_reply_beacon_parameter();
			break;		
		case COMMAND_SET_CHANNEL_CONDITION:
			   command_set_channel_condition();
			break;
		case COMMAND_REPLY_CHANNEL_CONDITION:
			   command_reply_channel_condition();
			break;
		case COMMAND_REPLY_RATE:    //���ò�������
			   command_reply_SampleParameter();
		  break;
		case COMMAND_SAMPLE_RATE:    //���ò�������
			   command_set_SampleParameter();
			break;
		case COMMAND_SET_TCPSERVE:   //����tcp server��Ŀ���ַ����
			   command_set_tcpserve();
		  break;
		case COMMAND_REPLYTCPSERVE:   //����tcp server��Ŀ���ַ����
			   command_replytcpserve();
			break; 
		case COMMAND_SET_DNS:   //����tcp server��Ŀ���ַ����
			   command_set_dns();
		  break;
		case COMMAND_REPLY_DNS:   //����tcp server��Ŀ���ַ����
			   command_replydns();
			break;
		case COMMAND_SET_TIME:           //����apֵ
			   command_set_time();
      break;
		case COMMAND_SET_AP:
			   command_set_ap();
			break;		
		case COMMAND_REPLYAP:
			   command_replyap();  //����apֵ
			break;
    case COMMAND_APPLYNETSET:
			   command_applynetset();  //Ӧ����������
			break;
//		 case 0x40:
//			   command_counter();  //Ӧ����������
//			break;
		case COMMAND_ADJUST_TEMP:
			   command_adjust_temp();
		  break;
//		case COMMAND_ADJUST_ADC:
//			   command_adjust_adc();
//		  break;
		case COMMAND_SET_SCALE:
			   command_set_scale();
			break;
		case COMMAND_REPLY_SCALE:
			   command_reply_scale();
			break;
    case COMMAND_SET_SNNUMBER:
			   command_set_snnumber();
			break;
		case COMMAND_IAP_STATUE:
			   command_iap_statue();
			break;
		case COMMAND_REPLY_SW_VERSION:
			   command_reply_sw_version();
			break;
		case COMMAND_DEVICE_KEY:
			   command_device_key();
			break;		
		case COMMAND_IAP_DATA:
			   command_iap_data();
			break;
		case COMMAND_RESET_SYSTEM:
			   command_reset_system();
			break;
		case COMMAND_SET_CHANNELKIND:
			 set_channelkind();
			break;
		case COMMAND_REPLY_CHANNELKIND:
			 reply_channelkind();
			break;
		case COMMAND_ESP32_STATUE:
			 command_esp32_statue();
			break;
		case COMMAND_REPLY_Connection_Method:
			 command_reply_connect_method();
			break;
		case COMMAND_SET_Connection_Method:
			 command_set_connect_method();
			break;
		case COMMAND_ADJUST_4_20MA:
			command_adjust_4_20ma();
			break;
		default:
			return 1;
	}
 return 1;
	
}
float temp_4_20ma[8];

uint8_t command_adjust_4_20ma(void)
{
	 {for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;

	 }
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
//#ifdef Integrated_Sensor   //ʹ��PT1000   //����1000�Ľ�һ��������1200�Ľ�һ��
	 if(CmdBuf[4]==1)
		 {
		 for(uint32_t ii=0;ii<8;ii++)
		 {

			 if((config.interface_type[ii]==TYPE_4_20MA) && (CmdBuf[6] == (ii+1)))
			 {				 	
						temp_4_20ma[ii]=Parameter.average[ii];	
			 }else
			 {
				continue;
			 }

		 }
	 
	 return 1;
	 }
	  
	 if(CmdBuf[5]==1){
	 for(uint32_t ii=0;ii<8;ii++){
			if((config.interface_type[ii]==TYPE_4_20MA) && (CmdBuf[6] == (ii+1)))
		 {
				config.floatadjust[ii] = (Parameter.average[ii] * 0.0 - temp_4_20ma[ii] * 16.0) / (temp_4_20ma[ii]-Parameter.average[ii]);
				config.floatscale[ii]= (0.0 + config.floatadjust[ii]) / temp_4_20ma[ii];
		 }else
		 {
			continue;
		 }
		 
	 }
//#endif

	 saveConfig();
   return 1;
	 }
	 return 1;
}

void command_set_connect_method(void)
{
	 {for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
	 }
	 //�����Ȳ���ֵ����Ӧ�����磬�������ǰ��ֵ
	 connect_md = CmdBuf[4];
//	 config.Connection_Method = CmdBuf[4];  //0����wifi����   1������������

}

void command_reply_connect_method(void)
{
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=0X95;   
	 CmdBuf[2]=0x01;
	 CmdBuf[3]=0x00;   //L
	 CmdBuf[4]=config.Connection_Method;   //0����wifi����   1������������
	 CmdBuf[5] = 0;
	for(uint16_t i = 1; i < 5; i++)
	{
		CmdBuf[5]+=CmdBuf[i];//У��ֵ
	}
	
	 CmdBuf[6]=0x7e;
	 CmdBufLength=7;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
}


volatile uint16_t esp32_ready=0;
volatile uint16_t WifiConnectedFlag=0;
volatile uint16_t WifiDisconnectedFlag=0;
uint8_t command_esp32_statue(void)
{ 
	switch(RXDCmdBuf[4]){
		case 0x01:
			esp32_ready=1;
//			bsp_LedToggle(1);
		break;
		case 0x02:
			WifiConnectedFlag=1;
		break;
		case 0x03:
			WifiDisconnectedFlag=1;
		break;
		default:
			break;

	}
	 return 1;
}


float temp0[8];
uint8_t command_adjust_temp(void)
{ 
	 {for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;

	 }
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
//#ifdef Integrated_Sensor   //ʹ��PT1000   //����1000�Ľ�һ��������1200�Ľ�һ��
	 if(CmdBuf[4]==1){
	 for(uint32_t ii=0;ii<8;ii++)
	 {
		 if(config.interface_type[ii]==TYPE_PT)
		 {
		 		  temp0[ii]=Parameter.average[ii];	
		 }else
		 {
			continue;
		 }

	 }
	 
	 return 1;
	 }
	  
	 if(CmdBuf[5]==1){
	 for(uint32_t ii=0;ii<8;ii++){
		 if(config.interface_type[ii]==TYPE_PT)
		 {
		 	  config.floatadjust[ii] = (Parameter.average[ii] * 1000.0 - temp0[ii] * 1200.0) / (temp0[ii]-Parameter.average[ii]);
				config.floatscale[ii]= (1000.0 + config.floatadjust[ii]) / temp0[ii];
		 }else
		 {
			continue;
		 }
		 
	 }
//#endif

	 saveConfig();
   return 1;
	 }
	 return 1;
}



void reply_channelkind(void)//��ȡͨ������,����
{
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=0X93;   
	 CmdBuf[2]=0x10;
	 CmdBuf[3]=0x00;   //L
	 for(uint8_t i = 0;i<Acceleration_ADCHS;i++)
	 {
		 CmdBuf[4+2*i]=i+1;   //ͨ����
		 CmdBuf[5+2*i] = config.interface_type[i];//ͨ������

	 }
	 CmdBuf[20] = 0;
	for(uint16_t i = 1; i < 20; i++)
	{
		CmdBuf[20]+=CmdBuf[i];//У��ֵ
	}
	 

	 CmdBuf[21]=0x7e;
	 CmdBufLength=22;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
}


void set_channelkind(void)
{
	 {
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;

	 }
	 	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 	for(uint8_t i = 0;i<Acceleration_ADCHS;i++)
	 {
			config.interface_type[i] = CmdBuf[5+i*2];//				 
	 }
		saveConfig();
 



	 
}


uint16_t getTelLength(void)  //�����TLV��V�ĳ���
{
					
	return(((uint16_t)RxdTelBuf[3]<<8)+RxdTelBuf[2]);
  
}
uint8_t  isVaildTel(void)
{
	if(RxdBytes>=1)if(RxdTelBuf[0]!=0x7e)return(0);
	if(RxdBytes>=2)if((RxdTelBuf[1]!=COMMAND_START)&&(RxdTelBuf[1]!=COMMAND_STOP)&&(RxdTelBuf[1]!=COMMAND_ID)
		                  &&(RxdTelBuf[1]!=COMMAND_CHANNELKIND)&&(RxdTelBuf[1]!=COMMAND_REPLYIP)&&(RxdTelBuf[1]!=COMMAND_SETIP)
						  &&(RxdTelBuf[1]!=COMMAND_REPLY_RATE)&&(RxdTelBuf[1]!=COMMAND_SAMPLE_RATE)&&(RxdTelBuf[1]!=COMMAND_ADJUST_TEMP)
						  &&(RxdTelBuf[1]!=COMMAND_REPLY_SCALE)&&(RxdTelBuf[1]!=COMMAND_SET_SCALE)&&(RxdTelBuf[1]!=COMMAND_ADJUST_ADC)
						  &&(RxdTelBuf[1]!=COMMAND_SET_SNNUMBER)&&(RxdTelBuf[1]!=COMMAND_REQUIRE_PERIODWAVE)&&(RxdTelBuf[1]!=COMMAND_SET_CHANNEL_CONDITION)
							&&(RxdTelBuf[1]!=COMMAND_SET_RUNMODE)&&(RxdTelBuf[1]!=COMMAND_SET_TIME)&&(RxdTelBuf[1]!=COMMAND_SET_AP)
						  &&(RxdTelBuf[1]!=COMMAND_RECEIVE_BEACON)&&(RxdTelBuf[1]!=COMMAND_SET_TCPSERVE)&&(RxdTelBuf[1]!=COMMAND_REPLYAP)
						  &&(RxdTelBuf[1]!=COMMAND_REPLYTCPSERVE)&&(RxdTelBuf[1]!=COMMAND_APPLYNETSET)&&(RxdTelBuf[1]!=COMMAND_REPLY_RUNMODE)
							&&(RxdTelBuf[1]!=COMMAND_REPLY_CHANNEL_CONDITION)&&(RxdTelBuf[1]!=COMMAND_RECEIVE_ACTIVE_BEACON)&&(RxdTelBuf[1]!=COMMAND_SET_FACTORY_PARAMETER)&&(RxdTelBuf[1]!=COMMAND_ADJUST_4_20MA)
							&&(RxdTelBuf[1]!=COMMAND_IAP_DATA)&&(RxdTelBuf[1]!=COMMAND_IAP_STATUE)&&(RxdTelBuf[1]!=COMMAND_REPLY_SW_VERSION)&&(RxdTelBuf[1]!=COMMAND_RESET_SYSTEM)&&(RxdTelBuf[1]!=COMMAND_REPLY_Connection_Method)
							&&(RxdTelBuf[1]!=COMMAND_SET_BEACON_PARAMETER)&&(RxdTelBuf[1]!=COMMAND_REPLY_BEACON_PARAMETER)&&(RxdTelBuf[1]!=COMMAND_ESP32_STATUE)&&(RxdTelBuf[1]!=COMMAND_SET_Connection_Method)
							&&(RxdTelBuf[1]!=COMMAND_SET_DNS)&&(RxdTelBuf[1]!=COMMAND_REPLY_DNS)&&(RxdTelBuf[1]!=COMMAND_DEVICE_KEY)&&(RxdTelBuf[1]!=COMMAND_SET_CHANNELKIND)&&(RxdTelBuf[1]!=COMMAND_REPLY_CHANNELKIND))
				return(0);  //���������ID
	
  if(RxdBytes>=4) { 
		uint16_t length=getTelLength();
		if((length>1000)) return(0);  //��������Ϊ1000
	}
	return(1);				 // �Ϸ���
}
uint8_t sumofRxdBuf(uint16_t l)  //��͵ı��ģ���������ʼ��ʶ,	l	�ĳ��Ȱ�����ʼ��ʶ
{ 
	uint8_t sum=0;
	if(l<2) return (0);
	for(uint16_t i=1;i<l-2;i++)
	 sum=sum+RxdTelBuf[i];
	return (sum);
}
uint8_t isTelComplete(void)	   // =0 ������  =1 sum Error =2 ��ȷ
{
	uint32_t  temp8;
	uint32_t   dat_len;

	if(RxdBytes<4)return(0);
  ////////////////
	dat_len=getTelLength()+6;	//
	if(dat_len==0)return(0);
	if(RxdBytes<(dat_len))return(0);

	temp8=sumofRxdBuf(dat_len);
 
  if (RxdTelBuf[dat_len-1]==0x7e)
		return(2); 
	if (RxdTelBuf[dat_len-2]==temp8)
		return(2); 
	else{
		return(1);
	}	
}						 

uint8_t leftRxdTel(void)		//��������һλ
{
	uint32_t i;
	if(RxdBytes<1)return(0);     // �޷�����
	for	(i=1;i<RxdBytes;i++)
	{
		RxdTelBuf[i-1]=RxdTelBuf[i];		
	}
	RxdBytes--;
	return(1);					 // ����һ���ֽڳɹ�

}

 void RxdByte(uint8_t c)
{	
	uint32_t 	i;
	RxdTelBuf[RxdBytes]=c;
	RxdBytes++;

	switch(RxdBytes)
	{
		case 0:	break;
		case 3:	break;
		case 1:
		case 2:
		case 4:while(!isVaildTel())	//������Ϸ�			 
				{
					if(!leftRxdTel())break;	  // �������ֽ�
				}
				break;
			
		
		default:		
				i=isTelComplete();
				if(i==2)
				{
					//do some thing
					for(uint16_t j=0;j<RxdBytes;j++)
					RXDCmdBuf[j]=RxdTelBuf[j];
					CmdBufLength=RxdBytes;
					AnalyCmd(CmdBufLength);
					
					RxdBytes=0;
					
				}
				else if(i==1)	 // CRC error
				{
					leftRxdTel();
					while(!isVaildTel())	//������Ϸ�			 
					{
						if(!leftRxdTel())break;
					}	
				}
				else if(i==0) //û���������
				{
				
				}
				else
				{
				}
				break;
			
		}
	
}
int send_server_data( uint8_t *data, uint16_t len )
{
												 
	if( tcp_client_server_conn )
	{
		return netconn_write( tcp_client_server_conn, data, len, NETCONN_COPY);
	}
	else
		
	
		return ERR_CONN; 

}

#endif /* LWIP_NETCONN */

















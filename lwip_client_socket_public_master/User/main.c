/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.1
*	˵    �� : 
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2018-12-12   Eric2013     1. CMSIS����汾 V5.4.0
*                                     2. HAL��汾 V1.3.0
*
*   V1.1    2019-04-01   suozhang     1. add FreeRTOS V10.20
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* �ײ�Ӳ������ */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"

#include "cmsis_armcc.h"
/**
 * Log default configuration for EasyLogger.
 * NOTE: Must defined before including the <elog.h>
 */
#if !defined(LOG_TAG)
#define LOG_TAG                    "main_test_tag:"
#endif
#undef LOG_LVL
#if defined(XX_LOG_LVL)
    #define LOG_LVL                    XX_LOG_LVL
#endif

//#include "elog.h"

#include "lwip/netif.h"
#include "lwip/tcpip.h"

#include "app.h"
#include "iwdg.h"
extern uint16_t command_Register(void);
void Esp32ProcessFunction(void *pvParameters);
static void vTaskLED (void *pvParameters);
extern void vTaskRegister(void *pvParameters);
extern void vTaskLwip(void *pvParameters);
static void vTaskTlvloop(void *pvParameters);
static void vTaskTSendBuf(void *pvParameters);
static void vTaskTDataProcess(void *pvParameters);
static void vTaskDataEmu(void *pvParameters);

extern SemaphoreHandle_t xServerregisterSemaphore;

TaskHandle_t xHandleTaskLED  = NULL;
TaskHandle_t xHandleTaskRegister = NULL;
TaskHandle_t xHandleTaskLwip = NULL;
TaskHandle_t xHandleTaskTlvloop = NULL;
TaskHandle_t xHandleTaskSendbuf = NULL;
TaskHandle_t xHandleDataProcess = NULL;
TaskHandle_t xHandleDataEmu = NULL;

TaskHandle_t xHandleEsp32ProcessFunction = NULL;
SemaphoreHandle_t sem_spi_idle;
SemaphoreHandle_t WRITE_ready;
SemaphoreHandle_t SPI_sem_req;
SemaphoreHandle_t AD7606_ready;
SemaphoreHandle_t SAMPLEDATA_ready;

struct  CONFIG  config={0xAA55, //	uint16_t vaildsign;
	1,//uint8_t baundrate;    
	1,//uint8_t addr; 
	0x0000000000000008,
	//0x000000000000001,//0x6275110032120001,//0x6275110032120003,//0x5955125011120002, 03 yec-test 101
	0, //uint8_t parity;		// =0 : n,8,1   =1: o,8,1  =2: e,8,1	 ���ݸ�ʽ
	{1,1,1,1,1,1,1,1,1,1,1,1},
	0, //uint8_t DisplayMode;  // ��ʾģʽ��=0���̶���=1 ѭ��
	{TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,1,1,1}, //uint8_t interface_type[12]; // ��������
	{UNIT_M_S2,UNIT_TEMP,UNIT_M_S2,UNIT_M_S2,UNIT_M_S2,UNIT_M_S2,1,1,1,1,1,1},//uint8_t unit[12];  // ��λ
	{10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000},//uint32_t scale[12]; // ת��ϵ��
	{0.0f,0.0f,0.0f,0.0f,0.0f,0,0,0,8192,8192,8192,8192},//uint32_t adjust[12]; // ����ϵ��
//	{0,1,2,~0,~0,~0,~0,~0,3,4,5,6},//uint16_t interface_addr[12]; // modbus ��ַ �ϴ�
	{100,100,100,100,100,100,100,100,100,100,100,100},//	float alarmgate[12]; // float����ֵ
	{1.004f,1.004f,1.004f,1,1,1,1.004f,1.004f,1.004f,1,1,1},
	0,//uint8_t means	;// ��ֵ����
	1,//uint16_t means_times; // ��ֵ��������
	20000,//uint16_t freq;  // ����Ƶ�� Hz
	4096,//uint16_t avr_count;
	2, // uint8_t reflash; // ˢ��ʱ�� 
	~0, //	uint16_t din_addr;  //  ����������Ĵ�����ַ
	~0, // uint16_t dout_addr; //  ����������Ĵ�����ַ
	300, 30, // uint32_t force_gate,force_backlash,   // Ӧ��������ֵ�� �ز
	~0,~0, //	uint16_t max_addr0,max_addr1; ���ֵ��ַ
	300,
	300,
	300,
	0x4a,                          //PGA
	1,                          //��������
	0x0100,                         //�Ŵ���
	0x11,                      //  ���书������
	0x01,                    //AD����ʱ�䣬����������һ������
	25600,                     //AD����Ƶ��
	0x58B6A4C3,           //8��00�֣�aabb�ĸ�ʽ��aa����ʱ�䣬bb������ӣ���Ĭ��Ϊ0
	0,                //flash��ʼ��ַ
	PARAMETERMODE,
	0xFF,     //ѡ�ļ���ͨ������
	0, //DHCP
	"Tenda_5D9330",//"TP-LINK-SCZZB",//"yec-test",//"wifi-sensor",//"TP-LINK-sczzb2",//"hold-704",//"wifi-test1",//"yec-test",//"wifi-test",//"yec-test",//"zl_sensor",/////"yec-test",//"test3",//"qiangang2", //"qiangang1", //"qiangang1", /////
  "",//"china-yec",//"",//"wifi-sensor",//"18051061462",//"wifi-test",//"zl_sensor",///"china-yec",//"",////"",//"zl_sensor",/"lft13852578307",//"",//"",//"123456789",//"china-yec.com",// //
  "192.168.0.94",// "192.168.0.112",//�������˵�IP��ַ  "192.168.0.18", //M
  "8712", //�˿ں�
  {0xc0,0xa8,0x64,0x79},//  "192.168.100.30",  //LocalIP
  {0xc0,0xa8,0x64,0x01},//"192.168.100.1",  //LocalGATEWAY
	{0xff,0xff,0xff,0x00},//  "255.255.255.0",	//LocalMASK
	1,
	1,
	1,
	0xff,
	1,
	1,  //�Ƿ���������������
	0,
	0,
	{25600,25600,25600,25600,25600,25600,25600,25600,25600,8192,12800,12800},
	5, //�����������������5sһ��
	"www.av.com",
	"192.168.120.120",
	1,
};

struct PARAMETER Parameter;
/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/

extern void bsp_LedToggle(uint8_t _no);
extern void Down_statu2(void);
uint8_t random_time=0;
int main(void)
{
//	SCB->VTOR = 0x08020000;//FLASH_BASE|0x60000;//����ƫ����
	uint32_t i =0;


	random_time = rand();
	
	

	bsp_Init();		/* Ӳ����ʼ�� */
	
		HAL_Delay(random_time*5);
	Down_statu2();
	initEEPROM();
	Parameter.AutoPeriodTransmissonCounter = 0;
	Parameter.PeroidWaveTransmissionCounter=0;
	Parameter.DeviceKey=0;
	joinnetstatue.JoinCount=0;
	joinnetstatue.CurrentLink=0;
	Parameter.LINK_SELECT = LINK_SELECT_1;
	Parameter.LINK_STATUS = LINK_STATUS_FAILURE;
	Parameter.status=0;
	TxdBufTailIndex=0;
	TxdBufHeadIndex=0; 

	
	WRITE_ready = xSemaphoreCreateBinary();
	SPI_sem_req = xSemaphoreCreateBinary();
	sem_spi_idle = xSemaphoreCreateBinary();
	AD7606_ready = xSemaphoreCreateBinary();
	SAMPLEDATA_ready= xSemaphoreCreateBinary();
	xSemaphoreGive(WRITE_ready);
	xSemaphoreGive(sem_spi_idle);
	bsp_LedToggle(1);
//����������ע��ʹ��
//	xTaskCreate( vTaskRegister, "v1TaskLED", 512, NULL, 6, &xHandleTaskRegister );


//	xTaskCreate( vTaskLED, "vTaskLED", 1024, NULL, 9, &xHandleTaskLED );
//	if(config.Connection_Method == 0)//wifi����
//	{
		xTaskCreate( Esp32ProcessFunction,"Esp32ProcessFunction"     ,2048, NULL, 12, &xHandleEsp32ProcessFunction );
//	}else if(config.Connection_Method == 1)//��������
//	{
		xTaskCreate( vTaskLwip,"vTaskLwip"     ,1024, NULL, 13, &xHandleTaskLwip );
		xTaskCreate( vTaskTSendBuf,"vTaskSendbuf"     ,1024, NULL, 15, &xHandleTaskSendbuf );
//	}
//	xTaskCreate( vTaskTlvloop,"vTaskTLV_LOOP"     ,1024, NULL, 14, &xHandleTaskTlvloop );

	xTaskCreate( vTaskTDataProcess,"vTaskdataprocess"     ,1024, NULL, 17, &xHandleDataProcess );
//	xTaskCreate( vTaskDataEmu,"vTaskdataEMU"     ,2048, NULL, 7, &xHandleDataEmu );
	/* �������ȣ���ʼִ������ */
	vTaskStartScheduler();
}




extern void send_buffer_task( void );
static void vTaskTSendBuf(void *pvParameters)
{

	send_buffer_task();
}

extern void AD7606_TASK(void);
static void vTaskTDataProcess(void *pvParameters)
{
	AD7606_TASK();
}

extern void receive_server_data_task(void);
static void vTaskTlvloop(void *pvParameters)
{
	receive_server_data_task();
}

extern void DATA_EMU_TASK ( void );
static void vTaskDataEmu(void *pvParameters)
{
	DATA_EMU_TASK();
}
	


/*
*********************************************************************************************************
*	�� �� ��: vTaskLwip
*	����˵��: ��ʼ�� ETH,MAC,DMA,GPIO,LWIP,�������߳����ڴ�����̫����Ϣ
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 2  
*********************************************************************************************************
*/
extern void TCPIP_Init(void);
extern void client_init(void);

extern void tcp_client_conn_server_task(void);


/*
*********************************************************************************************************
*	�� �� ��: vTaskLED
*	����˵��: KED��˸	
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 2  
*********************************************************************************************************
*/

uint16_t send_active_beacon(void)
{ 
	uint8_t SENDBUF[20];
	 {
		SENDBUF[0]=0x7e;
		SENDBUF[1]=COMMAND_RECEIVE_ACTIVE_BEACON;
		SENDBUF[2]=0x00;
		SENDBUF[3]=0x00;
		SENDBUF[4]=COMMAND_RECEIVE_ACTIVE_BEACON;
		SENDBUF[5]=0x7e;
    WriteDataToTXDBUF(SENDBUF,6);
	 }
	 return 1;
}

void software_reset(void)
{

	__disable_irq();

	HAL_NVIC_SystemReset();
	                                                   /* wait until reset */
}
uint8_t REGISTER_FLAG = 0x00;
uint8_t spi_data_come[SLAVE_MACHINE_NUM];
extern SPI_HandleTypeDef SpiHandle;
uint8_t current_rx_machine = 0;
extern volatile uint8_t aTxBuffer[BUFFERSIZE] __attribute__((at(0x30002000))); //����SRAM1
extern volatile uint8_t aRxBuffer[BUFFERSIZE] __attribute__((at(0x30000000))); //����SRAM1
extern volatile bool flag_board_all_read;
static void vTaskLED(void *pvParameters)
{
	uint32_t i = 0;
	
		while(!(xSemaphoreTake(SPI_sem_req, portMAX_DELAY) == pdTRUE));
		for(i = 0; i < SLAVE_MACHINE_NUM; i++)
		{
			if(spi_data_come[i]){
				__disable_irq();
				spi_data_come[i] = 0;
				__enable_irq();
				while(!(xSemaphoreTake(sem_spi_idle, portMAX_DELAY) == pdTRUE));
				if(flag_board_all_read==true)
				{
					flag_board_all_read=false;
					xSemaphoreGive(AD7606_ready);
				}
				switch(i){
					case 0:
						BOARD1_CS_0();
						break;
					case 1:
						BOARD2_CS_0();
						break;
				}
				current_rx_machine = i;
				//delay_us(3);
				SCB_InvalidateDCache_by_Addr ((uint32_t *)aRxBuffer, 2*BUFFERSIZE);
				HAL_SPI_TransmitReceive_DMA(&SpiHandle, (uint8_t*)aTxBuffer, (uint8_t *)aRxBuffer, 2*BUFFERSIZE);
			
			}
		}
		
//	uint32_t ulNotifiedValue     = 0;
//	uint32_t ledToggleIntervalMs = 1000;
//	uint32_t time_counter_beacon=0;
//	for(;;)
//	{
//		vTaskDelay( 1000 );
////		IWDG_Feed();    			//ι��
//		bsp_LedToggle(1);
//			time_counter_beacon++;
//			if(time_counter_beacon>120)  //180����һ��������
//			{
//				
//				if(isReceiveBeaconMessage())
//				{
//					DeleteBeaconMessage(); 
//				}else
//				{
//					software_reset();
//				}
//				
//				if((config.Enable_active_beacon==1)&&(isCollectServer()))  //����������������������ͣ����ж���ǰ�Ƿ��յ�����������
//				{
//					if(isReceiveActiveBeaconMessage())
//					{
//					 DeleteActiveBeaconMessage(); 
//					}else
//					{
//					 software_reset();
//					}			
//				}
//			 time_counter_beacon=0; 				
//				
//			}
//			if((config.Enable_active_beacon==1)&&(isCollectServer()))  //�����������������
//			{
//				if(time_counter_beacon%config.BeaconInterval==0) //5s������һ��
//				{	send_active_beacon();
////					IWDG_Feed();    			//ι��
//				}
//			}
//		
//	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

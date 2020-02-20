#include "bsp_Esp32_UART.h"
#include <stdio.h> 
#include <FreeRTOS.h> 
#include <fsl_lpuart.h> 
#include <task.h>
#include <string.h>  
#include <stdbool.h>
#include <stdarg.h>
#include <app.h>
#include <fsl_lptmr.h>

#define DMA_LPUART LPUART1
#define DEMO_LPUART_CLKSRC kCLOCK_Osc0ErClk
#define DEMO_LPUART_CLK_FREQ CLOCK_GetFreq(kCLOCK_Osc0ErClk)
#define LPUART_TX_DMA_CHANNEL 1U
#define LPUART_RX_DMA_CHANNEL 2U
#define EXAMPLE_LPUART_DMAMUX_BASEADDR DMAMUX0
#define LPUART_DMA_BASEADDR DMA0
#define LPUART_TX_DMA_REQUEST kDmaRequestMux0LPUART1Tx
#define LPUART_RX_DMA_REQUEST kDmaRequestMux0LPUART1Rx
#define ECHO_BUFFER_LENGTH 8


uint8_t 	 	 TxdBuf[TX_BUFFER_SIZE];
uint8_t        RxdBuf[RX_BUFFER_SIZE];
uint8_t    RxdTelBuf[RX_BUFFER_SIZE];
uint8_t    ErrorBuf[RX_BUFFER_SIZE];
uint16_t ErrorBufBytes=0;
volatile uint16_t RxdBufHeadIndex=0;
volatile uint16_t RxdBufTailIndex=0;

uint16_t TxdBytes=0;
volatile uint16_t RxdBytes=0;
uint16_t TxdTelBytes=0;
volatile uint16_t RxdTelBytes=0;
volatile uint8_t ReceivedTel=0;

uint8_t    ATcmd[RX_BUFFER_SIZE];
uint16_t ATcmdLength;
struct  STRUCT_USARTx_Fram strEsp32_Fram_Record = { 0 };

lpuart_edma_handle_t wifi_lpuartEdmaHandle;
edma_handle_t wifi_lpuartTxEdmaHandle;
lpuart_transfer_t xfer;
lpuart_transfer_t sendXfer;
lpuart_transfer_t receiveXfer;
/**
  * @brief  Esp32��ʼ������
  * @param  ��
  * @retval ��
  */
void Esp32_Init ( void )
{ 
	macEsp32_RST_LOW_LEVEL();
	DelayMS(100);
	macEsp32_RST_HIGH_LEVEL();
 // DelayMS(350);
//	macEsp32_CH_DISABLE();	
	
}

uint8_t hasByteToTxd()		 // =1 ?????
{
	if(TailIndex==HeadIndex) {
		TxdTelBytes=0;
  	TxdBytes=0;
	  return(0);
	}
	{
		TxdTelBytes=TXDBUFLength[TailIndex];
	  return 1;
	}
			
}
uint8_t  getTxdByte()   //
{ 
	uint8_t re;
	#ifdef USE_LPUART1_TransmissionCompleteInterrupt	
	if(TxdBytes <TxdTelBytes)
	{
		re=TXDBUF[TailIndex][TxdBytes];
		TxdBytes++;
		if(TxdBytes ==TxdTelBytes)
		{ TxdBytes=0;
			Increase(TailIndex);  //increase
		}
		
	}
	return(re);
	#endif
	
	#ifdef USE_LPUART1_DMA_TransmissionInterrupt
	re=TXDBUF[TailIndex][0];
	return(re);
	#endif	
}

/*
 * ��������itoa
 * ����  ������������ת�����ַ���
 * ����  ��-radix =10 ��ʾ10���ƣ��������Ϊ0
 *         -value Ҫת����������
 *         -buf ת������ַ���
 *         -radix = 10
 * ���  ����
 * ����  ����
 * ����  ����USART2_printf()����
 */
static char * itoa( int value, char *string, int radix )
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



void Enable485TXD(void)
{ 
	#ifdef USE_LPUART1_TransmissionCompleteInterrupt
	LPUART_EnableInterrupts(LPUART1,kLPUART_TransmissionCompleteInterruptEnable);
  #endif
	#ifdef USE_LPUART1_DMA_TransmissionInterrupt

	DMA0->SERQ = DMA_SERQ_SERQ(1);

	#endif

}

void Disable485TXD(void)
{ 
	#ifdef USE_LPUART1_TransmissionCompleteInterrupt

  LPUART_DisableInterrupts(LPUART1,kLPUART_TransmissionCompleteInterruptEnable);
	
	#endif
	
	#ifdef USE_LPUART1_DMA_TransmissionInterrupt
  

	DMA0->CERQ =DMA_CERQ_CERQ(1);
	
	#endif
}
void startTxd(LPUART_Type * USARTx)
{ 
	Enable485TXD(); 
}

void USART_DATA_Esp32( LPUART_Type * USARTx, uint8_t * Data, uint16_t length )
{ 
	for(uint32_t i=0;i<length;i++)
	TxdBuf[i]=* Data++;
	
	TxdBytes=0;
	TxdTelBytes=length;
	
	startTxd(USARTx);			   // ?????? ,????
	
}


 uint8_t WriteDataToTXDBUF(uint8_t * source,uint16_t length)
{ 
	
	if(xSemaphoreTake(WRITE_ready, portMAX_DELAY) != pdTRUE);//  ���ź�����ûë��
	__disable_irq();	   //��Ӧ�����������жϣ�������
	if(isTxdBufFull()) 
	{
	xSemaphoreGive(WRITE_ready);//�ͷ��ź������
	__enable_irq();

	return 0;
	} //�������˾Ͳ���
	__enable_irq();
#ifdef Auto_CuttingPackage
	TXDBUFLength[HeadIndex]=length+4;
#else
	TXDBUFLength[HeadIndex]=length;
#endif
	for(uint16_t i=0;i<length;i++)
	TXDBUF[HeadIndex][i]=source[i];
#ifdef Auto_CuttingPackage
  TXDBUF[HeadIndex][length]=0xff;
  TXDBUF[HeadIndex][length+1]=0xff;
	TXDBUF[HeadIndex][length+2]=0xff;
	TXDBUF[HeadIndex][length+3]=0xff;
#endif

#ifdef USE_LPUART1_TransmissionCompleteInterrupt
  Increase(HeadIndex);
	Enable485TXD();
	#endif
	
#ifdef USE_LPUART1_DMA_TransmissionInterrupt
	  __disable_irq();	   //��Ӧ�����������жϣ�������
	 if(HeadIndex==TailIndex)
	 { 
    //__enable_irq();
	  DMA0->TCD[1].SADDR = (uint32_t)&TXDBUF[TailIndex][0];
		DMA0->TCD[1].CITER_ELINKNO = TXDBUFLength[TailIndex];
		DMA0->TCD[1].BITER_ELINKNO = TXDBUFLength[TailIndex];	
		DMA0->TCD[1].SLAST =0-TXDBUFLength[TailIndex];//��䱾���Ͽ��Բ�Ҫ���ҷ�������Դ��ַ�� 
		//**** Destination address reload after major loop finish, no address reload needed
		DMA0->TCD[1].DLAST_SGA = 0x00;

		//__disable_irq();	   //��Ӧ�����������жϣ�������
	  Increase(HeadIndex);
	  __enable_irq();
		DMA0->SERQ = DMA_SERQ_SERQ(1);
		
	 }else
	 {
   // __disable_irq();	   //��Ӧ�����������жϣ�������
	  Increase(HeadIndex);
	  __enable_irq();
	 }
	#endif
	
	
	xSemaphoreGive(WRITE_ready);//�ͷ��ź������
	return 1;
}

void USART_CMD_Esp32( LPUART_Type * USARTx, char * Data, ... )
{ const char *s;
	int d;   
  char buf[126];  //�����˷����ֽڵĳ��ȣ�66��
	va_list ap;
	va_start(ap, Data);
	uint16_t 	j=0;
	while ( * Data != 0 )     // �ж��Ƿ񵽴��ַ���������
	{				                          

			if ( * Data == '%')
		{									  //
			switch ( *++Data )
			{				
				case 's':										  //�ַ���
				s = va_arg(ap, const char *);
				
				for ( ; *s; s++) 
				{
					ATcmd[j++]=*s;	
					
				}
				
				Data++;
				break;

				case 'd':			
					//ʮ����
				d = va_arg(ap, int);
				
				itoa(d, buf, 10);
				
				for (s = buf; *s; s++) 
				{
					ATcmd[j++]=*s;
				}
				
				Data++;
				break;
				
				default:
				Data++;
				j++;
				break;
				
			}		 
		}
		
		else {ATcmd[j++]=*Data++;
		}
		
			
		
	}
	
	//TxdBytes=0;
	ATcmdLength=j;
	WriteDataToTXDBUF(ATcmd,ATcmdLength);
//	startTxd(USARTx);			   // ?????? ,????

}


/**
  * @brief  ��ʼ��Esp32�õ���GPIO����
  * @param  ��
  * @retval ��
  */


/**
  * @brief  ���� Esp32 USART �� NVIC �ж�
  * @param  ��
  * @retval ��
  */
/* LPUART user callback */
volatile uint16_t ucTcpClosedFlag;
volatile uint16_t esp32_ready=0;
volatile uint16_t WifiConnectedFlag;
volatile uint16_t WifiDisconnectedFlag;
void LPUART1_Init(uint32_t Baudrate)
{
    lpuart_config_t config;
    TxdBytes=0;
	  CLOCK_SetLpuartClock(2U);

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kLPUART_ParityDisabled;
     * config.stopBitCount = kLPUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = Baudrate;
    config.enableTx = true;
    config.enableRx = true;

    LPUART_Init(LPUART1, &config, CLOCK_GetFreq(kCLOCK_Osc0ErClk));
    LPUART_EnableInterrupts(LPUART1, kLPUART_RxDataRegFullInterruptEnable);
	  LPUART_EnableInterrupts(LPUART1, kLPUART_IdleLineInterruptEnable);
		LPUART_EnableInterrupts(LPUART1,kLPUART_TransmissionCompleteInterruptEnable);
		LPUART_EnableInterrupts(LPUART1,kLPUART_RxOverrunInterruptEnable);
    //NVIC_SetPriorityGrouping(1);
		NVIC_SetPriority(LPUART1_IRQn,2);
		EnableIRQ(LPUART1_IRQn);
}

void LPUART1_DMA_Init(uint32_t Baudrate){
	lpuart_config_t lpuartConfig;
	TxdBytes=0;
	

  CLOCK_SetLpuartClock(2U);

    /* Initialize the LPUART. */
    /*
     * lpuartConfig.baudRate_Bps = 115200U;
     * lpuartConfig.parityMode = kLPUART_ParityDisabled;
     * lpuartConfig.stopBitCount = kLPUART_OneStopBit;
     * lpuartConfig.txFifoWatermark = 0;
     * lpuartConfig.rxFifoWatermark = 0;
     * lpuartConfig.enableTx = false;
     * lpuartConfig.enableRx = false;
     */
    LPUART_GetDefaultConfig(&lpuartConfig);
    lpuartConfig.baudRate_Bps = Baudrate;
    lpuartConfig.enableTx = true;
    lpuartConfig.enableRx = true;
   #ifdef USE_UARTFLOW	 
    lpuartConfig.enableRxRTS = true;
    lpuartConfig.enableTxCTS = true;
    lpuartConfig.txCtsConfig = kLPUART_CtsSampleAtStart;
    lpuartConfig.txCtsSource = kLPUART_CtsSourcePin;
   #endif
    LPUART_Init(LPUART1, &lpuartConfig, DEMO_LPUART_CLK_FREQ);
		//LPUART1->CTRL &=~LPUART_CTRL_TCIE_MASK;
    LPUART_EnableInterrupts(LPUART1, kLPUART_RxDataRegFullInterruptEnable);
		LPUART_EnableInterrupts(LPUART1, kLPUART_IdleLineInterruptEnable);
		LPUART_EnableInterrupts(LPUART1,kLPUART_RxOverrunInterruptEnable);
		NVIC_SetPriorityGrouping(1);
		NVIC_SetPriority(LPUART1_IRQn,2);
		EnableIRQ(LPUART1_IRQn);
}



void LPUART_UserCallback1(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

   
}


void DMA_MUXChannel_LPUART1_Init(void){
   edma_config_t config;
	  uint8_t g_tipString[2];
	  xfer.data = g_tipString;
    xfer.dataSize = sizeof(g_tipString) - 1;
    DMAMUX_Init(DMAMUX0);
    /* Set channel for LPUART */
    DMAMUX_SetSource(DMAMUX0, LPUART_TX_DMA_CHANNEL, LPUART_TX_DMA_REQUEST);
   
    DMAMUX_EnableChannel(DMAMUX0, LPUART_TX_DMA_CHANNEL);

//    /* Init the EDMA module */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(LPUART_DMA_BASEADDR, &config);
    EDMA_CreateHandle(&wifi_lpuartTxEdmaHandle, LPUART_DMA_BASEADDR, LPUART_TX_DMA_CHANNEL);
    
    /* Create LPUART DMA handle. */
    LPUART_TransferCreateHandleEDMA(DMA_LPUART, &wifi_lpuartEdmaHandle, LPUART_UserCallback1, NULL, &wifi_lpuartTxEdmaHandle,NULL);
 
    LPUART_SendEDMA(LPUART1, &wifi_lpuartEdmaHandle, &xfer);


 
}

 void DMA1_DMA17_IRQHandler(void)
{  
	 Increase(TailIndex);
   EDMA_ClearChannelStatusFlags(DMA0, LPUART1_DMA_CHANNEL, kEDMA_InterruptFlag);//kEDMA_InterruptFlag
   if(hasByteToTxd())		{
			DMA0->TCD[1].SADDR = (uint32_t)&TXDBUF[TailIndex][0];
      DMA0->TCD[1].CITER_ELINKNO = TXDBUFLength[TailIndex];
      DMA0->TCD[1].BITER_ELINKNO = TXDBUFLength[TailIndex];	
		  DMA0->TCD[1].SLAST =0-TXDBUFLength[TailIndex];//��䱾���Ͽ��Բ�Ҫ���ҷ�������Դ��ַ�� 
      //**** Destination address reload after major loop finish, no address reload needed
      DMA0->TCD[1].DLAST_SGA = 0x00;	
		  DMA0->SERQ = DMA_SERQ_SERQ(1);
    	 
		}else
	 {
		 Disable485TXD();
	 }
 } 
 
volatile uint32_t ssssss=0;
void LPUART1_IRQHandler()
{  
   uint8_t ucCh;
	uint32_t esp32_readyyyyyyyyyyy=0;
	uint32_t WifiConnectedFlagg=0;
	uint32_t WifiDisconnectedFlagg=0;
	if((LPUART_GetStatusFlags ( LPUART1 ) & LPUART_STAT_TC_MASK)&&(LPUART1->CTRL & LPUART_CTRL_TCIE_MASK)) 	{	  // ??????		
#ifdef USE_LPUART1_TransmissionCompleteInterrupt
	if(hasByteToTxd()){
		LPUART_WriteByte(LPUART1, getTxdByte());
		}else{ 
			Disable485TXD();
		}
#endif
	
#ifdef USE_LPUART1_DMA_TransmissionInterrupt		
#endif
	LPUART_ClearStatusFlags(LPUART1,LPUART_STAT_TC_MASK);
	}else if (( LPUART_GetStatusFlags ( LPUART1 ) & LPUART_STAT_RDRF_MASK )&&(LPUART1->CTRL & LPUART_CTRL_RIE_MASK))
	{
		ucCh  = LPUART_ReadByte( LPUART1 );
		if(Parameter.Esp32TransmissionMode==BrainTransmission){
			strEsp32_Fram_Record .Data_RX_BUF [ strEsp32_Fram_Record .InfBit .FramLength ++ ]  = ucCh;
	  }else if(Parameter.Esp32TransmissionMode==NoBrainTransmission)
		{
			strEsp32_Fram_Record .Data_RX_BUF [ strEsp32_Fram_Record .InfBit .FramLength ++ ]  = ucCh;
			if(isRxdBufFull()) 
				return;
			RxdBuf[RxdBufHeadIndex]=ucCh;
			IncreaseRxdBufNum(RxdBufHeadIndex);
		}
		LPUART_ClearStatusFlags(LPUART1,LPUART_STAT_RDRF_MASK);  //����Ǹ�DMA�ӵ�
	}	else if ( ( LPUART_GetStatusFlags ( LPUART1 ) & LPUART_STAT_IDLE_MASK)&&(LPUART1->CTRL & LPUART_CTRL_ILIE_MASK))                                         //����֡�������
	{ 
		ucCh  = LPUART_ReadByte( LPUART1 ); //�ɶ��ɲ�����  
		strEsp32_Fram_Record .FramHeadIndex = 0;
		strEsp32_Fram_Record .InfBit .FramFinishFlag = 1;	
	  strEsp32_Fram_Record .InfBit .FramLength  =0;                                                     //�������������жϱ�־λ(�ȶ�USART_SR��Ȼ���USART_DR)
    ucTcpClosedFlag = strstr ( strEsp32_Fram_Record .Data_RX_BUF, "CLOSED\r\n" ) ? 1 : 0; //�����ж�
		
		WifiConnectedFlagg= strstr ( strEsp32_Fram_Record .Data_RX_BUF, "connectserver" ) ? 1 : 0; //�Ѿ�������
		WifiDisconnectedFlagg= strstr ( strEsp32_Fram_Record .Data_RX_BUF, "lostserver" ) ? 1 : 0; //�Ѿ���ʧ
		esp32_readyyyyyyyyyyy= strstr ( strEsp32_Fram_Record .Data_RX_BUF, "ready" ) ? 1 : 0; //�Ѿ���ʧ
		if(esp32_readyyyyyyyyyyy)
		{
			esp32_ready=1;
			esp32_readyyyyyyyyyyy=0;
		}else	if(WifiConnectedFlagg)
		{
			WifiConnectedFlag=1;
			WifiConnectedFlagg=0;
		}else if(WifiDisconnectedFlagg)
		{
			WifiDisconnectedFlag=1;
			WifiDisconnectedFlagg=0;
		}
		
		LPUART_ClearStatusFlags(LPUART1,LPUART_STAT_IDLE_MASK);
  }	
	else if ( ( LPUART_GetStatusFlags ( LPUART1 ) & LPUART_STAT_OR_MASK)&&(LPUART1->CTRL & LPUART_CTRL_ORIE_MASK))                                         //����֡�������
	 LPUART_ClearStatusFlags(LPUART1,LPUART_STAT_OR_MASK);

    
}



/**
  * @brief  ��ʼ��Esp32�õ��� USART
  * @param  ��
  * @retval ��
  */

void MK27_USART_Config ( uint32_t Baudrate )
{ 

	#ifdef USE_LPUART1_TransmissionCompleteInterrupt
	LPUART1_Init(Baudrate);
	#endif
	
	#ifdef USE_LPUART1_DMA_TransmissionInterrupt
	
	
	LPUART1_DMA_Init(Baudrate);
	DMA_MUXChannel_LPUART1_Init();
	

	#endif

}



/*
 * ��������Esp32_Rst
 * ����  ������WF-Esp32ģ��
 * ����  ����
 * ����  : ��
 * ����  ���� Esp32_AT_Test ����
 */
//const portTickType xDelay = pdMS_TO_TICKS(500);  
void Esp32_Rst ( void )
{
	#if 0
	 Esp32_Cmd ( "AT+RST", "OK", "ready", 2500 );   	
	
	#else
	 macEsp32_RST_LOW_LEVEL();
	 vTaskDelay (pdMS_TO_TICKS(500)); 
	 macEsp32_RST_HIGH_LEVEL();
	#endif

}


/*
 * ��������Esp32_Cmd
 * ����  ����WF-Esp32ģ�鷢��ATָ��
 * ����  ��cmd�������͵�ָ��
 *         reply1��reply2���ڴ�����Ӧ��ΪNULL������Ӧ������Ϊ���߼���ϵ
 *         waittime���ȴ���Ӧ��ʱ��
 * ����  : 1��ָ��ͳɹ�
 *         0��ָ���ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_Cmd ( char * cmd, char * reply1, char * reply2, uint32_t waittime )
{    
	strEsp32_Fram_Record .InfBit .FramLength = 0;               //���¿�ʼ�����µ����ݰ�
  strEsp32_Fram_Record .FramHeadIndex=0;
	macEsp32_Usart ( "%s\r\n", cmd );

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //����Ҫ��������
		return true;
	
	vTaskDelay ( pdMS_TO_TICKS(waittime) );                 //��ʱ
	
	strEsp32_Fram_Record .Data_RX_BUF [ strEsp32_Fram_Record .InfBit .FramLength ]  = '\0';

	//macPC_Usart ( "%s", strEsp32_Fram_Record .Data_RX_BUF );
  Parameter.ch1_int_max=( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply1 );
	if ( ( reply1 != 0 ) && ( reply2 != 0 ) )
		
		return ( ( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply1 ) || 
						 ( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply2 ) ); 
 	
	else if ( reply1 != 0 )
		return ( ( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply1 ) );
	
	else
		return ( ( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply2 ) );
	
}

bool Esp32_wait_response ( char * reply1, char * reply2, uint32_t waittime )
{    
	strEsp32_Fram_Record .InfBit .FramLength = 0;               //���¿�ʼ�����µ����ݰ�
  strEsp32_Fram_Record .FramHeadIndex=0;
	

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //����Ҫ��������
		return true;
	
	vTaskDelay ( pdMS_TO_TICKS(waittime) );                 //��ʱ
	
	strEsp32_Fram_Record .Data_RX_BUF [ strEsp32_Fram_Record .InfBit .FramLength ]  = '\0';

	//macPC_Usart ( "%s", strEsp32_Fram_Record .Data_RX_BUF );
  Parameter.ch1_int_max=( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply1 );
	if ( ( reply1 != 0 ) && ( reply2 != 0 ) )
		
		return ( ( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply1 ) || 
						 ( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply2 ) ); 
 	
	else if ( reply1 != 0 )
		return ( ( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply1 ) );
	
	else
		return ( ( bool ) strstr ( strEsp32_Fram_Record .Data_RX_BUF, reply2 ) );
	
}


/*
 * ��������Esp32_AT_Test
 * ����  ����WF-Esp32ģ�����AT��������
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
//void Esp32_AT_Test ( void )
//{
//	macEsp32_RST_HIGH_LEVEL();
//	
//	vTaskDelay ( 1000 ); 
//	
//	while ( ! Esp32_Cmd ( "AT", "OK", NULL, 500 ) ) Esp32_Rst ();  	

//}
void Esp32_AT_Test ( void )
{
	char count=0;
	
	macEsp32_RST_HIGH_LEVEL();	
	vTaskDelay ( pdMS_TO_TICKS(1000) );
	while ( count < 10 )
	{
		if( Esp32_Cmd ( "AT", "OK", NULL, 500 ) ) return;
		Esp32_Rst();
		++ count;
	}
}
/*
 * ��������Esp32_Net_Mode_Choose
 * ����  ��ѡ��WF-Esp32ģ��Ĺ���ģʽ
 * ����  ��enumMode������ģʽ
 * ����  : 1��ѡ��ɹ�
 *         0��ѡ��ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_SETUARTBaudrate ( uint32_t Baudrate )
{
	char cCmd [120];
  #ifdef USE_UARTFLOW
	sprintf ( cCmd, "AT+UART_CUR=%d,8,1,0,3",  Baudrate );  //ʹ������
	#else
	sprintf ( cCmd, "AT+UART_CUR=%d,8,1,0,0",  Baudrate );  //ʹ������
	#endif
	return Esp32_Cmd ( cCmd, "OK", NULL, 500 );
	
}
/*
 * ��������Esp32_Net_Mode_Choose
 * ����  ��ѡ��WF-Esp32ģ��Ĺ���ģʽ
 * ����  ��enumMode������ģʽ
 * ����  : 1��ѡ��ɹ�
 *         0��ѡ��ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_Net_Mode_Choose ( ENUM_Net_ModeTypeDef enumMode )
{
	switch ( enumMode )
	{
		case STA:
			return Esp32_Cmd ( "AT+CWMODE=1", "OK", "no change", 500 ); 
		
	  case AP:
		  return Esp32_Cmd ( "AT+CWMODE=2", "OK", "no change", 2500 ); 
		
		case STA_AP:
		  return Esp32_Cmd ( "AT+CWMODE=3", "OK", "no change", 2500 ); 
		
	  default:
		  return false;
  }
	
}


/*
 * ��������Esp32_JoinAP
 * ����  ��WF-Esp32ģ�������ⲿWiFi
 * ����  ��pSSID��WiFi�����ַ���
 *       ��pPassWord��WiFi�����ַ���
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_JoinAP ( char * pSSID, char * pPassWord )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
	
	return Esp32_Cmd ( cCmd, "OK","WIFI CONNECTED", 4500 ); //WIFI CONNECTED
	
} 

bool Esp32_SetIP ( char * IP, char * MASK,char * GATEWAY )
	{
	
	char cCmd [120]="";;

	sprintf ( cCmd, "AT+CIPSTA=\"%s\",\"%s\",\"%s\"",IP,MASK,GATEWAY);
	
  return Esp32_Cmd ( cCmd, "OK", NULL, 500 );
	
}
bool Esp32_SetIP_1(char *localIP,char *LocalMASK,char *LocalGATEWAY,uint8_t dhcp)  //����IP��ַ
{
	 uint8_t localIPlength=strlen(localIP);
	 uint8_t LocalMASKlength=strlen(LocalMASK);
	 uint8_t LocalGATEWAYlength=strlen(LocalGATEWAY);
	 CmdBufLength=15+localIPlength+LocalMASKlength+LocalGATEWAYlength;
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=0xe4;   //T
	 CmdBuf[2]=CmdBufLength-6;
	 CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	 CmdBuf[4]=0X00;   //V_IP
	 CmdBuf[5]=localIPlength;
	{
		for(uint8_t i=0;i<localIPlength;i++)
		CmdBuf[6+i]=localIP[i];
	 
	 }
   CmdBuf[6+localIPlength]=0X01;   //V_MASK
	 CmdBuf[7+localIPlength]=LocalMASKlength;
	{
		for(uint8_t i=0;i<LocalMASKlength;i++)
		CmdBuf[8+localIPlength+i]=LocalMASK[i];
	 
	 }
	 CmdBuf[8+localIPlength+LocalMASKlength]=0X02;   //V_GATEWAY
	 CmdBuf[9+localIPlength+LocalMASKlength]=LocalGATEWAYlength;
	{
		for(uint8_t i=0;i<LocalGATEWAYlength;i++)
		CmdBuf[10+localIPlength+LocalMASKlength+i]=LocalGATEWAY[i];
	 
	 }
	 CmdBuf[10+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X03;
	 CmdBuf[11+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X01;
	 CmdBuf[12+localIPlength+LocalMASKlength+LocalGATEWAYlength]=dhcp;
	 CmdBuf[13+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0;
	 for(uint8_t i=1;i<(13+localIPlength+LocalMASKlength+LocalGATEWAYlength);i++)
	 CmdBuf[13+localIPlength+LocalMASKlength+LocalGATEWAYlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	 CmdBuf[14+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0x7e;
	 CmdBufLength=15+localIPlength+LocalMASKlength+LocalGATEWAYlength;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 return Esp32_wait_response( "OK", NULL, 10);
}

bool Esp32_SetAP_1(char *APssid,char *APpassword )  //����IP��ַ
{  
	//���Ҫ�ж��Ƿ񹤳�����
	 uint8_t APSSIDlength=strlen(APssid);
	 uint8_t APPASSWORDlength=strlen(APpassword);
	 CmdBufLength=10+APSSIDlength+APPASSWORDlength;
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=0xe5;   //T
	 CmdBuf[2]=CmdBufLength-6;
	 CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	 CmdBuf[4]=0X00;   //V_IP
	 CmdBuf[5]=APSSIDlength;
	{
		for(uint8_t i=0;i<APSSIDlength;i++)
		CmdBuf[6+i]=APssid[i];
	 
	 }
   CmdBuf[6+APSSIDlength]=0X01;   //V_MASK
	 CmdBuf[7+APSSIDlength]=APPASSWORDlength;
	{
		for(uint8_t i=0;i<APPASSWORDlength;i++)
		CmdBuf[8+APSSIDlength+i]=APpassword[i];
	 
	 }
	 CmdBuf[8+APSSIDlength+APPASSWORDlength]=0X00;   //У���
	 
	 for(uint8_t i=1;i<(8+APSSIDlength+APPASSWORDlength);i++)
	 CmdBuf[8+APSSIDlength+APPASSWORDlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	 CmdBuf[9+APSSIDlength+APPASSWORDlength]=0x7e;
	 CmdBufLength=10+APSSIDlength+APPASSWORDlength;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 return Esp32_wait_response( "OK", NULL, 10);
}

bool Esp32_applynetset()  //����IP��ַ
{
	CmdBuf[0]=0x7e;
	CmdBuf[1]=0xff;
	CmdBuf[2]=0x00;
	CmdBuf[3]=0x00;
	CmdBuf[4]=0xff;
	CmdBuf[5]=0x7e;
  CmdBufLength=6;
	WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	return Esp32_wait_response( "connectserver", NULL, 400);
}

bool Esp32_SetTCPSERVER_1(char *TcpServer_IP,char *TcpServer_Port)  //����IP��ַ
{
	 uint8_t TcpServer_IPlength=strlen(TcpServer_IP);
	 uint8_t TcpServer_Portlength=strlen(TcpServer_Port);
	 CmdBufLength=10+TcpServer_Portlength+TcpServer_IPlength;
	 CmdBuf[0]=0X7E;
	 CmdBuf[1]=0xe7;   //T
	 CmdBuf[2]=CmdBufLength-6;
	 CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	 CmdBuf[4]=0X00;   //V_IP
	 CmdBuf[5]=TcpServer_IPlength;
	{
		for(uint8_t i=0;i<TcpServer_IPlength;i++)
		CmdBuf[6+i]=TcpServer_IP[i];
	 
	 }
   CmdBuf[6+TcpServer_IPlength]=0X01;   //V_MASK
	 CmdBuf[7+TcpServer_IPlength]=TcpServer_Portlength;
	{
		for(uint8_t i=0;i<TcpServer_Portlength;i++)
		CmdBuf[8+TcpServer_IPlength+i]=TcpServer_Port[i];
	 
	 }
	 CmdBuf[8+TcpServer_IPlength+TcpServer_Portlength]=0X00;   //У���
	 
	 for(uint8_t i=1;i<(8+TcpServer_IPlength+TcpServer_Portlength);i++)
	 CmdBuf[8+TcpServer_IPlength+TcpServer_Portlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	 CmdBuf[9+TcpServer_IPlength+TcpServer_Portlength]=0x7e;
	 CmdBufLength=10+TcpServer_IPlength+TcpServer_Portlength;
	 WriteDataToTXDBUF(CmdBuf,CmdBufLength);
	 return Esp32_wait_response( "OK", NULL, 10);
}


/*
 * ��������Esp32_AUTOCONNAP
 * ����  ���ϵ��Ƿ��Զ����ӵ�AP

 * ����  : 1���Զ�����
 *         0��ȡ���Զ����� ���浽flash
 * ����  �����ⲿ����
 */
bool Esp32_AutoConn( FunctionalState enumEnUnvarnishTx )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWAUTOCONN=%d",( enumEnUnvarnishTx ? 1 : 0 ) );
	
	return Esp32_Cmd ( cCmd, "OK",NULL, 500 ); //WIFI AUTOCONNECTED
	
}
/*
 * ��������Esp32_BuildAP
 * ����  ��WF-Esp32ģ�鴴��WiFi�ȵ�
 * ����  ��pSSID��WiFi�����ַ���
 *       ��pPassWord��WiFi�����ַ���
 *       ��enunPsdMode��WiFi���ܷ�ʽ�����ַ���
 * ����  : 1�������ɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_BuildAP ( char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWSAP=\"%s\",\"%s\",1,%d", pSSID, pPassWord, enunPsdMode );
	
	return Esp32_Cmd ( cCmd, "OK", 0, 1000 );
	
}


/*
 * ��������Esp32_Enable_MultipleId
 * ����  ��WF-Esp32ģ������������
 * ����  ��enumEnUnvarnishTx�������Ƿ������
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_Enable_MultipleId ( FunctionalState enumEnUnvarnishTx )
{
	char cStr [20];
	
	sprintf ( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );
	
	return Esp32_Cmd ( cStr, "OK", 0, 500 );
	
}


/*
 * ��������Esp32_Enable_MultipleId
 * ����  ��WF-Esp32ģ������������
 * ����  ��enumEnUnvarnishTx�������Ƿ������
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_ReturnSENDCMD (bool disable)
{
	char cStr [20];
	
	sprintf ( cStr, "ATE%d", disable );
	
	return Esp32_Cmd ( cStr, "OK", 0, 500 );
	
}

bool Esp32_setDHCP (FunctionalState disable)
{
	char cStr [20];
	
	sprintf ( cStr, "AT+CWDHCP=%d,1", disable );  //dhcp station
	
	return Esp32_Cmd ( cStr, "OK", 0, 500 );
	
}


/*
 * ��������Esp32_Link_Server
 * ����  ��WF-Esp32ģ�������ⲿ������
 * ����  ��enumE������Э��
 *       ��ip��������IP�ַ���
 *       ��ComNum���������˿��ַ���
 *       ��id��ģ�����ӷ�������ID
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_Link_Server ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr [100] = { 0 }, cCmd [120];

  switch (  enumE )
  {
		case enumTCP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
		 
		  break;
		//  sprintf ( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
		case enumUDP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
		  break;
		
		default:
			break;
  }

  if ( id < 5 )
    sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);

  else
	  sprintf ( cCmd, "AT+CIPSTART=%s", cStr );  //2sһ��������

	return Esp32_Cmd ( cCmd, "OK", "ALREAY CONNECT", 500 );  //500MS��һ�� ����ȵ�ʱ��Ҫ���ƣ�С·����֮�󣬻��һ���ֱ��ѯ���豸�ţ�����Ϊ1s��Ϳ��Խ���͸��ģʽ
	
}


/*
 * ��������Esp32_StartOrShutServer
 * ����  ��WF-Esp32ģ�鿪����رշ�����ģʽ
 * ����  ��enumMode������/�ر�
 *       ��pPortNum���������˿ں��ַ���
 *       ��pTimeOver����������ʱʱ���ַ�������λ����
 * ����  : 1�������ɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_StartOrShutServer ( FunctionalState enumMode, char * pPortNum, char * pTimeOver )
{
	char cCmd1 [120], cCmd2 [120];

	if ( enumMode )
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum );
		
		sprintf ( cCmd2, "AT+CIPSTO=%s", pTimeOver );

		return ( Esp32_Cmd ( cCmd1, "OK", 0, 500 ) &&
						 Esp32_Cmd ( cCmd2, "OK", 0, 500 ) );
	}
	
	else
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 0, pPortNum );

		return Esp32_Cmd ( cCmd1, "OK", 0, 500 );
	}
	
}


/*
 * ��������Esp32_Get_LinkStatus
 * ����  ����ȡ WF-Esp32 ������״̬�����ʺϵ��˿�ʱʹ��
 * ����  ����
 * ����  : 2�����ip
 *         3����������
 *         3��ʧȥ����
 *         0����ȡ״̬ʧ��
 * ����  �����ⲿ����
 */
uint8_t Esp32_Get_LinkStatus ( void )
{
	if ( Esp32_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
	{
		if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "STATUS:2\r\n" ) )
			return 2;
		
		else if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "STATUS:3\r\n" ) )
			return 3;
		
		else if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "STATUS:4\r\n" ) )
			return 4;		

	}
	
	return 0;
	
}

/*
 * ��������Esp32_Get_Sation_IP
 * ����  ����ȡ WF-Esp32 �ı���IP
 * ����  ����
 * ����  : 2�����ip
 *         3����������
 *         3��ʧȥ����
 *         0����ȡ״̬ʧ��
 * ����  �����ⲿ����
 */
uint8_t Esp32_Get_Sation_IP ( void )
{ 
	uint8_t num;
	char *localIP;
	if ( Esp32_Cmd ( "AT+CIFSR", "OK", 0, 500 ) )
	{
		localIP= strstr ( strEsp32_Fram_Record .Data_RX_BUF, "STAIP,\""  );
			
	if ( localIP )
		localIP += 7;
	
	else
		return 0;
	
	for ( num = 0; num < 16; num ++ )
	{
		config.LocalIP[ num ] = * ( localIP + num);
		
		if ( config.LocalIP[ num ] == '\"' )
		{
			config.LocalIP[ num ] = '\0';
			break;
		}
		
	}
	}
	return 0;	
}

/*
 * ��������Esp32_Get_IdLinkStatus
 * ����  ����ȡ WF-Esp32 �Ķ˿ڣ�Id������״̬�����ʺ϶�˿�ʱʹ��
 * ����  ����
 * ����  : �˿ڣ�Id��������״̬����5λΪ��Чλ���ֱ��ӦId5~0��ĳλ����1���Id���������ӣ�������0���Idδ��������
 * ����  �����ⲿ����
 */
uint8_t Esp32_Get_IdLinkStatus ( void )
{
	uint8_t ucIdLinkStatus = 0x00;
	
	
	if ( Esp32_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
	{
		if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "+CIPSTATUS:0," ) )
			ucIdLinkStatus |= 0x01;
		else 
			ucIdLinkStatus &= ~ 0x01;
		
		if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "+CIPSTATUS:1," ) )
			ucIdLinkStatus |= 0x02;
		else 
			ucIdLinkStatus &= ~ 0x02;
		
		if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "+CIPSTATUS:2," ) )
			ucIdLinkStatus |= 0x04;
		else 
			ucIdLinkStatus &= ~ 0x04;
		
		if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "+CIPSTATUS:3," ) )
			ucIdLinkStatus |= 0x08;
		else 
			ucIdLinkStatus &= ~ 0x08;
		
		if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "+CIPSTATUS:4," ) )
			ucIdLinkStatus |= 0x10;
		else 
			ucIdLinkStatus &= ~ 0x10;	

	}
	
	return ucIdLinkStatus;
	
}


/*
 * ��������Esp32_Inquire_ApIp
 * ����  ����ȡ F-Esp32 �� AP IP
 * ����  ��pApIp����� AP IP ��������׵�ַ
 *         ucArrayLength����� AP IP ������ĳ���
 * ����  : 0����ȡʧ��
 *         1����ȡ�ɹ�
 * ����  �����ⲿ����
 */
uint8_t Esp32_Inquire_ApIp ( char * pApIp, uint8_t ucArrayLength )
{
	char uc;
	
	char * pCh;
	
	
  Esp32_Cmd ( "AT+CIFSR", "OK", 0, 500 );
	
	pCh = strstr ( strEsp32_Fram_Record .Data_RX_BUF, "APIP,\"" );
	
	if ( pCh )
		pCh += 6;
	
	else
		return 0;
	
	for ( uc = 0; uc < ucArrayLength; uc ++ )
	{
		pApIp [ uc ] = * ( pCh + uc);
		
		if ( pApIp [ uc ] == '\"' )
		{
			pApIp [ uc ] = '\0';
			break;
		}
		
	}
	
	return 1;
	
}

void Set_Esp32_UnvarnishSend (void)
{
	while ( ! Esp32_UnvarnishSend () );
	Parameter.Esp32TransmissionMode=NoBrainTransmission;
}
/*
 * ��������Esp32_UnvarnishSend
 * ����  ������WF-Esp32ģ�����͸������
 * ����  ����
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_UnvarnishSend ( void )
{
	if ( ! Esp32_Cmd ( "AT+CIPMODE=1", "OK", 0, 200 ) )
		return false;
	
	return 
	  Esp32_Cmd ( "AT+CIPSEND", "OK", ">", 200 );
	
}


/*
 * ��������Esp32_ExitUnvarnishSend
 * ����  ������WF-Esp32ģ���˳�͸��ģʽ
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void Esp32_ExitUnvarnishSend ( void )
{
	vTaskDelay ( pdMS_TO_TICKS(1000) );
	
	macEsp32_Usart ( "+++" );
	
	vTaskDelay ( pdMS_TO_TICKS(500) );
	Parameter.Esp32TransmissionMode=BrainTransmission;
	Esp32_Cmd ( "AT+CIPMODE=0", "OK", 0, 200); //�˳�͸��ģʽ
	Esp32_Cmd ( "AT+CIPCLOSE", "OK", 0, 200); //�˳�TCP����
	
}

bool Esp32_DisconnAP ( void ) //�Ͽ�AP����
{
	char cStr [20];
	
	sprintf ( cStr, "AT+CWQAP" );
	
	return Esp32_Cmd ( cStr, "OK", 0, 500 );
	
}




/*
 * ��������Esp32_SendString
 * ����  ��WF-Esp32ģ�鷢���ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 *       ��pStr��Ҫ���͵��ַ���
 *       ��ulStrLength��Ҫ���͵��ַ������ֽ���
 *       ��ucId���ĸ�ID���͵��ַ���
 * ����  : 1�����ͳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool Esp32_SendString ( FunctionalState enumEnUnvarnishTx, char * pStr, uint32_t ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
	char cStr [20];
	bool bRet = false;
	
		
	if ( enumEnUnvarnishTx )
	{
		macEsp32_Usart ( "%s", pStr );
		
		bRet = true;
		
	}

	else
	{
		if ( ucId < 5 )
			sprintf ( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );

		else
			sprintf ( cStr, "AT+CIPSEND=%d", ulStrLength + 2 );
		
		Esp32_Cmd ( cStr, "> ", 0, 1000 );

		bRet = Esp32_Cmd ( pStr, "SEND OK", 0, 1000 );
  }
	
	return bRet;

}


/*
 * ��������Esp32_ReceiveString
 * ����  ��WF-Esp32ģ������ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 * ����  : ���յ����ַ����׵�ַ
 * ����  �����ⲿ����
 */
char * Esp32_ReceiveString ( FunctionalState enumEnUnvarnishTx )
{
	char * pRecStr = 0;
	
	
	strEsp32_Fram_Record .InfBit .FramLength = 0;
	strEsp32_Fram_Record .InfBit .FramFinishFlag = 0;
	
	while ( ! strEsp32_Fram_Record .InfBit .FramFinishFlag );
	strEsp32_Fram_Record .Data_RX_BUF [ strEsp32_Fram_Record .InfBit .FramLength ] = '\0';
	
	if ( enumEnUnvarnishTx )
		pRecStr = strEsp32_Fram_Record .Data_RX_BUF;
	
	else 
	{
		if ( strstr ( strEsp32_Fram_Record .Data_RX_BUF, "+IPD" ) )
			pRecStr = strEsp32_Fram_Record .Data_RX_BUF;

	}

	return pRecStr;
	
}

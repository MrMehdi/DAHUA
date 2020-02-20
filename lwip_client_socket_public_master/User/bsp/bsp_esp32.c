#include "bsp.h"
#include "app.h"

#include "bsp_esp32.h"
volatile uint32_t TxdBufHeadIndex = 0;
volatile uint32_t TxdBufTailIndex = 0;
extern SemaphoreHandle_t WRITE_ready;
uint8_t receive_buff[IdleRxdBufSize];
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_tx;

volatile uint8_t a_uart_RxBuffer[20];

uint16_t TXDBUFLength[TxdBufLine]={0};
extern uint8_t CmdBuf[RX_BUFFER_SIZE];
extern uint16_t CmdBufLength;

extern void software_reset(void);


void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==USART3)
  {

    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PB13 (NJTRST)     ------> USART3_CTS
    PB14 (JTDO/TRACESWO)     ------> USART3_RTS
    PB11    ------> USART3_RX
    PB10      ------> USART3_TX 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_14);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmatx);

    /* USART3 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct;// = {0};
  if(huart->Instance==USART3)
  {

    /* Peripheral clock enable */
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOJ_CLK_ENABLE();
		__HAL_RCC_GPIOK_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
		__HAL_RCC_DMA2_CLK_ENABLE();//DMA2ʱ��ʹ��	

		//spi5  clk pk0     miso pj11    mosi pj10
		GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);
		
		GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

		//CTS  RTS
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLUP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//RXD TXD
		GPIO_InitStruct.Pin=GPIO_PIN_11|GPIO_PIN_10;			//PB10/11
		GPIO_InitStruct.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_InitStruct.Pull=GPIO_PULLUP;//GPIO_NOPULL;	//		//����
		GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_VERY_HIGH;//����
		GPIO_InitStruct.Alternate=GPIO_AF7_USART3;	//����ΪUSART3
		HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);	   	//��ʼ��PB10/11


    /* USART6 DMA Init */
    hdma_usart3_tx.Instance = DMA2_Stream7;//
    hdma_usart3_tx.Init.Request = DMA_REQUEST_USART3_TX;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
		HAL_DMA_DeInit(&hdma_usart3_tx);   

    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
    {
      ERROR_HANDLER();
    }
    __HAL_LINKDMA(huart,hdmatx,hdma_usart3_tx);

//	 __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);  // ʹ�ܴ��ڿ����ж�

		/* USART6 DMA interrupt Init */
		HAL_NVIC_SetPriority(DMA2_Stream7_IRQn,2, 0);
	  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    /* USART6 interrupt Init */
		HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(USART3_IRQn);


  }

}


void esp32_gpio_init(void)
{

	GPIO_InitTypeDef gpio_init_structure;

	/* ʹ�� GPIOʱ�� */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* ���� GPIOB ��ص�IOΪ����������� */
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull = GPIO_PULLDOWN;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	gpio_init_structure.Pin = GPIO_PIN_1;
	HAL_GPIO_Init(GPIOB, &gpio_init_structure);
	
	gpio_init_structure.Pin = GPIO_PIN_2;
	HAL_GPIO_Init(GPIOC, &gpio_init_structure);

}

void DelayMS ( uint16_t t) 
{ 

uint16_t  i; 
while (t--) 
for ( i=0; i<24000; i++ ); 

} 

void bsp_Esp32_Init(void)
{
				
	esp32_gpio_init();

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 3000000;//115200;//9600;//
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode =UART_MODE_TX_RX;// UART_MODE_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;//UART_HWCONTROL_NONE;//
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;


  if (HAL_UART_Init(&huart3) != HAL_OK)
  {

    ERROR_HANDLER();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
		ERROR_HANDLER();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
		ERROR_HANDLER();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
		ERROR_HANDLER();	
  }
	
//	DelayMS(200);

	ResetEsp32();
//	HAL_Delay(500);
	HAL_UART_Receive_IT(&huart3, (uint8_t *)a_uart_RxBuffer, 1);
	EnableEsp32Power();	
//	HAL_Delay(500);
	RunEsp32();
}

/**
  * @brief This function handles USART1 global interrupt.
  */
extern uint8_t receive_buff[IdleRxdBufSize];  

//volatile uint16_t esp32_ready=0;
//volatile uint16_t WifiConnectedFlag=0;
//volatile uint16_t WifiDisconnectedFlag=0;

volatile uint16_t RxdBufHeadIndex=0;
volatile uint16_t RxdBufTailIndex=0;
uint8_t  RxdBuf[RxdBufLine];
uint32_t data_length=0;

//void USAR_UART_IDLECallback(UART_HandleTypeDef *huart)
//{
//	char cmpbuf[20]; //�����Ƚϵ�buf���ᳬ��20���ֽ�
//	// ֹͣ����DMA����
////    HAL_UART_DMAStop(&huart1);  
//                                            
//    // ������յ������ݳ���
////    data_length  = IdleRxdBufSize - __HAL_DMA_GET_COUNTER(&hdma_usart3_rx);   
//    
//	// ������ջ�����
//		
//		for(uint32_t i=0;i<data_length;i++)
//		{
//			if(!isRxdBufFull())
//			{
//					RxdBuf[RxdBufHeadIndex]=receive_buff[i];
//					IncreaseRxdBufNum(RxdBufHeadIndex);

//			}
//		}	
//		memcpy(cmpbuf,receive_buff,20);          		//�������������жϱ�־λ(�ȶ�USART_SR��Ȼ���USART_DR)
//		if(strstr ( (const char*)cmpbuf, "ready" ) ? 1 : 0)
//		{
//			esp32_ready=1;
//		}else	if(strstr ( (const char*)cmpbuf, "connectserver" ) ? 1 : 0)
//		{
//			WifiConnectedFlag=1;
//		}else if(strstr ((const char*) cmpbuf, "lostserver" ) ? 1 : 0)
//		{
//			WifiDisconnectedFlag=1;
////			  software_reset();
//		}
//		
////				HAL_UART_Transmit_DMA(&huart6,receive_buff,2);//���Խ��յ����ݺͷ�������
////    // ������ʼDMA���� ÿ��255�ֽ�����
//		memset(receive_buff,0,data_length); 
//		memset(cmpbuf,0,20); //��ֻ��20���ֽڵĳ���  
//    data_length = 0;

//}

//void USER_UART_IRQHandler(UART_HandleTypeDef *huart)
//{	// �ж��Ƿ��Ǵ���1
//	
//    if(USART3 == huart3.Instance)                                   
//    {	// �ж��Ƿ��ǿ����ж�
//        if(RESET != __HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE))   
//        {	 // ��������жϱ�־�������һֱ���Ͻ����жϣ�
//            __HAL_UART_CLEAR_IDLEFLAG(&huart3);                    
//            // �����жϴ�����
//            USAR_UART_IDLECallback(huart);                          
//        }
//    }
//}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  UNUSED(huart);
//	if(huart->Instance==USART3)//����Ǵ���3
//	{
	
//		if(data_length >= 1024)  //����ж�
//		{
//			data_length = 0;		
//		}
//		receive_buff[data_length]=a_uart_RxBuffer[0];//һ���жϽ���һ��
//		data_length++;

		if(!isRxdBufFull())
		{
				RxdBuf[RxdBufHeadIndex]=a_uart_RxBuffer[0];
				IncreaseRxdBufNum(RxdBufHeadIndex);

		}
//	}	
	HAL_UART_Receive_IT(&huart3, (uint8_t *)&a_uart_RxBuffer, 1);   //�ٿ��������ж�

}


//uint8_t WriteDataToTXDBUF(uint8_t * source,uint16_t length)
//{
//	uint32_t i;
//	uint8_t result;
//		xSemaphoreTake(WRITE_ready, portMAX_DELAY);//���ź���


//	//��������-*********************************************----------//

////		taskENTER_CRITICAL();   //��Ӧ�����������жϣ�������
//		if(isTxdBufFull()) 
//		{
//		
////		taskEXIT_CRITICAL();
//		xSemaphoreGive(WRITE_ready);//�ͷ��ź������

//		return 0;
//		} //�������˾Ͳ���
//		
//		
//		for(i=0;i<length;i++)
//			TXDBUF[TxdBufHeadIndex][i]=source[i];
//			/* AXI SRAM��SDRAM��64KB���ݴ������ ***********************************************/

//		TXDBUFLength[TxdBufHeadIndex]=length;
//		Increase(TxdBufHeadIndex);  
////		taskEXIT_CRITICAL();
//		xSemaphoreGive(WRITE_ready);//�ͷ��ź������
//	//	bsp_LedStatue(0,1);
//		return 1;

//}


void USART3_IRQHandler(void)
{  
	
	HAL_UART_IRQHandler(&huart3);

    // ����ӵĺ��������������ڿ����ж�
//  USER_UART_IRQHandler(&huart3);                                

}

void DMA2_Stream7_IRQHandler(void)
{

  HAL_DMA_IRQHandler(&hdma_usart3_tx);

}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
//�ص�����

//	bsp_LedStatue(0,1);
		 	Increase(TxdBufTailIndex);
	 if(!isTxdBufEmpty())		
	 {

			HAL_UART_Transmit_DMA(&huart3,TXDBUF[TxdBufTailIndex],TXDBUFLength[TxdBufTailIndex]);    	 
	 }

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
//	 return Esp32_wait_response( "OK", NULL, 10);
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
//	 return Esp32_wait_response( "OK", NULL, 10);
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
//	return Esp32_wait_response( "connectserver", NULL, 400);
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
//	 return Esp32_wait_response( "OK", NULL, 10);
}



#include "bsp.h"
#include "app.h"
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32H7������
//FDCAN��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2018/6/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
FDCAN_HandleTypeDef FDCAN1_Handler;
FDCAN_RxHeaderTypeDef FDCAN1_RxHeader;
FDCAN_TxHeaderTypeDef FDCAN1_TxHeader;
#define FDCAN1_RX0_INT_ENABLE 1

#define CAN_SET_BOARD_COLLECT 0X00
#define CAN_REQ_SENSOR_STATUE 0X01
#define CAN_REPLY_SENSOR_STATUE 0X81
#define CAN_REQ_BOARD_KIND 0X02
#define CAN_REPLY_BOARD_KIND 0X82
#define CAN_REQ_BOARD_SELFTEST 0X03
#define CAN_REPLY_BOARD_SELFTEST 0X83
#define CAN_REQ_BOARD_FREQ 0X04
#define CAN_REPLY_BOARD_FREQ 0X84
#define CAN_SET_BOARD_FREQ 0X05
//��ʼ��FDCAN1��������Ϊ500Kbit/S
//����FDCAN1��ʱ��ԴΪPLL1Q=200Mhz
//presc:��Ƶֵ��ȡֵ��Χ1~512
//ntsjw������ͬ����Ծʱ�䵥Ԫ.��Χ:1~128
//ntsg1: ȡֵ��Χ2~256
//ntsg2: ȡֵ��Χ2~128
//mode:FDCAN_MODE_NORMAL,��ͨģʽ;FDCAN_MODE_EXTERNAL_LOOPBACK,�ػ�ģʽ;
//����ֵ:0,��ʼ��OK;
//    ����,��ʼ��ʧ��; 
void bsp_InitCan1(void)
{
    FDCAN_FilterTypeDef FDCAN1_RXFilter;
        
    //��ʼ��FDCAN1
    HAL_FDCAN_DeInit(&FDCAN1_Handler);                              //�������ǰ������
    FDCAN1_Handler.Instance=FDCAN1;
    FDCAN1_Handler.Init.FrameFormat=FDCAN_FRAME_CLASSIC;            //��ͳģʽ
    FDCAN1_Handler.Init.Mode=FDCAN_FRAME_CLASSIC;                                  //�ػ�����
    FDCAN1_Handler.Init.AutoRetransmission=DISABLE;                 //�ر��Զ��ش�����ͳģʽ��һ��Ҫ�رգ�����
    FDCAN1_Handler.Init.TransmitPause=DISABLE;                      //�رմ�����ͣ
    FDCAN1_Handler.Init.ProtocolException=DISABLE;                  //�ر�Э���쳣����
    FDCAN1_Handler.Init.NominalPrescaler=10;                     //��Ƶϵ��
    FDCAN1_Handler.Init.NominalSyncJumpWidth=8;                 //����ͬ����Ծ���
    FDCAN1_Handler.Init.NominalTimeSeg1=31;                      //tsg1��Χ:2~256
    FDCAN1_Handler.Init.NominalTimeSeg2=8;                      //tsg2��Χ:2~128
    FDCAN1_Handler.Init.MessageRAMOffset=0;                         //��ϢRAMƫ��
    FDCAN1_Handler.Init.StdFiltersNbr=0;                            //��׼��ϢID�˲������
    FDCAN1_Handler.Init.ExtFiltersNbr=0;                            //��չ��ϢID�˲������
    FDCAN1_Handler.Init.RxFifo0ElmtsNbr=1;                          //����FIFO0Ԫ�ر��
    FDCAN1_Handler.Init.RxFifo0ElmtSize=FDCAN_DATA_BYTES_8;         //����FIFO0Ԫ�ش�С��8�ֽ�
    FDCAN1_Handler.Init.RxBuffersNbr=0;                             //���ջ�����
    FDCAN1_Handler.Init.TxEventsNbr=0;                              //�����¼����
    FDCAN1_Handler.Init.TxBuffersNbr=0;                             //���ͻ�����
    FDCAN1_Handler.Init.TxFifoQueueElmtsNbr=1;                      //����FIFO����Ԫ�ر��
    FDCAN1_Handler.Init.TxFifoQueueMode=FDCAN_TX_FIFO_OPERATION;    //����FIFO����ģʽ
    FDCAN1_Handler.Init.TxElmtSize=FDCAN_DATA_BYTES_8;              //���ʹ�С:8�ֽ�
    if(HAL_FDCAN_Init(&FDCAN1_Handler)!=HAL_OK) 
		{};           //��ʼ��FDCAN
  
    //����RX�˲���   
    FDCAN1_RXFilter.IdType=FDCAN_STANDARD_ID;                       //��׼ID
    FDCAN1_RXFilter.FilterIndex=0;                                  //�˲�������                   
    FDCAN1_RXFilter.FilterType=FDCAN_FILTER_MASK;                   //�˲�������
    FDCAN1_RXFilter.FilterConfig=FDCAN_FILTER_TO_RXFIFO0;           //������0������FIFO0  
    FDCAN1_RXFilter.FilterID1=0x0000;                               //32λID
    FDCAN1_RXFilter.FilterID2=0x0000;                               //���FDCAN����Ϊ��ͳģʽ�Ļ���������32λ����
    if(HAL_FDCAN_ConfigFilter(&FDCAN1_Handler,&FDCAN1_RXFilter)!=HAL_OK) 
		{}	;//�˲�����ʼ��
    HAL_FDCAN_Start(&FDCAN1_Handler);                               //����FDCAN
    HAL_FDCAN_ActivateNotification(&FDCAN1_Handler,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);
    
}

//FDCAN1�ײ��������������ã�ʱ��ʹ��
//HAL_FDCAN_Init()����
//hsdram:FDCAN1���
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
    GPIO_InitTypeDef GPIO_Initure;
    RCC_PeriphCLKInitTypeDef FDCAN_PeriphClk;
    
    __HAL_RCC_FDCAN_CLK_ENABLE();                   //ʹ��FDCAN1ʱ��
    __HAL_RCC_GPIOA_CLK_ENABLE();			        //����GPIOAʱ��
	
    //FDCAN1ʱ��Դ����ΪPLL1Q
    FDCAN_PeriphClk.PeriphClockSelection=RCC_PERIPHCLK_FDCAN;
    FDCAN_PeriphClk.FdcanClockSelection=RCC_FDCANCLKSOURCE_PLL;
    HAL_RCCEx_PeriphCLKConfig(&FDCAN_PeriphClk);
    
    GPIO_Initure.Pin=GPIO_PIN_11|GPIO_PIN_12;       //PA11,12
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //���츴��
    GPIO_Initure.Pull=GPIO_PULLUP;                  //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_MEDIUM;      //������
    GPIO_Initure.Alternate=GPIO_AF9_FDCAN1;         //����ΪCAN1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);             //��ʼ��
    
   
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn,1,2);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);

}

//�˺����ᱻHAL_FDCAN_DeInit����
//hfdcan:fdcan���
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
    __HAL_RCC_FDCAN_FORCE_RESET();       //��λFDCAN1
    __HAL_RCC_FDCAN_RELEASE_RESET();     //ֹͣ��λ
    
 
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);

}

//can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)	
//len:���ݳ���(���Ϊ8),������ΪFDCAN_DLC_BYTES_2~FDCAN_DLC_BYTES_8				     
//msg:����ָ��,���Ϊ8���ֽ�.
//����ֵ:0,�ɹ�;
//		 ����,ʧ��;
uint8_t id=0;
void can1_SendPacket(uint8_t *_DataBuf, uint8_t _Len)
{		
	FDCAN_TxHeaderTypeDef TxHeader;

	if (_Len > 8)
	{
		return;
	}
	
	/* Prepare Tx Header */
	TxHeader.Identifier = 0x111;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.DataLength = (uint32_t)_Len << 16;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_FRAME_CLASSIC;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;
	
    /* Add messages to TX FIFO */
    HAL_FDCAN_AddMessageToTxFifoQ(&FDCAN1_Handler, &TxHeader, _DataBuf);
}

void can1_SendBUF(uint8_t *_DataBuf, uint8_t _Len,uint32_t targetID)
{		
	FDCAN_TxHeaderTypeDef TxHeader;

	if (_Len > 8)
	{
		return;
	}
	
	/* Prepare Tx Header */
	TxHeader.Identifier = targetID;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.DataLength = (uint32_t)_Len << 16;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_FRAME_CLASSIC;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;
	
    /* Add messages to TX FIFO */
    HAL_FDCAN_AddMessageToTxFifoQ(&FDCAN1_Handler, &TxHeader, _DataBuf);
}
////can�ڽ������ݲ�ѯ
////buf:���ݻ�����;	 
////����ֵ:0,�����ݱ��յ�;
////		 ����,���յ����ݳ���;
//u8 FDCAN1_Receive_Msg(u8 *buf)
//{	
//    if(HAL_FDCAN_GetRxMessage(&FDCAN1_Handler,FDCAN_RX_FIFO0,&FDCAN1_RxHeader,buf)!=HAL_OK)return 0;//��������
//	return FDCAN1_RxHeader.DataLength>>16;	
//}


//FDCAN1�жϷ�����
void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&FDCAN1_Handler);
}

uint8_t rxdata[8];
uint8_t write_data_canbuf(uint8_t * source,uint16_t length) //��߲�û����can��ר�����ݴ��
{
	can1_SendBUF(source,length,Parameter.can_target_addr);  //
}
uint8_t can_txddata[8];
extern uint8_t board_all_read;
uint8_t AnalyseCan(void)
{
	
	if( ( (Parameter.can_addr+0x100) != FDCAN1_RxHeader.Identifier)
		&& ( 0x104 != FDCAN1_RxHeader.Identifier) ){
		return 0;
	}
	switch(rxdata[0]){
		case CAN_SET_BOARD_COLLECT:
			if(rxdata[1]==0)
			{
				AD7606_StartRecord(config.ADfrequence);
			}else
			{
				AD7606_StopRecord();
			}
			break;
		case CAN_REQ_SENSOR_STATUE:
//			can_txddata[0]=CAN_REPLY_SENSOR_STATUE;
//			can_txddata[1]=Parameter.sensor_satue;
//			can_txddata[1]=Parameter.sensor_satue>>8;
//			write_data_canbuf(can_txddata,8);
			break;
		case CAN_REQ_BOARD_KIND:
//			can_txddata[0]=CAN_REPLY_BOARD_KIND;
//			can_txddata[1]=Parameter.board_kind;
//			write_data_canbuf(can_txddata,8);
			break;
		case CAN_REQ_BOARD_SELFTEST:
//			can_txddata[0]=CAN_REPLY_BOARD_SELFTEST;
//			can_txddata[1]=Parameter.selftest_statue;//��ʱδ�������Լ���ж���ʽ�������Ż�
		if((FDCAN1_RxHeader.Identifier+0x100)==0x100)//�ж��ǵڼ����忨��������Ϣ
		{
			board_all_read |= 1;
		}else if((FDCAN1_RxHeader.Identifier+0x100)==0x110)
		{
			board_all_read |= 2;
		}
		
		board_all_read = 3;//��߽����ļ��ſ���ֵһ��
//			write_data_canbuf(can_txddata,8);
			break;
		case CAN_REQ_BOARD_FREQ:
			can_txddata[0]=CAN_REPLY_BOARD_FREQ;
			can_txddata[1]=config.ADfrequence;
			can_txddata[2]=config.ADfrequence>>8;
			can_txddata[3]=config.freq;
			can_txddata[4]=config.freq>>8;
			write_data_canbuf(can_txddata,8);
			break;
		case CAN_SET_BOARD_FREQ:
			config.ADfrequence=(uint16_t)rxdata[1]|(rxdata[2]<<8);
			config.freq=(uint16_t)rxdata[3]|(rxdata[4]<<8);
			break;
		default:
			break;
	}
}
//FIFO0�ص�����
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t i=0;

    if((RxFifo0ITs&FDCAN_IT_RX_FIFO0_NEW_MESSAGE)!=RESET)   //FIFO1�������ж�
    {
        //��ȡFIFO0�н��յ�������
        HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO0,&FDCAN1_RxHeader,rxdata);
			
        HAL_FDCAN_ActivateNotification(hfdcan,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);
				AnalyseCan();
    }
}


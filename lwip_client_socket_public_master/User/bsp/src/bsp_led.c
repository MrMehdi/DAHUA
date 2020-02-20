/*
*********************************************************************************************************
*
*	ģ������ : LEDָʾ������ģ��
*	�ļ����� : bsp_led.c
*	��    �� : V1.0
*	˵    �� : ����LEDָʾ��
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2018-09-05 armfly  ��ʽ����
*
*	Copyright (C), 2015-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

	
/*
	suozhang
	2019-4-1 11:00:24
	STM32H743 Nucleo-144 
	
	led red PB14

*/	
#define GPIO_PORT_LED1  GPIOA
#define GPIO_PIN_LED1		GPIO_PIN_0

#define GPIO_PORT_LED2  GPIOA
#define GPIO_PIN_LED2		GPIO_PIN_6


void Down_statu2(void)
{
	HAL_GPIO_WritePin(GPIO_PORT_LED2, GPIO_PIN_LED2,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO_PORT_LED1, GPIO_PIN_LED1,GPIO_PIN_SET);
}


static void led_config_gpio(void)
{

	GPIO_InitTypeDef gpio_init_structure;

	/* ʹ�� GPIOʱ�� */
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();

	/* ���� GPIOB ��ص�IOΪ����������� */
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init_structure.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
	HAL_GPIO_Init(GPIOJ, &gpio_init_structure);
	
//	/* ����GPIOA */
//	gpio_init_structure.Pin = GPIO_PIN_LED1;
//	HAL_GPIO_Init(GPIO_PORT_LED1, &gpio_init_structure);
//	
//	gpio_init_structure.Pin = GPIO_PIN_LED2;
//	HAL_GPIO_Init(GPIO_PORT_LED2, &gpio_init_structure);

	
//	
////		/* ����GPIOA */
//	gpio_init_structure.Pin = GPIO_PIN_3;
//	HAL_GPIO_Init(GPIOB, &gpio_init_structure);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitLed
*	����˵��: ����LEDָʾ����ص�GPIO,  �ú����� bsp_Init() ���á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	led_config_gpio();
	
//	bsp_LedOff(1);

}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedOn
*	����˵��: ����ָ����LEDָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_LedStatue(uint8_t _no,uint8_t nb)
{
	if(nb==1)
	{
		if (_no == 1)
		{
			HAL_GPIO_WritePin(GPIO_PORT_LED1, GPIO_PIN_LED1,GPIO_PIN_SET);
		}else
		{
			HAL_GPIO_WritePin(GPIO_PORT_LED1, GPIO_PIN_LED1,GPIO_PIN_RESET);
		}
	}else
	{
		if (_no == 2)
		{					
			HAL_GPIO_WritePin(GPIO_PORT_LED2, GPIO_PIN_LED2,GPIO_PIN_SET);
		}else
		{
			HAL_GPIO_WritePin(GPIO_PORT_LED2, GPIO_PIN_LED2,GPIO_PIN_RESET);
		}
	}

}



/*
*********************************************************************************************************
*	�� �� ��: bsp_LedToggle
*	����˵��: ��תָ����LEDָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
void bsp_LedToggle(uint8_t _no)
{

	if (_no == 1)
	{
		HAL_GPIO_TogglePin(GPIO_PORT_LED1, GPIO_PIN_LED1);
	}else if(_no == 2)
	{
		HAL_GPIO_TogglePin(GPIO_PORT_LED2, GPIO_PIN_LED2);
	}

}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
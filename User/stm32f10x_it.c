/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stm32f10x.h"
#include "systemclock.h"
#include "HandleTask.h"
#include "Stm32_Configuration.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 RxFlag3=1;
u8 RxFlag2=1;
u8 RxFlag1=1;
u8 TxFlag3=1;
u8 TxFlag2=1;
u8 TxFlag1=1;
u8 RecLen3=1;
u8 RecLen2=1;
u8 RecLen1=1;
u8 RecDataBuffer3[128];	 //����3�������ϴ�
u8 RecDataBuffer2[128];	 //����2�������ϴ�
u8 RecDataBuffer1[128];

u8 USART3SendTCB[128];	 //����3�������ϴ�
u8 USART3BufferCNT=0;
u8 TxCount3=0;
#define RXENABLE3		         GPIO_ResetBits(GPIOA, GPIO_Pin_15)

u8 USART2SendTCB[128];	 //����2�������ϴ�
u8 USART2BufferCNT=0;
u8 TxCount2=0;
#define RXENABLE2		         GPIO_ResetBits(GPIOA, GPIO_Pin_1)

u8 USART1SendTCB[128];
u8 USART1BufferCNT=0;
u8 TxCount1=0;

u8 SI4463_RxBUFF[128] = {0};
u8 SI4463_RxLenth = 0;
u8 Int_BUFF[9] = {0};
u8 cmd[2] = {0}; 
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */

void SysTick_Handler(void)
{
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/
void EXTI15_10_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
 /*nIRQ�жϴ�����*/
 	Stop_timerEx(WX_CMD_EVT);
	SI4463_INT_STATUS(Int_BUFF);				//��ȡ�ж�״̬��ͬʱ���ж�
	if(Int_BUFF[3] & ( 1<<3 ))				    //�����CRC����
	{	
		Start_timerEx(WX_CMD_EVT,20);
	}
	if(Int_BUFF[3] & ( 1<<4 ))				    //����ǽ����ж�
	{
		Start_timerEx(WX_RECEIVE_EVT,50);
	}
	else if(Int_BUFF[3] & ( 1<<5 ))
	{
		 Start_timerEx(WX_CMD_EVT,20);
	}
	SI4463_INT_STATUS(Int_BUFF);
	EXTI_ClearITPendingBit(EXTI_Line4);
}

void EXTI9_5_IRQHandler(void)
{
}

void USART1_IRQHandler(void)
{	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		RxFlag1=0;
            
		Start_timerEx(RX1_TIMEOUT_EVT,3);
	
		/* Read one byte from the receive data register */
	    RecDataBuffer1[RecLen1] = USART_ReceiveData(USART1);
		RecLen1+=1;
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);		
	}
	else if(USART_GetITStatus(USART1, USART_IT_TXE) == SET)
	{
		USART_SendData(USART1, USART1SendTCB[TxCount1++] );

		if( TxCount1==USART1BufferCNT) 
		{
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);//��Ϊ�Ƿ��ͼĴ����յ��жϣ����Է����ַ��������ص�������ֻҪ���ˣ��ͻ���ж�
			TxCount1=0;
			USART1BufferCNT=0;
			TxFlag1=1;
		}
	}
}

void USART2_IRQHandler(void)
{		
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{ 
	   RxFlag2=0;
		/* Read one byte from the receive data register */
	  RecDataBuffer2[RecLen2] = USART_ReceiveData(USART2);
		RecLen2+=1;
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
    Start_timerEx(RX2_TIMEOUT_EVT,30);  //�����ݽ��к�������		
	}

  else if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
	{			
		USART_SendData(USART2,USART2SendTCB[TxCount2++]);
    USART_ClearITPendingBit(USART2,USART_IT_RXNE);		
    if(TxCount2>=USART2BufferCNT)
		{ 			
			USART_ITConfig(USART2,USART_IT_TXE,DISABLE); 
			USART_ClearITPendingBit(USART2, USART_IT_TXE); 
			USART_ITConfig(USART2, USART_IT_TC, ENABLE);    //ʹ�ܷ�������ж�
		 }
   	}	
	else if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
	{     		
		USART_ITConfig(USART2, USART_IT_TC, DISABLE);
    USART_ClearITPendingBit(USART2, USART_IT_TC);
    RXENABLE2; 		
		TxCount2=0;
		USART2BufferCNT=0;
		TxFlag2=1;
		RxFlag2=1;
	 }	 
}


void USART3_IRQHandler(void)
{	
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{ 
	   RxFlag3=0;   

		/* Read one byte from the receive data register */

	  RecDataBuffer3[RecLen3] = USART_ReceiveData(USART3);
		RecLen3+=1;
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
    Start_timerEx(RX3_TIMEOUT_EVT,3);  //�����ݽ��к�������		
	}
  else if(USART_GetITStatus(USART3, USART_IT_TXE) == SET)
	{	
		USART_SendData(USART3, USART3SendTCB[TxCount3++] );
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);//RS�����������Ľ����ж�
		if( TxCount3>=USART3BufferCNT) 
		{
          USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
			    USART_ClearITPendingBit(USART3,USART_IT_TXE);			    
			    USART_ITConfig(USART3, USART_IT_TC, ENABLE);    //ʹ�ܷ�������ж�   	
		}
   }		
	else if (USART_GetITStatus(USART3, USART_IT_TC) != RESET)
  { 
    USART_ITConfig(USART3, USART_IT_TC, DISABLE);		
    USART_ClearITPendingBit(USART3,USART_IT_TC);		
		RXENABLE3;
		TxCount3=0;
		USART3BufferCNT=0;
		TxFlag3=1;	
	 }		
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

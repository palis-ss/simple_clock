/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>

#include "SevenSegment.h"

#define NUM_BUTTONS 4

#define STATE_NORMAL			0
#define STATE_DEBOUNCING1	1
#define STATE_KEYDOWN1		2
#define STATE_HOLDING1		3
#define STATE_PRESSED1		4
#define STATE_DEBOUNCING2	5
#define STATE_KEYDOWN2		6
#define STATE_HOLDING2		7
#define STATE_PRESSED2		8

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
RTC_TimeTypeDef sTime1 = {0};

uint32_t kdtick[NUM_BUTTONS] = {0}, lasttick = 0;
uint8_t state = STATE_NORMAL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void DisplayTime(RTC_TimeTypeDef t)
{
	SevenSegment_SetDigit(1, t.Seconds%10);							
	SevenSegment_SetDigit(0, t.Seconds/10);
	SevenSegment_SetDigit(5, t.Minutes%10);
	SevenSegment_SetDigit(4, t.Minutes/10);
	SevenSegment_SetDigit(3, t.Hours%10);
	if(t.Hours>=10)
		SevenSegment_SetDigit(2, t.Hours/10);	
	else
		SevenSegment_SetDigit(2, 15);
}

void UpdateTime(uint8_t digittoset)
{
	uint8_t num0 = 0;
	
		switch(digittoset)
		{
			case 1:
				num0 = sTime1.Hours%10;
				
				if(num0 == 9)
					sTime1.Hours-=9;
				else
					sTime1.Hours++;			
				break;
			case 2:
				num0 = sTime1.Hours/10;
			
				if(num0 == 2)
					sTime1.Hours-=20;
				else
					sTime1.Hours+=10;
				
				while(sTime1.Hours > 24)
					sTime1.Hours-=10;
				
				break;						
			case 3:
				num0 = sTime1.Minutes%10;
				
				if(num0 == 9)
					sTime1.Minutes-=9;
				else
					sTime1.Minutes++;						
				break;
			case 4:
				num0 = sTime1.Minutes/10;
			
				if(num0 == 5)
					sTime1.Minutes-=50;
				else
					sTime1.Minutes+=10;
				break;
			case 5:  // seconds
				sTime1.Seconds = 0;
				break;
		}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	char s[20];
	uint32_t nowtick, updatetick = 0;
	uint8_t digittoset = 0;
	RTC_TimeTypeDef newtime;
	//RTC_DateTypeDef sDate1;
	
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
	
	UNUSED(s);
	HAL_Delay(100);
	SevenSegment_Init(8, SPI1_CS_GPIO_Port, SPI1_CS_Pin, 3, &hspi1);		
	SevenSegment_SetPeriod(6, 1);
	
	state = STATE_NORMAL;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		nowtick = HAL_GetTick();
		
		switch(state)
		{
			case STATE_NORMAL:
					if(nowtick - lasttick < 50)
						continue;
					
					if(HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET)
					{
						kdtick[0] = HAL_GetTick();  // start counting
						state = STATE_DEBOUNCING1;						
						break;
					}
					if(HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET)
					{
						kdtick[1] = HAL_GetTick();  // start counting
						state = STATE_DEBOUNCING2;
						break;
					}
					
			// BTN1 states
			case STATE_DEBOUNCING1:
				if(HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET)
				{
					 if(nowtick - kdtick[0] > 50)
						state = STATE_KEYDOWN1;
				}
				else
					state = STATE_NORMAL;
					
				break;
			case STATE_KEYDOWN1:
				if(HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_SET)
					state = STATE_PRESSED1;
				
				// we are not implementing BTN1 held down
				break;
			case STATE_PRESSED1:
				lasttick = HAL_GetTick();
			
				// we do something here and move back to normal after
				digittoset++;
				if(digittoset > 5)
					digittoset = 0;
				else
					SevenSegment_SetPeriod(6, 1);
			
				switch(digittoset)
				{
				case 1:  // hour unit
					HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);
				
					SevenSegment_SetPeriod(3, 1);					
					break;
				case 2:  // hour tenth
					SevenSegment_SetPeriod(3, 0);				
					SevenSegment_SetPeriod(2, 1);					
					break;
				case 3:  // minute unit
					SevenSegment_SetPeriod(2, 0);
					SevenSegment_SetPeriod(5, 1);
					break;
				case 4:  // minute tenth					
					SevenSegment_SetPeriod(5, 0);
					SevenSegment_SetPeriod(4, 1);
					break;				
				case 5:  // seconds					
					SevenSegment_SetPeriod(4, 0);
					SevenSegment_SetPeriod(1, 1);
					break;				
				case 0:				
					SevenSegment_SetPeriod(1, 0);
				
					HAL_RTC_SetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);
					break;
				}	
			
				state = STATE_NORMAL;
				break;
			
			
			// BTN2 states
			case STATE_DEBOUNCING2:
				if(HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET)
				{
					 if(nowtick - kdtick[1] > 50)
						state = STATE_KEYDOWN2;
				}
				else
					state = STATE_NORMAL;
					
				break;
			case STATE_KEYDOWN2:
				if(HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_SET)
					state = STATE_PRESSED2;
				
				if(nowtick - kdtick[1] > 1000)
					state = STATE_HOLDING2;
				break;
			case STATE_HOLDING2:
				if(HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_SET)
					state = STATE_NORMAL;
					
				if(digittoset > 0)
				{
					UpdateTime(digittoset);						
					DisplayTime(sTime1);							
					HAL_Delay(200);
				}
				break;
			case STATE_PRESSED2:
				lasttick = HAL_GetTick();			
				
				// set time
				if(digittoset > 0)
				{
					UpdateTime(digittoset);
					DisplayTime(sTime1);
				}
			
				state = STATE_NORMAL;
				break;			
		}
		
		
		if(state == STATE_NORMAL)
		{			
			// display time
			if(digittoset == 0)
			{
				HAL_RTC_GetTime(&hrtc, &newtime, RTC_FORMAT_BIN);
				
				if(newtime.Hours != sTime1.Hours || newtime.Minutes!=sTime1.Minutes || newtime.Seconds!=sTime1.Seconds)
				{
					sTime1 = newtime;
					
					DisplayTime(sTime1);
			
					updatetick = nowtick;
					SevenSegment_SetPeriod(6, 1);
					
					//sprintf(s, "%d:%d:%d\r\n", sTime1.Hours, sTime1.Minutes, sTime1.Seconds);				
					//CDC_Transmit_FS(s, strlen(s));
				}
				
				if(nowtick - updatetick >= 500)
						SevenSegment_SetPeriod(6, 0);
			}
		}
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 1;
  DateToUpdate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD0_GPIO_Port, LD0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LD0_Pin */
  GPIO_InitStruct.Pin = LD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN1_Pin BTN2_Pin BTN3_Pin BTN4_Pin */
  GPIO_InitStruct.Pin = BTN1_Pin|BTN2_Pin|BTN3_Pin|BTN4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_SYSTICK_Callback(void)
{
	if((HAL_GetTick()%1000) == 0)
		HAL_GPIO_TogglePin(LD0_GPIO_Port, LD0_Pin);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

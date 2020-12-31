/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DHT.h"
#include "fonts.h"
#include "ssd1306.h"
#include "test.h"
#include "bitmap.h"
#include "horse_anim.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define HUMIDITY_THRESHOLD 60
#define heatThreshold 40
#define MAX_PERIOD 30
#define heatInc 11
#define heatDec 7
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* DEFINE THE DHT DataTypedef
* Look in the DHT.h for the definition
*/
DHT_DataTypedef DHT11_Data;
float Temperature = 0, Humidity = 0;

char uart_buf[50];
uint32_t uart_buf_len;

enum states {Init, SetPeriod, HeaterOn, HeaterOff, PumpOn};
enum states state = Init;

enum buttonStates {NotPressed, Pressed};
enum buttonStates adjust = NotPressed, increase = NotPressed;

enum adjustMode {Start, Increasing, Confirm};
enum adjustMode mode = Start;

uint8_t adjustFirstRead, adjustSecondRead, increaseFirstRead, increaseSecondRead;

uint32_t cnt0, cnt, heat = 0, period = 15;
uint8_t fan2, fan3;
float fan1, simTemp;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void PrintInfo();
void PrintInfoUART();
void PrintPeriodUART();
void PrintPeriod();
void ReadAdjust();
void ReadIncrease();
void FSM();
void AdjustFSM();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void ReadAdjust() {
	adjustFirstRead = adjustSecondRead;
	adjustSecondRead = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
	if (!adjustFirstRead && adjustSecondRead) { //read rising edge
			adjust = Pressed;
	}
	else
		adjust = NotPressed;
}

void ReadIncrease() {
	increaseFirstRead = increaseSecondRead;
	increaseSecondRead = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10);
	if (!increaseFirstRead && increaseSecondRead) { //read rising edge
		increase = Pressed;
	}
	else
		increase = NotPressed;
}

void PrintInfo() {
	SSD1306_GotoXY (0,0);
	uart_buf_len = sprintf(uart_buf, "Temperature %02.1foC\r\n", simTemp);
	SSD1306_Puts (uart_buf, &Font_7x10, 1);

	SSD1306_GotoXY (0, 10);
	uart_buf_len = sprintf(uart_buf, "Humidity %d%%", (int)(Humidity));
	SSD1306_Puts (uart_buf, &Font_7x10, 1);

	SSD1306_GotoXY (0,20);
	uart_buf_len = sprintf(uart_buf, "Fan 1", fan1);
	SSD1306_Puts (uart_buf, &Font_7x10, 0);
	SSD1306_GotoXY (40,20);
	if (fan1 != 0) {
		uart_buf_len = sprintf(uart_buf, "%02.02f", fan1);
	}
	else {
		uart_buf_len = sprintf(uart_buf, "OFF    ");
	}
	SSD1306_Puts (uart_buf, &Font_7x10, 1);

	SSD1306_GotoXY (0, 30);
	uart_buf_len = sprintf(uart_buf, "Fan 2 & Heater");
	SSD1306_Puts (uart_buf, &Font_7x10, 0);
	SSD1306_GotoXY (100, 30);
	if (fan2 == 1) {
		uart_buf_len = sprintf(uart_buf, "ON ");
	}
	else {
		uart_buf_len = sprintf(uart_buf, "OFF");
	}
	SSD1306_Puts (uart_buf, &Font_7x10, 1);


	SSD1306_GotoXY (0, 40);
	uart_buf_len = sprintf(uart_buf, "Fan 3 & Bump");
	SSD1306_Puts (uart_buf, &Font_7x10, 0);
	SSD1306_GotoXY (100, 40);
	if (fan3 == 1) {
			uart_buf_len = sprintf(uart_buf, "ON ");
		}
		else {
			uart_buf_len = sprintf(uart_buf, "OFF");
		}
	SSD1306_Puts (uart_buf, &Font_7x10, 1);

	SSD1306_GotoXY (55, 52);
	uart_buf_len = sprintf(uart_buf, "%02d", cnt);
	SSD1306_Puts (uart_buf, &Font_7x10, 1);

	SSD1306_UpdateScreen(); //display
}

void printInfoUART() {
	uart_buf_len = sprintf(uart_buf, "================================");
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, uart_buf_len, 100);
	uart_buf_len = sprintf(uart_buf, "Temperature: %d.%doC\r\n", (int)(Temperature), (heat/1000)%10);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, uart_buf_len, 100);
	uart_buf_len = sprintf(uart_buf, "Humidity: %d%%\r\n", (int)(Humidity));
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, uart_buf_len, 100);
	uart_buf_len = sprintf(uart_buf, "Count: %d\r\n", cnt);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, uart_buf_len, 100);
	uart_buf_len = sprintf(uart_buf, "State: %d\r\n", state);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, uart_buf_len, 100);
}

void PrintPeriod() {
	SSD1306_GotoXY (0, 0);
	uart_buf_len = sprintf(uart_buf, "Period");
	SSD1306_Puts (uart_buf, &Font_7x10, 1);

	SSD1306_GotoXY (50, 30);
	uart_buf_len = sprintf(uart_buf, "%02lu", period);
	SSD1306_Puts (uart_buf, &Font_16x26, 1);
	SSD1306_UpdateScreen();
};

void PrintPeriodUART() {
	uart_buf_len = sprintf(uart_buf, "=================================\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, uart_buf_len, 100);
	uart_buf_len = sprintf(uart_buf, "Period: %lu\r\n", period);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, uart_buf_len, 100);
	uart_buf_len = sprintf(uart_buf, "Mode = %d\r\n", mode);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, uart_buf_len, 100);
}

void AdjustFSM() {
	switch(mode){
	case Start:
		fan2 = 0;
		fan3 = 0;
		cnt = 0;
		cnt0 = 0;
		if (adjust == NotPressed){
			mode = Increasing;
			SSD1306_Clear();
		}
		break;
	case Increasing:
		if (increase == Pressed) {
			if (period < MAX_PERIOD) {
				period++;
			}
			else {
				period = 1;
			}
		}
		if (adjust == Pressed) {
			mode = Confirm;
		}
		else {
			mode = Increasing;
		}
		break;
	case Confirm:
		mode = Start;
		state = Init;
		SSD1306_Clear();
		break;
	}
};

void FSM() {
	switch(state) {
	case Init:
		fan2 = 0;
		fan3 = 0;
		simTemp = Temperature;
		if (adjust == Pressed) {
			state = SetPeriod;
		}
		else {
			state = HeaterOn;
		}
		break;
	case SetPeriod:
		break;
	case HeaterOn:
		fan2 = 1;
		fan3 = 0;
		heat += heatInc;
		simTemp = Temperature + (float)(heat)/10000;
		if (adjust == Pressed) {
			state = SetPeriod;
		}
		else if (cnt >= period) {
			cnt = 0;
			state = PumpOn;
		}
		else if (simTemp >= heatThreshold) {
			state = HeaterOff;
		}
		else
			state = HeaterOn;
		break;
	case HeaterOff:
		fan2 = 0;
		if (adjust == Pressed) {
			state = SetPeriod;
		}
		else if (cnt >= period) {
			cnt = 0;
			state = PumpOn;
		}
		else
			state = HeaterOff;
		break;
	case PumpOn:
		fan2 = 0;
		fan3 = 1;
		heat -= heatDec;
		simTemp = Temperature + (float)(heat)/10000;
		if (adjust == Pressed) {
			state = SetPeriod;
		}
		else if (cnt >= period) {
			cnt = 0;
			state = HeaterOn;
		}
		else
			state = PumpOn;
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);

  SSD1306_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 if (DHT11_Data.error == 0) {
		 if (state != SetPeriod) {
			 PrintInfo();
			 printInfoUART();
		 }
		 else {
			 PrintPeriod();
			 PrintPeriodUART();
		 }
	 }
	else {
		SSD1306_GotoXY (0, 30);
		uart_buf_len = sprintf(uart_buf, "DHT11 timeout");
		SSD1306_Puts (uart_buf, &Font_7x10, 1);
		SSD1306_UpdateScreen();
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
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
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 6400-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 6400-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim3 && DHT11_Data.error == 0) {
		if (state != SetPeriod) {
			cnt0++;
			if (cnt0 % 100 == 0)
				cnt++;
		}
		if (DHT11_Data.Humidity > HUMIDITY_THRESHOLD) {
			fan1 = (100 * DHT11_Data.Humidity - 100 * HUMIDITY_THRESHOLD)/(100 - HUMIDITY_THRESHOLD);
		}
		else {
			fan1 = 0;
		}
		ReadAdjust();
		ReadIncrease();
		FSM();
		if (state == SetPeriod) {
			AdjustFSM();
		}
	}
	if (htim == &htim2) {
		DHT_GetData(&DHT11_Data);
		if (DHT11_Data.error == 0) {
			Temperature = DHT11_Data.Temperature;
			Humidity = DHT11_Data.Humidity;
		}
		else {
			SSD1306_Clear();
		}
	}
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

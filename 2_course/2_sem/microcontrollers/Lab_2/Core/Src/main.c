/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
GPIO_PinState GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	  if((GPIOx->IDR & GPIO_Pin) != (uint32_t)GPIO_PIN_RESET){
		  return GPIO_PIN_SET;
	  }else{
		  return GPIO_PIN_RESET;
	  }
 }

 void GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	  if((GPIOx->IDR & GPIO_Pin) != 0){
		   GPIOx->BSRR = (GPIO_Pin << 16);
	  }else{
		  GPIOx->BSRR = GPIO_Pin;
	  }
 }

 void GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState) {
   if(PinState == GPIO_PIN_SET) {
     GPIOx->BSRR = GPIO_Pin;
   } else {
     GPIOx->BSRR = (GPIO_Pin << 16);
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
  RCC->AHB1ENR = RCC->AHB1ENR | RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER &= 0xFFFFFFFFC;
  GPIOG->MODER &= ~0x14000000;
  GPIOG->MODER |= 0x14000000;

  GPIOG->OTYPER &= 0xFFFF9FFF;

  GPIOA->OSPEEDR &= ~0x00000002;
  GPIOA->OSPEEDR |= 0x00000001;
  GPIOG->OSPEEDR &= ~0x28000000;
  GPIOG->OSPEEDR |= 0x14000000;

  GPIOA->PUPDR &= 0xFFFFFFFF3;
  GPIOG->PUPDR &= 0x3C000000;

  GPIOA->LCKR |= 0x00010001;
  GPIOG->LCKR |= 0x00016000;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  const uint16_t Led1_Pin = 0x2000;
  const uint16_t Led2_Pin = 0x4000;

  GPIO_WritePin(GPIOG, Led1_Pin|Led2_Pin, GPIO_PIN_RESET);
  int score1 = 0;
  int score2 = 0;
  int cube = 0;
  int Key_State = 0;
  int Prev_Key_State = 0;
  int lose1 = 0;
  int lose2 = 0;
  int curr_player = 0;
//  Key_State = HAL_GPIO_ReadPin(GPIOA, Key_Pin);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  while (1)
  {

    /* USER CODE END WHILE */
	Key_State = GPIO_ReadPin(GPIOA, 1);



	if(Key_State == 0 && Prev_Key_State == 1 && lose1 == 0 && lose2 ==0 ){
	   cube = (rand()%6)+1;

	   for(int i = 0; i < cube; i++){
		  if(curr_player == 0){
			  GPIO_TogglePin(GPIOG, Led1_Pin);
			  HAL_Delay(400);
			  GPIO_TogglePin(GPIOG, Led1_Pin);
			  HAL_Delay(400);
		  }else{
			  GPIO_TogglePin(GPIOG, Led2_Pin);
			  HAL_Delay(400);
			  GPIO_TogglePin(GPIOG, Led2_Pin);
			  HAL_Delay(400);
		  }

	   }

	   GPIO_WritePin(GPIOG, Led1_Pin|Led2_Pin, GPIO_PIN_RESET);

	   if(curr_player == 0){
		   score1 += cube;
	   }else{
		   score2 += cube;
	   }


	   if (score1 > 21){
		   lose1 = 1;
		   GPIO_WritePin(GPIOG, Led2_Pin, GPIO_PIN_SET);
	   }

	   if (score2 > 21){
	   		   lose2 = 1;
	   		   GPIO_WritePin(GPIOG, Led1_Pin, GPIO_PIN_SET);
	   }
	}

	Prev_Key_State = Key_State;

	if (lose1) {
	      GPIO_TogglePin(GPIOG, Led2_Pin);
	      HAL_Delay(1000);
	    }

	if(lose2){
		GPIO_TogglePin(GPIOG, Led1_Pin);
		HAL_Delay(1000);
	}
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

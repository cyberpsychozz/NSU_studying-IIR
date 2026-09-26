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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* --- Лабораторная 7: АЦП + FreeRTOS --- */
#define AUDIO_BUF_LEN   8000U      /* отсчётов в фрагменте (~1 с при ~8.5 кГц) */
#define ADC_VREF        2.93f      /* опорное напряжение АЦП (VREF+), В — измеренное */
#define ADC_FULL_SCALE  4095.0f    /* 12-битный АЦП */
#define TS_V25          0.76f      /* напряжение датчика темп. при 25 °C (datasheet) */
#define TS_AVG_SLOPE    0.0025f    /* средний наклон 2.5 мВ/°C */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task1 */
osThreadId_t Task1Handle;
const osThreadAttr_t Task1_attributes = {
  .name = "Task1",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task2 */
osThreadId_t Task2Handle;
const osThreadAttr_t Task2_attributes = {
  .name = "Task2",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for btnSem */
osSemaphoreId_t btnSemHandle;
const osSemaphoreAttr_t btnSem_attributes = {
  .name = "btnSem"
};
/* Definitions for recSem */
osSemaphoreId_t recSemHandle;
const osSemaphoreAttr_t recSem_attributes = {
  .name = "recSem"
};
/* Definitions for tempMtx */
osSemaphoreId_t tempMtxHandle;
const osSemaphoreAttr_t tempMtx_attributes = {
  .name = "tempMtx"
};
/* USER CODE BEGIN PV */
uint16_t          audioBuffer[AUDIO_BUF_LEN];   /* буфер звукового фрагмента (SRAM) */
volatile uint32_t playIndex     = 0;            /* индекс воспроизводимого отсчёта */
volatile uint8_t  isPlaying     = 0;            /* флаг идущего воспроизведения */
volatile float    g_temperature = 25.0f;        /* температура МК, °C */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC3_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
void StartDefaultTask(void *argument);
void StartTask01(void *argument);
void StartTask02(void *argument);

/* USER CODE BEGIN PFP */
void     Lab_PeripheralsStart(void);
float    ReadTemperature(void);
uint32_t TempToPlaybackARR(float tempC);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC3_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  Lab_PeripheralsStart();

  /* === Запуск FreeRTOS (вынесено сюда из-за бага CubeMX 6.7.0,
   *     который не создаёт задачи и семафоры между osKernelInitialize и osKernelStart).
   *     Так код переживает перегенерацию .ioc. === */
  osKernelInitialize();

  btnSemHandle  = osSemaphoreNew(1, 1, &btnSem_attributes);
  recSemHandle  = osSemaphoreNew(1, 1, &recSem_attributes);
  tempMtxHandle = osSemaphoreNew(2, 2, &tempMtx_attributes);

  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  Task1Handle       = osThreadNew(StartTask01,     NULL, &Task1_attributes);
  Task2Handle       = osThreadNew(StartTask02,     NULL, &Task2_attributes);

  osKernelStart();                /* сюда не возвращаемся при успешном старте */
  while (1) { }                   /* страховка, если планировщик вдруг вернулся */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
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
/**
* @}
*/
/**
* @}
*/

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = ENABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

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
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 124;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
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
  htim3.Init.Prescaler = 83;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 124;
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
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 255;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : BTN_Pin */
  GPIO_InitStruct.Pin = BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */

/* ====================================================================
 *  Лабораторная 7: АЦП + FreeRTOS — наш код на регистрах (CMSIS).
 *  Периферию настраивает HAL (MX_*_Init); здесь только запуск и работа.
 * ==================================================================== */

/* Запуск периферии, оставленной HAL-инициализацией выключенной.
 * Вызывается из USER CODE BEGIN 2 (до старта планировщика). */
void Lab_PeripheralsStart(void)
{
  /* ADC1 (датчик температуры) — включить АЦП */
  ADC1->CR2 |= ADC_CR2_ADON;

  /* ADC3 (микрофон) — включить режим DMA и сам АЦП */
  ADC3->CR2 |= ADC_CR2_DMA;
  ADC3->CR2 |= ADC_CR2_ADON;

  /* TIM3 — разрешить прерывание по обновлению (счёт включаем при воспроизведении) */
  TIM3->DIER |= TIM_DIER_UIE;

  /* TIM4 — несущая Ш�?М: вывести 0, включить канал 1 и запустить таймер */
  TIM4->CCR1  = 0;
  TIM4->CCER |= TIM_CCER_CC1E;
  TIM4->CR1  |= TIM_CR1_CEN;
}

/* Чтение встроенного датчика температуры через ADC1 (программный запуск).
 * T = (Vsense - V25) / Avg_Slope + 25  (формула из RM0090). */
float ReadTemperature(void)
{
  ADC1->CR2 |= ADC_CR2_SWSTART;                 /* запуск преобразования */
  while ((ADC1->SR & ADC_SR_EOC) == 0U) { }     /* ждём конца преобразования */
  uint32_t raw = ADC1->DR;                      /* чтение DR сбрасывает флаг EOC */

  float vsense = ((float)raw * ADC_VREF) / ADC_FULL_SCALE;
  return ((vsense - TS_V25) / TS_AVG_SLOPE) + 25.0f;
}

/* Пересчёт температуры в период TIM3 -> смещение спектра при воспроизведении.
 * Чем выше температура — тем выше частота дискретизации и тон. */
uint32_t TempToPlaybackARR(float tempC)
{
  const float timTick = 90000000.0f / 84.0f;    /* тактовая TIM3: 90 МГц / (PSC+1) */
  float freq = (timTick / 125.0f) + (tempC - 25.0f) * 250.0f;
  if (freq < 4000.0f)  { freq = 4000.0f;  }
  if (freq > 16000.0f) { freq = 16000.0f; }
  uint32_t arr = (uint32_t)(timTick / freq);
  return (arr > 0U) ? (arr - 1U) : 0U;
}

/* --- Обработчики прерываний (вызываются из stm32f4xx_it.c) --- */

/* Кнопка (EXTI line0) — разбудить задачу аудио */
void Lab_OnButtonIRQ(void)
{
  if (EXTI->PR & EXTI_PR_PR0)
  {
    EXTI->PR = EXTI_PR_PR0;                     /* сброс флага EXTI (запись 1) */
    osSemaphoreRelease(btnSemHandle);
  }
}

/* Завершение записи фрагмента (DMA2 Stream0 от ADC3) */
void Lab_OnRecordDMA_IRQ(void)
{
  if (DMA2->LISR & DMA_LISR_TCIF0)
  {
    DMA2->LIFCR = DMA_LIFCR_CTCIF0;             /* сброс флага завершения DMA */
    TIM2->CR1  &= ~TIM_CR1_CEN;                 /* остановить триггер выборки */
    osSemaphoreRelease(recSemHandle);
  }
}

/* Темп воспроизведения (TIM3) — выдать очередной отсчёт в Ш�?М */
void Lab_OnPlaybackTIM3_IRQ(void)
{
  if (TIM3->SR & TIM_SR_UIF)
  {
    TIM3->SR &= ~TIM_SR_UIF;                    /* сброс флага обновления */
    if (isPlaying)
    {
      TIM4->CCR1 = (uint32_t)(audioBuffer[playIndex] >> 4);  /* 12 бит -> 8 бит */
      playIndex++;
      if (playIndex >= AUDIO_BUF_LEN)
      {
        isPlaying = 0;
        TIM4->CCR1 = 0;
      }
    }
  }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Задача 1 (defaultTask): измерение температуры встроенного датчика */
  for(;;)
  {
    float t = ReadTemperature();
    if (osSemaphoreAcquire(tempMtxHandle, 100) == osOK)
    {
      g_temperature = t;
      osSemaphoreRelease(tempMtxHandle);
    }
    osDelay(500);                 /* температуру достаточно мерить медленно */
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask01 */
/**
* @brief Function implementing the Task1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask01 */
void StartTask01(void *argument)
{
  /* USER CODE BEGIN StartTask01 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask01 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the Task2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Задача 2 (Task2): запись звука по кнопке + воспроизведение со смещением спектра */

  /* CubeMX создаёт бинарные семафоры со счётом 1 — сбрасываем их в 0 */
  osSemaphoreAcquire(btnSemHandle, 0);
  osSemaphoreAcquire(recSemHandle, 0);

  for(;;)
  {
    /* --- ожидание нажатия кнопки --- */
    osSemaphoreAcquire(btnSemHandle, osWaitForever);

    /* --- ЗАП�?СЬ: ADC3 по триггеру TIM2 + DMA в audioBuffer --- */
    ADC3->SR = 0;                                  /* сброс флагов АЦП (в т.ч. OVR) */

    DMA2_Stream0->CR &= ~DMA_SxCR_EN;              /* выключить поток DMA */
    while (DMA2_Stream0->CR & DMA_SxCR_EN) { }
    DMA2->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0
                | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0;
    DMA2_Stream0->PAR  = (uint32_t)&ADC3->DR;
    DMA2_Stream0->M0AR = (uint32_t)audioBuffer;
    DMA2_Stream0->NDTR = AUDIO_BUF_LEN;
    DMA2_Stream0->CR  |= DMA_SxCR_TCIE;            /* прерывание по завершению */
    DMA2_Stream0->CR  |= DMA_SxCR_EN;              /* запустить поток */

    TIM2->CNT = 0;
    TIM2->CR1 |= TIM_CR1_CEN;                      /* старт триггера выборки ~8.5 кГц */

    osSemaphoreAcquire(recSemHandle, osWaitForever);  /* ждём завершения записи */

    /* --- чтение текущей температуры --- */
    float t = 25.0f;
    if (osSemaphoreAcquire(tempMtxHandle, 100) == osOK)
    {
      t = g_temperature;
      osSemaphoreRelease(tempMtxHandle);
    }

    /* --- ВОСПРО�?ЗВЕДЕН�?Е: Ш�?М TIM4, темп задаёт TIM3 (зависит от температуры) --- */
    TIM3->ARR = TempToPlaybackARR(t);
    TIM3->CNT = 0;
    TIM3->SR &= ~TIM_SR_UIF;
    playIndex = 0;
    isPlaying = 1;
    TIM3->CR1 |= TIM_CR1_CEN;                      /* старт воспроизведения */

    while (isPlaying)
    {
      osDelay(10);
    }

    TIM3->CR1 &= ~TIM_CR1_CEN;                     /* стоп */
  }
  /* USER CODE END StartTask02 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

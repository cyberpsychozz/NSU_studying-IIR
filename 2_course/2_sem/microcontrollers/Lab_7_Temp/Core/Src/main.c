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
#include "music.h"
#include <stdint.h>
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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

/* Definitions for Record */
osThreadId_t RecordHandle;
const osThreadAttr_t Record_attributes = {
  .name = "Record",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};

/* Definitions for Playback */
osThreadId_t PlaybackHandle;
const osThreadAttr_t Playback_attributes = {
  .name = "Playback",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow1,
};

/* Definitions for Temperature */
osThreadId_t TemperatureHandle;
const osThreadAttr_t Temperature_attributes = {
  .name = "Temperature",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};

/* Queue for EXTI0 button events and playback events */
osMessageQueueId_t AudioQueueHandle;
const osMessageQueueAttr_t AudioQueue_attributes = {
  .name = "AudioQueue"
};

/* USER CODE BEGIN PV */


#define RECORD_SECONDS      10U
#define SAMPLES_PER_SEC     8000U
#define RECORD_SIZE         (RECORD_SECONDS * SAMPLES_PER_SEC)

#define ADC_CH_MIC          10U      /* PC0 = ADC1_IN10 */
#define ADC_CH_TEMP         16U      /* internal temperature sensor */

#define PWM_ARR_VALUE       2099U
#define TIM2_BASE_PSC       83U      /* 84 MHz / (83 + 1) = 1 MHz */
#define TIM2_BASE_ARR       124U     /* 1 MHz / (124 + 1) = 8000 Hz */

#define OWNER_BACKGROUND   0U
#define OWNER_RECORD       1U
#define OWNER_PLAYBACK     2U


typedef enum {
    MSG_START_RECORD = 1,
    MSG_PLAYBACK_READY
} message_type_t;

typedef struct {
    message_type_t type;
    uint32_t param;
} message_t;



/*
 * 12-bit samples from ADC are stored here.
 * RECORD_SIZE = 10 sec * 8000 samples/sec = 80000 samples.
 * uint16_t buffer size = 160 KB.
 */
uint16_t record_buffer[RECORD_SIZE];

volatile uint32_t record_index = 0;
volatile uint32_t recorded_size = 0;
volatile uint32_t sample_index = 0;

volatile uint8_t recording = 0;
volatile uint8_t audio_tick = 0;

volatile uint8_t playback_owner = OWNER_BACKGROUND;
volatile float current_temperature = 25.0f;
/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
void StartRecord(void *argument);
void StartPlayback(void *argument);
void StartTemperature(void *argument);

/* USER CODE BEGIN PFP */
static void RegisterPeripheralConfig(void);
static void ADC_SelectChannel(uint32_t channel, uint32_t sample_time);
static uint16_t ADC_ReadRaw(uint32_t channel, uint32_t sample_time);
static float ReadTemperature(void);
static void TIM2_SetARR(uint32_t arr);
static uint32_t TemperatureToARR(float temperature);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void RegisterPeripheralConfig(void)
{
    /*
     * GPIO clocks:
     * PA0  - button EXTI0
     * PC0  - microphone ADC1_IN10
     * PD12 - TIM4_CH1 PWM output
     * PG13/PG14 - LEDs
     */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOCEN |
                    RCC_AHB1ENR_GPIODEN |
                    RCC_AHB1ENR_GPIOGEN;

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN |
                    RCC_APB1ENR_TIM4EN;

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN |
                    RCC_APB2ENR_SYSCFGEN;

    __DSB();

    /* -------------------- PA0 button -> EXTI0 -------------------- */
    GPIOA->MODER &= ~GPIO_MODER_MODER0;      /* input */
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;  /* EXTI0 = PA0 */
    EXTI->IMR  |= EXTI_IMR_MR0;
    EXTI->RTSR |= EXTI_RTSR_TR0;              /* rising edge */
    EXTI->FTSR &= ~EXTI_FTSR_TR0;
    EXTI->PR = EXTI_PR_PR0;

    NVIC_SetPriority(EXTI0_IRQn, 6);
    NVIC_EnableIRQ(EXTI0_IRQn);

    /* -------------------- PC0 microphone input -------------------- */
    GPIOC->MODER |= GPIO_MODER_MODER0;        /* analog mode */
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD0;

    /* -------------------- PG13/PG14 LEDs -------------------- */
    GPIOG->MODER &= ~(GPIO_MODER_MODER13 | GPIO_MODER_MODER14);
    GPIOG->MODER |=  (GPIO_MODER_MODER13_0 | GPIO_MODER_MODER14_0);
    GPIOG->OTYPER &= ~(GPIO_OTYPER_OT13 | GPIO_OTYPER_OT14);
    GPIOG->PUPDR  &= ~(GPIO_PUPDR_PUPD13 | GPIO_PUPDR_PUPD14);

    /* -------------------- PD12 = TIM4_CH1 AF2 -------------------- */
    GPIOD->MODER &= ~GPIO_MODER_MODER12;
    GPIOD->MODER |=  GPIO_MODER_MODER12_1;    /* alternate function */

    GPIOD->AFR[1] &= ~(0xFU << ((12U - 8U) * 4U));
    GPIOD->AFR[1] |=  (0x2U << ((12U - 8U) * 4U));  /* AF2 = TIM4 */

    /*
     * -------------------- TIM4 PWM CH1 --------------------
     * APB1 timer clock = 84 MHz.
     * PWM frequency = 84 MHz / (2099 + 1) = 40 kHz.
     * CCR1 defines duty cycle.
     */
    TIM4->PSC = 0;
    TIM4->ARR = PWM_ARR_VALUE;
    TIM4->CCR1 = 0;

    TIM4->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M);
    TIM4->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos);  /* PWM mode 1 */
    TIM4->CCMR1 |= TIM_CCMR1_OC1PE;             /* preload enable */

    TIM4->CCER |= TIM_CCER_CC1E;                /* enable CH1 output */
    TIM4->CR1  |= TIM_CR1_ARPE;
    TIM4->EGR   = TIM_EGR_UG;
    TIM4->CR1  |= TIM_CR1_CEN;

    /*
     * -------------------- TIM2 sample timer --------------------
     * 84 MHz / (83 + 1) = 1 MHz.
     * 1 MHz / (124 + 1) = 8000 Hz.
     * TIM2 interrupt sets audio_tick.
     */
    TIM2->PSC = TIM2_BASE_PSC;
    TIM2->ARR = TIM2_BASE_ARR;
    TIM2->CNT = 0;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->EGR = TIM_EGR_UG;

    NVIC_SetPriority(TIM2_IRQn, 2);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->CR1 |= TIM_CR1_CEN;

    /*
     * -------------------- ADC1 --------------------
     * Single conversion, software start.
     * Channels are switched manually:
     * ADC1_IN10 - microphone
     * channel 16 - internal temperature sensor
     */
    ADC->CCR |= ADC_CCR_TSVREFE;               /* temperature sensor/Vref enable */
    ADC->CCR &= ~ADC_CCR_ADCPRE;
    ADC->CCR |= (1U << ADC_CCR_ADCPRE_Pos);    /* PCLK2 / 4 */

    ADC1->CR1 = 0;
    ADC1->CR2 = 0;

    ADC1->CR1 &= ~ADC_CR1_SCAN;
    ADC1->CR2 &= ~ADC_CR2_ALIGN;               /* right alignment */
    ADC1->CR2 &= ~ADC_CR2_EXTEN;               /* software trigger */
    ADC1->SQR1 &= ~ADC_SQR1_L;                 /* one conversion */

    ADC1->SMPR1 |= (7U << ADC_SMPR1_SMP16_Pos); /* temp sensor: 480 cycles */
    ADC1->SMPR1 |= (3U << ADC_SMPR1_SMP10_Pos); /* mic channel 10: 56 cycles */

    ADC1->CR2 |= ADC_CR2_ADON;
}

static void ADC_SelectChannel(uint32_t channel, uint32_t sample_time)
{
    if (channel <= 9U) {
        uint32_t shift = channel * 3U;
        ADC1->SMPR2 &= ~(7U << shift);
        ADC1->SMPR2 |=  (sample_time << shift);
    } else {
        uint32_t shift = (channel - 10U) * 3U;
        ADC1->SMPR1 &= ~(7U << shift);
        ADC1->SMPR1 |=  (sample_time << shift);
    }

    ADC1->SQR3 = channel;
}

static uint16_t ADC_ReadRaw(uint32_t channel, uint32_t sample_time)
{
    ADC_SelectChannel(channel, sample_time);

    ADC1->SR &= ~ADC_SR_EOC;
    ADC1->CR2 |= ADC_CR2_SWSTART;

    while ((ADC1->SR & ADC_SR_EOC) == 0U) {
        /* wait conversion */
    }

    return (uint16_t)(ADC1->DR & 0x0FFFU);
}

static float ReadTemperature(void)
{
    uint16_t adc_val = ADC_ReadRaw(ADC_CH_TEMP, 7U); /* 480 cycles */

    /*
     * Typical STM32F4 formula:
     * Vsense = ADC * Vref / 4096
     * Temp = ((Vsense - 0.76) / 0.0025) + 25
     */
    float voltage = ((float)adc_val * 3.3f) / 4096.0f;
    return ((voltage - 0.76f) / 0.0025f) + 25.0f;
}

static void TIM2_SetARR(uint32_t arr)
{
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->ARR = arr;
    TIM2->CNT = 0;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

static uint32_t TemperatureToARR(float temperature)
{
    /*
     * Чем выше температура, тем быстрее воспроизведение,
     * значит ARR меньше, а частота sampling выше.
     */
    float factor = 1.0f + (temperature - 25.0f) * 0.f;

    if (factor < 0.5f) factor = 0.5f;
    if (factor > 2.0f) factor = 2.0f;

    uint32_t arr = (uint32_t)((float)TIM2_BASE_ARR / factor);

    if (arr < 10U) arr = 10U;
    if (arr > 500U) arr = 500U;

    return arr;
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
  MX_TIM4_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  RegisterPeripheralConfig();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  AudioQueueHandle = osMessageQueueNew(5, sizeof(message_t), &AudioQueue_attributes);

  RecordHandle = osThreadNew(StartRecord, NULL, &Record_attributes);
  PlaybackHandle = osThreadNew(StartPlayback, NULL, &Playback_attributes);
  TemperatureHandle = osThreadNew(StartTemperature, NULL, &Temperature_attributes);

  osKernelStart();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */



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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
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
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartRecord(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  message_t msg;

  for (;;)
  {
	  if (playback_owner == OWNER_RECORD)
	  {
		  osMessageQueueGet(AudioQueueHandle, &msg, NULL, osWaitForever);

	      if (msg.type == MSG_START_RECORD)
	      {
	    	  GPIOG->BSRR = GPIO_BSRR_BS13;  /* green ON */
	          GPIOG->BSRR = GPIO_BSRR_BR14;  /* red OFF */

	          record_index = 0;
	          recorded_size = 0;
	          recording = 1;

	          while (recording && (record_index < RECORD_SIZE))
	          {
	        	  if (audio_tick) {
	        		  audio_tick = 0;
	        		  record_buffer[record_index++] = ADC_ReadRaw(ADC_CH_MIC, 3U);
	              }
	          }

	          recording = 0;
	          recorded_size = record_index;

	          if (recorded_size > 0U) {
	        	  msg.type = MSG_PLAYBACK_READY;
	              msg.param = recorded_size;

	              playback_owner = OWNER_PLAYBACK;
	              osMessageQueuePut(AudioQueueHandle, &msg, 0, 0);
	          } else {
	        	  playback_owner = OWNER_BACKGROUND;
	          }
	       }
	   }
	   else
	   {
		   osDelay(1);
	   }
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartPlayback(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  message_t msg;
  uint32_t play_index = 0;

  for (;;)
  {
	  if (playback_owner == OWNER_PLAYBACK)
	  {
		  osMessageQueueGet(AudioQueueHandle, &msg, NULL, osWaitForever);

	      if (msg.type == MSG_PLAYBACK_READY)
	      {
	    	  uint32_t play_size = msg.param;

	          current_temperature = ReadTemperature();

	          uint32_t new_arr = TemperatureToARR(current_temperature);
	          TIM2_SetARR(new_arr);

	          GPIOG->BSRR = GPIO_BSRR_BR13;  /* green OFF */
	          GPIOG->BSRR = GPIO_BSRR_BS14;  /* red ON */

	          play_index = 0;

	          while (play_index < play_size)
	          {
	        	  if (audio_tick) {
	        		  audio_tick = 0;

	                  uint16_t sample = record_buffer[play_index++];
	                  TIM4->CCR1 = ((uint32_t)sample * (PWM_ARR_VALUE + 1U)) / 4096U;
	        	  }
	          }

	          TIM4->CCR1 = 0;

	          TIM2_SetARR(TIM2_BASE_ARR);

	          playback_owner = OWNER_BACKGROUND;
	          sample_index = 0;
	      }
	  }
	  else if (playback_owner == OWNER_BACKGROUND)
	  {
		/*
		 * Фоновый звук: показывает, что PWM выход работает,
		 * пока не идёт запись или воспроизведение записанного фрагмента.
		 */
		  if (audio_tick) {
			  audio_tick = 0;

	          if (MUSIC_SIZE > 0U) {
	        	  uint8_t sample = music_data[sample_index++];
	              TIM4->CCR1 = ((uint32_t)sample * (PWM_ARR_VALUE + 1U)) / 256U;

	              if (sample_index >= MUSIC_SIZE) {
	            	  sample_index = 0;
	              }
	          }
		  }
	  }
	  else
	  {
		  osDelay(1);
	  }
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTemperature(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
	  /* ADC не трогаем во время записи, чтобы не конфликтовать с микрофоном*/
	  if (playback_owner == OWNER_BACKGROUND && recording == 0U) {
		  current_temperature = ReadTemperature();
	  }

	  osDelay(1000);
  }
  /* USER CODE END StartTask03 */
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

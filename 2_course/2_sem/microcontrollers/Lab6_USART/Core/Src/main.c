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
#include <stdio.h>
#include <string.h>
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
uint8_t my_id = 0;
uint8_t is_initiator = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
volatile uint32_t ms_ticks = 0;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
RingBuffer rx_buf1 = {0};
RingBuffer rx_buf3 = {0};
RingBuffer tx_buf1 = {0};
RingBuffer tx_buf3 = {0};


void send_string_pc(const char *str) {
    while (*str) {
        Buffer_Push(&tx_buf1, (uint8_t)*str++);
    }
}


void send_uint8_pc(uint8_t num) {
    char buf[4];
    int i = 0;
    if (num == 0) {
        send_string_pc("0");
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0) {
        Buffer_Push(&tx_buf1, buf[--i]);
    }
}


void send_ring_packet(Packet_t *pkt) {
    uint8_t check = pkt->cmd_type ^ pkt->sender_id ^ pkt->target_id ^ pkt->length;
    for (uint8_t i = 0; i < pkt->length; i++) {
        check ^= pkt->payload[i];
    }
    pkt->checksum = check;

    Buffer_Push(&tx_buf3, PACKET_PREAMBLE);
    Buffer_Push(&tx_buf3, pkt->cmd_type);
    Buffer_Push(&tx_buf3, pkt->sender_id);
    Buffer_Push(&tx_buf3, pkt->target_id);
    Buffer_Push(&tx_buf3, pkt->length);

    for (uint8_t i = 0; i < pkt->length; i++) {
        Buffer_Push(&tx_buf3, pkt->payload[i]);
    }

    Buffer_Push(&tx_buf3, pkt->checksum);
    Buffer_Push(&tx_buf3, PACKET_POSTAMBLE);
}


void process_ring_packet(Packet_t *pkt) {
    extern uint8_t my_id;
    extern uint8_t is_initiator;

    if (pkt->length < MAX_PAYLOAD_SIZE) pkt->payload[pkt->length] = '\0';
    else pkt->payload[MAX_PAYLOAD_SIZE - 1] = '\0';

    // Инициализация
    if (pkt->cmd_type == CMD_INIT) {
		uint8_t pkt_counter = pkt->payload[0];
		uint8_t pkt_stage   = pkt->payload[1];

		if (pkt_stage == 1) {
			if (is_initiator == 2) {
				is_initiator = 1;

				send_string_pc("\r\n[INIT] Ring initialized! Total nodes: ");
				send_uint8_pc(pkt_counter);
				send_string_pc("\r\n");

				pkt->payload[1] = 2;
				send_ring_packet(pkt);
			}
			else {
				is_initiator = 0;

				my_id = pkt_counter + 1;
				pkt->payload[0] = my_id;

				send_string_pc("\r\n[INIT] Assigned Node ID: ");
				send_uint8_pc(my_id);
				send_string_pc("\r\n");

				send_ring_packet(pkt);
			}
		}
		else if (pkt_stage == 2) {
			if (is_initiator == 1) {
				is_initiator = 0;
			}
			else {
				is_initiator = 0;

				send_string_pc("\r\n[INIT] Initialization finished! Total nodes in ring: ");
				send_uint8_pc(pkt_counter);
				send_string_pc("\r\n");

				send_ring_packet(pkt);
			}
		}
		return;
	}

    // СИТУАЦИЯ А: Пакет совершил полный круг и вернулся к нам
    if (pkt->sender_id == my_id) {
        if (pkt->target_id == my_id || pkt->target_id == BROADCAST_ID) {
            if (pkt->cmd_type == CMD_LED_GREEN) {
                GPIOG->BSRR = (GPIOG->ODR & GPIO_ODR_ODR_13) ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13;
            } else if (pkt->cmd_type == CMD_LED_RED) {
                GPIOG->BSRR = (GPIOG->ODR & GPIO_ODR_ODR_14) ? GPIO_BSRR_BR14 : GPIO_BSRR_BS14;
            }
        }

        if (pkt->cmd_type == CMD_CHAT) {
            send_string_pc("\r\n[ACK] ");
            send_string_pc((char *)pkt->payload);
            send_string_pc("\r\n");
        } else if (pkt->cmd_type == CMD_PRIVATE) {
            send_string_pc("\r\n[ACK -> Node ");
            send_uint8_pc(pkt->target_id);
            send_string_pc("]: ");
            send_string_pc((char *)pkt->payload);
            send_string_pc("\r\n");
        }
        return;
    }

    // СИТУАЦИЯ Б: Пакет прилетел от другого участника кольца
    if (pkt->target_id == my_id || pkt->target_id == BROADCAST_ID) {
        switch (pkt->cmd_type) {
            case CMD_CHAT:
                send_string_pc("\r\n[Node ");
                send_uint8_pc(pkt->sender_id);
                send_string_pc("]: ");
                send_string_pc((char *)pkt->payload);
                send_string_pc("\r\n");
                break;

            case CMD_PRIVATE:
                send_string_pc("\r\[Node ");
                send_uint8_pc(pkt->sender_id);
                send_string_pc(" -> Me] ");
                send_string_pc((char *)pkt->payload);
                send_string_pc("\r\n");
                break;

            case CMD_LED_GREEN:
                GPIOG->BSRR = (GPIOG->ODR & GPIO_ODR_ODR_13) ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13;
                break;

            case CMD_LED_RED:
                GPIOG->BSRR = (GPIOG->ODR & GPIO_ODR_ODR_14) ? GPIO_BSRR_BR14 : GPIO_BSRR_BS14;
                break;

            default:
                break;
        }
    }
    send_ring_packet(pkt);
}


void handle_pc_input(char *line, uint8_t len) {
    extern uint8_t my_id;
    extern uint8_t is_initiator;
    Packet_t tx_packet;

    if (line[0] == '/') {
    	if (strncmp(line, "/init", 5) == 0) {
			is_initiator = 2;
			my_id = 1;

			tx_packet.cmd_type = CMD_INIT;
			tx_packet.sender_id = my_id;
			tx_packet.target_id = BROADCAST_ID;
			tx_packet.length = 2;
			tx_packet.payload[0] = 1;
			tx_packet.payload[1] = 1;

			send_ring_packet(&tx_packet);
			send_string_pc("\r\n[INIT] Starting network map... I am Node 1\r\n");
			return;
		}

        tx_packet.sender_id = my_id;

        if (line[1] == 'g' || line[1] == 'r') {
            char cmd = line[1];
            uint8_t target = BROADCAST_ID;

            if (line[2] == ' ' && line[3] != '\0') {
                char *p = &line[3];
                while (*p >= '0' && *p <= '9') {
                    target = target * 10 + (*p - '0');
                    p++;
                }
            }
            tx_packet.cmd_type = (cmd == 'g') ? CMD_LED_GREEN : CMD_LED_RED;
            tx_packet.target_id = target;
            tx_packet.length = 0;
            send_ring_packet(&tx_packet);
        }
        else if (line[1] == 'w' && line[2] == ' ') {
            char *id_str = &line[3];
            char *msg_str = id_str;
            uint8_t target = 0;

            while (*msg_str >= '0' && *msg_str <= '9') {
                target = target * 10 + (*msg_str - '0');
                msg_str++;
            }

            if (*msg_str == ' ') {
                msg_str++;
                tx_packet.cmd_type = CMD_PRIVATE;
                tx_packet.target_id = target;

                uint8_t msg_len = 0;
                while (msg_str[msg_len] != '\0' && msg_len < MAX_PAYLOAD_SIZE) {
                    tx_packet.payload[msg_len] = msg_str[msg_len];
                    msg_len++;
                }
                tx_packet.length = msg_len;
                send_ring_packet(&tx_packet);
            }
        }
        else {
        	send_string_pc("\r\[ERROR] Unknown command '");
        	send_string_pc(line);
        	send_string_pc("'\r\n");
        }
    } else {
        tx_packet.sender_id = my_id;
        tx_packet.cmd_type = CMD_CHAT;
        tx_packet.target_id = BROADCAST_ID;

        uint8_t msg_len = len;
        if (msg_len > MAX_PAYLOAD_SIZE) msg_len = MAX_PAYLOAD_SIZE;

        tx_packet.length = msg_len;
        for (uint8_t i = 0; i < msg_len; i++) {
            tx_packet.payload[i] = line[i];
        }
        send_ring_packet(&tx_packet);
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
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOGEN;
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_SYSCFGEN;
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

  GPIOG->MODER &= ~(GPIO_MODER_MODER13 | GPIO_MODER_MODER14);
  GPIOG->MODER |= (GPIO_MODER_MODER13_0 | GPIO_MODER_MODER14_0);

  GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
  GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

  GPIOC->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
  GPIOC->MODER |= (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);

  GPIOA->AFR[1] |= 0x00000770; //AF7
  GPIOC->AFR[1] |= 0x00007700;

  USART1->BRR = 0x30D; //48.828
  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
  USART1->CR1 |= USART_CR1_UE;

  USART3->BRR = 0x187; //24.414
  USART3->CR1 |= USART_CR1_TE | USART_CR1_RE;
  USART3->CR1 |= USART_CR1_UE;

  USART1->CR1 |= USART_CR1_RXNEIE;
  NVIC_EnableIRQ(USART1_IRQn);

  USART3->CR1 |= USART_CR1_RXNEIE;
  NVIC_EnableIRQ(USART3_IRQn);

  SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
  EXTI->IMR |= EXTI_IMR_MR0;
  EXTI->FTSR |= EXTI_FTSR_TR0;
  EXTI->RTSR |= EXTI_RTSR_TR0;
  NVIC_SetPriority(EXTI0_IRQn, 0);
  NVIC_EnableIRQ(EXTI0_IRQn);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  typedef enum {
	STATE_PREAMBLE,
	STATE_CMD_TYPE,
	STATE_SENDER_ID,
	STATE_TARGET_ID,
	STATE_LENGTH,
	STATE_PAYLOAD,
	STATE_CHECKSUM,
	STATE_POSTAMBLE
  } RingParserState_t;

  RingParserState_t ring_state = STATE_PREAMBLE;
  Packet_t rx_packet;
  uint8_t payload_idx = 0;
  uint8_t calculated_checksum = 0;

  char pc_line[128];
  uint8_t pc_idx = 0;

  while (1)
  {
	if (rx_buf1.head != rx_buf1.tail) {
		char c = rx_buf1.buffer[rx_buf1.tail];
		rx_buf1.tail = (rx_buf1.tail + 1) & (BUFFER_SIZE - 1);

		if (c == '\r' || c == '\n') {
			if (pc_idx > 0) {
				pc_line[pc_idx] = '\0';
				handle_pc_input(pc_line, pc_idx);
				pc_idx = 0;
			}
		} else {
			if (pc_idx < sizeof(pc_line) - 1) {
				pc_line[pc_idx++] = c;
			}
		}
	}


	if (rx_buf3.head != rx_buf3.tail) {
		uint8_t b = rx_buf3.buffer[rx_buf3.tail];
		rx_buf3.tail = (rx_buf3.tail + 1) & (BUFFER_SIZE - 1);

		switch (ring_state) {
			case STATE_PREAMBLE:
				if (b == PACKET_PREAMBLE) {
					ring_state = STATE_CMD_TYPE;
					calculated_checksum = 0;
				}
				break;

			case STATE_CMD_TYPE:
				rx_packet.cmd_type = b;
				calculated_checksum ^= b;
				ring_state = STATE_SENDER_ID;
				break;

			case STATE_SENDER_ID:
				rx_packet.sender_id = b;
				calculated_checksum ^= b;
				ring_state = STATE_TARGET_ID;
				break;

			case STATE_TARGET_ID:
				rx_packet.target_id = b;
				calculated_checksum ^= b;
				ring_state = STATE_LENGTH;
				break;

			case STATE_LENGTH:
				if (b <= MAX_PAYLOAD_SIZE) {
					rx_packet.length = b;
					calculated_checksum ^= b;
					payload_idx = 0;
					if (rx_packet.length == 0) {
						ring_state = STATE_CHECKSUM;
					} else {
						ring_state = STATE_PAYLOAD;
					}
				} else {
					ring_state = STATE_PREAMBLE;
				}
				break;

			case STATE_PAYLOAD:
				rx_packet.payload[payload_idx++] = b;
				calculated_checksum ^= b;
				if (payload_idx >= rx_packet.length) {
					ring_state = STATE_CHECKSUM;
				}
				break;

			case STATE_CHECKSUM:
				rx_packet.checksum = b;
				if (rx_packet.checksum == calculated_checksum) {
					ring_state = STATE_POSTAMBLE;
				} else {
					ring_state = STATE_PREAMBLE;
				}
				break;

			case STATE_POSTAMBLE:
				if (b == PACKET_POSTAMBLE) {
					process_ring_packet(&rx_packet);
				}
				ring_state = STATE_PREAMBLE;
				break;
		}
	}


	if (tx_buf1.head != tx_buf1.tail && (USART1->SR & USART_SR_TXE)) {
		USART1->DR = tx_buf1.buffer[tx_buf1.tail];
		tx_buf1.tail = (tx_buf1.tail + 1) & (BUFFER_SIZE - 1);
	}


	if (tx_buf3.head != tx_buf3.tail && (USART3->SR & USART_SR_TXE)) {
		USART3->DR = tx_buf3.buffer[tx_buf3.tail];
		tx_buf3.tail = (tx_buf3.tail + 1) & (BUFFER_SIZE - 1);
	}
  /* USER CODE END 3 */
  }
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */


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

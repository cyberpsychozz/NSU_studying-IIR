/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern volatile uint32_t ms_ticks;

#define MAX_PAYLOAD_SIZE  64
#define PACKET_PREAMBLE   0xAA
#define PACKET_POSTAMBLE  0x55
#define BROADCAST_ID      0x00

typedef enum {
    CMD_CHAT        = 0x01,  // Сообщение в общий чат
    CMD_PRIVATE     = 0x02,  // Личное сообщение (адресное)
    CMD_LED_GREEN   = 0x03,  // Управление зеленым светодиодом
    CMD_LED_RED     = 0x04,  // Управление красным светодиодом
    CMD_INIT        = 0x05   // Служебная команда инициализации сети
} CmdType_t;

#pragma pack(push, 1)
typedef struct {
    uint8_t preamble;       // Стартовый байт (всегда 0xAA)
    uint8_t cmd_type;       // Тип команды / сообщения (из CmdType_t)
    uint8_t sender_id;      // ID платы-отправителя
    uint8_t target_id;      // ID платы-получателя (0x00 - для всех)
    uint8_t length;         // Реальная длина поля payload (0..MAX_PAYLOAD_SIZE)
    uint8_t payload[MAX_PAYLOAD_SIZE]; // Буфер для данных (текст, параметры)
    uint8_t checksum;       // Контрольная сумма
    uint8_t postamble;      // Стоповый байт (всегда 0x55)
} Packet_t;
#pragma pack(pop)
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define BUFFER_SIZE 512
typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
static inline void Buffer_Push(RingBuffer* rb, uint8_t data) {
    uint16_t next = (rb->head + 1) & (BUFFER_SIZE - 1);
    if (next != rb->tail) {
        rb->buffer[rb->head] = data;
        rb->head = next;
    }
}

static inline uint8_t Buffer_Pop(RingBuffer* rb) {
    uint8_t data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & (BUFFER_SIZE - 1);
    return data;
}

static inline void InjectString(RingBuffer* rb, const char* str) {
    while (*str) {
        Buffer_Push(rb, *str++);
    }
}
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

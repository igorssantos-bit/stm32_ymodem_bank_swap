/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdbool.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private defines */
#define ARRAY_LEN(x)        (sizeof(x) / sizeof((x)[0]))
#define UART1_BUFFER_SIZE 	2048

typedef struct {
	UART_HandleTypeDef* p_huart;
	DMA_TypeDef *p_hdma;
	uint32_t rx_channel;
	uint8_t* p_rxBuf;
	size_t rxBufSize;
	size_t rxPos;
} huart_port_t;

extern huart_port_t huart_p1;

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */
void uart_write_byte(huart_port_t* huart, uint8_t byte);
void uart_write_string(huart_port_t* huart, void *p_buffer, uint16_t size);
bool uart_read_byte(huart_port_t* huart, void *byte);
bool uart_read_string(huart_port_t* huart, void *p_dest, size_t len);
bool uart_read_loop(huart_port_t* huart, void *p_dest, size_t len, uint32_t timeout);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */


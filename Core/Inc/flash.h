/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flash.h
  * @brief   This file contains all the function prototypes for
  *          the flash.c file
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
#ifndef __flash_H__
#define __flash_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
/* Error code */
enum {
	FLASHIF_OK = 0,
	FLASHIF_ERASEKO,
	FLASHIF_WRITINGCTRL_ERROR,
	FLASHIF_WRITING_ERROR
};

#if defined(STM32U535xx) || defined(STM32U545xx)
#define FLASH_START_BANK1 (uint32_t) FLASH_BASE
#define FLASH_START_BANK2 (uint32_t) (FLASH_BASE+FLASH_BANK_SIZE)
#else
#error "Not compatible"
#endif

/* USER CODE END Private defines */

void MX_FLASH_Init(void);

/* USER CODE BEGIN Prototypes */
uint32_t FLASH_BankErase(uint32_t bank);
uint32_t FLASH_Write(uint32_t addr, const void *data, uint32_t cnt);
void Flash_BankSwap(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __flash_H__ */


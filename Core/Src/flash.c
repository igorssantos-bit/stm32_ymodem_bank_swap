/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flash.c
  * @brief   This file provides code for the configuration
  *          of the flash instances.
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
/* Includes ------------------------------------------------------------------*/
#include "flash.h"

/* USER CODE BEGIN 0 */
#include "string.h"
#include "icache.h"

/* USER CODE END 0 */

/* FLASH init function */
void MX_FLASH_Init(void)
{

  /* USER CODE BEGIN FLASH_Init 0 */

  /* USER CODE END FLASH_Init 0 */

  /* USER CODE BEGIN FLASH_Init 1 */

  /* USER CODE END FLASH_Init 1 */
  if (HAL_FLASH_Unlock() != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_FLASH_Lock() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FLASH_Init 2 */

  /* USER CODE END FLASH_Init 2 */

}

/* USER CODE BEGIN 1 */
/**
 * @brief  This function does an erase of all user flash area
 * @param  bank: Flash bank to be erased.
 * @retval FLASHIF_OK : user flash area successfully erased
 *         FLASHIF_ERASEKO : error occurred
 */
uint32_t FLASH_BankErase(uint32_t bank) {
	FLASH_EraseInitTypeDef desc;
	uint32_t result = FLASHIF_OK;
	uint32_t pageerror;
	/* Check the parameters */
	if(!IS_FLASH_BANK_EXCLUSIVE(bank)) return FLASHIF_ERASEKO;
	/* Unlock the Flash to enable the flash control register access */
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
	/* Setting erase options */
	desc.NbPages = FLASH_BANK_SIZE;
	desc.Page = 0U;
	desc.TypeErase = FLASH_TYPEERASE_MASSERASE;
	desc.Banks = bank;
	/* Erase bank */
	HAL_ICACHE_Disable();
    if (HAL_FLASHEx_Erase(&desc, &pageerror) != HAL_OK) result = FLASHIF_ERASEKO;
    /* Lock the Flash to disable the flash control register access */
    MX_ICACHE_Init();
	HAL_FLASH_Lock();
	return result;
}

/**
 * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
 * @note   After writing data buffer, the flash content is checked.
 * @param  addr: start address for target location
 * @param  data: pointer on buffer with data to write
 * @param  cnt: length of data buffer in bytes
 * @retval uint32_t FLASHIF_OK: Data successfully written to Flash memory
 *         FLASHIF_WRITINGCTRL_ERROR: Error occurred while writing data in Flash memory
 *         FLASHIF_WRITING_ERROR: Written Data in flash memory is different from expected one
 */
uint32_t FLASH_Write(uint32_t addr, const void *data, uint32_t cnt) {
	HAL_StatusTypeDef err;
    uint32_t loop = 0;
    void *dest = (void *)addr;
    /* Check if data is aligned */
    if (cnt % 4 != 0) return FLASHIF_WRITINGCTRL_ERROR;
    /* Unlock the Flash to enable the flash control register access */
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    HAL_ICACHE_Disable();
    /* Flash Program loop for QUADWORD type */
    do {
    	uint64_t dword[2];
    	memcpy(dword, (void *)((uint32_t)data + loop), sizeof(dword));
    	if ((dword[0] != -1) || (dword[1] != -1))
    		err = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, addr, (uint32_t)&dword[0]);
    	else
    		err = HAL_OK;
    	loop += sizeof(dword);
    	addr += sizeof(dword);
    } while ((loop != cnt) && (err == HAL_OK));
    /* Lock the Flash to disable the flash control register access */
    MX_ICACHE_Init();
    HAL_FLASH_Lock();
    /* Check written data */
	if ((err == HAL_OK) && memcmp(dest, data, cnt)) return FLASHIF_WRITING_ERROR;
    return (err == HAL_OK) ? FLASHIF_OK : FLASHIF_WRITINGCTRL_ERROR;
}

/**
 * @brief  This function swaps the active flash bank.
 * @param  None.
 * @retval uint32_t FLASH_BANK_1: Flash bank 1 is active
 *         FLASH_BANK_2: Flash bank 2 is active
 */
uint32_t Flash_Get_ActiveBank(void){
	FLASH_OBProgramInitTypeDef OBInit = {0};
	uint32_t bank = 0;
    /* Unlock the Flash to enable the flash control register access */
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
	/* Get the option bytes configuration status */
	HAL_FLASHEx_OBGetConfig(&OBInit);
	/* Check OB_SWAP_BANK status */
	if ((OBInit.USERConfig & FLASH_OPTR_SWAP_BANK_Msk) == OB_SWAP_BANK_DISABLE) {
		bank = FLASH_BANK_1;
	} else {
		bank = FLASH_BANK_2;
	}
	/* Lock the Flash to disable the flash control register access */
    HAL_FLASH_Lock();
    return bank;
}

/**
 * @brief  This function swaps the active flash bank.
 * @param  None.
 * @retval None.
 */
void Flash_BankSwap(void){
	FLASH_OBProgramInitTypeDef OBInit = {0};
    /* Unlock the Flash to enable the flash control register access */
    if (HAL_FLASH_Unlock() != HAL_OK) Error_Handler();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    /* Unlock Option Bytes */
    if (HAL_FLASH_OB_Unlock() != HAL_OK) Error_Handler();
    /* Get the option bytes configuration status */
    HAL_FLASHEx_OBGetConfig(&OBInit);
    OBInit.OptionType = OPTIONBYTE_USER;
    OBInit.USERType = OB_USER_SWAP_BANK;
    /* Check OB_SWAP_BANK status */
    if ((OBInit.USERConfig & FLASH_OPTR_SWAP_BANK_Msk) == OB_SWAP_BANK_DISABLE) {
    	OBInit.USERConfig = OB_SWAP_BANK_ENABLE; /* Swap to bank 2 */
    } else {
    	OBInit.USERConfig = OB_SWAP_BANK_DISABLE; /* Swap to bank 1 */
    }
    /*  Disable interrupts for timers */
    HAL_NVIC_DisableIRQ(SysTick_IRQn);
    /* Clear any pending ones */
    HAL_NVIC_ClearPendingIRQ(SysTick_IRQn);
    /*Disable ICACHE*/
    HAL_ICACHE_Disable();
    HAL_ICACHE_DeInit();
    /* Program Option bytes loading */
    if (HAL_FLASHEx_OBProgram(&OBInit) != HAL_OK) Error_Handler();
    /* Launch Option Bytes Loading */
    if (HAL_FLASH_OB_Launch() != HAL_OK) Error_Handler();
}

/* USER CODE END 1 */

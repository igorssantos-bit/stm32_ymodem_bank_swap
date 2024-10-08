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
#include "icache.h"

/* USER CODE BEGIN 0 */

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
 * @brief  Unlocks Flash for write access
 * @param  None
 * @retval None
 */
void FLASH_Init(void){
	/* Unlock the Program memory */
	HAL_FLASH_Unlock();
	/* Clear pending flags (if any) */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
			FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
}

/**
 * @brief  This function does an erase of all user flash area
 * @param  start: start of user flash area
 * @retval FLASHIF_OK : user flash area successfully erased
 *         FLASHIF_ERASEKO : error occurred
 */
uint32_t FLASH_Erase(uint32_t start) {
	FLASH_EraseInitTypeDef desc;
	uint32_t result = FLASHIF_OK;
	uint32_t pageerror;
	/* NOTE: Following implementation expects the IAP code address to be < Application address */
	if (start < USER_FLASH_END_ADDRESS) return FLASHIF_ERASEKO;
	/* Unlock the Flash to enable the flash control register access *************/
	FLASH_Init();
	/* Get the page where start the user flash area */
	if (start < (FLASH_BASE + FLASH_BANK_SIZE)) {
		/* Bank 1 */
		desc.Page = (start - FLASH_BASE) / FLASH_PAGE_SIZE;
		desc.Banks = FLASH_BANK_1;
	} else {
		/* Bank 2 */
		desc.Page = (start - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
		desc.Banks = FLASH_BANK_2;
	}
	desc.TypeErase = FLASH_TYPEERASE_PAGES;
	desc.NbPages = (USER_FLASH_END_ADDRESS - start) / FLASH_PAGE_SIZE;
	/* Erase user pages */
	HAL_ICACHE_Disable();
    if (HAL_FLASHEx_Erase(&desc, &pageerror) != HAL_OK) result = FLASHIF_ERASEKO;
    MX_ICACHE_Init();
    /* Lock the Flash to disable the flash control register access *************/
	HAL_FLASH_Lock();
	return result;
}

/* Public functions ---------------------------------------------------------*/
/**
 * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
 * @note   After writing data buffer, the flash content is checked.
 * @param  destination: start address for target location
 * @param  p_source: pointer on buffer with data to write
 * @param  length: length of data buffer (unit is 32-bit word)
 * @retval uint32_t 0: Data successfully written to Flash memory
 *         1: Error occurred while writing data in Flash memory
 *         2: Written Data in flash memory is different from expected one
 */
uint32_t FLASH_Write(uint32_t destination, uint32_t *p_source, uint32_t length) {
    uint32_t status = FLASHIF_OK;
    uint32_t i = 0;
    /* Certifique-se de que o comprimento é um múltiplo de 4 para quad words (16 bytes) */
    if (length % 4 != 0) return FLASHIF_WRITINGCTRL_ERROR;
    /* Unlock the Flash to enable the flash control register access */
    HAL_FLASH_Unlock();
    HAL_ICACHE_Disable();
    for (i = 0; (i < length / 4) && (destination <= (USER_FLASH_END_ADDRESS - 16)); i++) {
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by quad word */
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, destination, *((uint64_t *)(p_source + 4 * i))) == HAL_OK) {
            /* Check the written value */
            if ((uint64_t)destination != *(uint64_t *)(p_source + 4 * i)) {
                /* Flash content doesn't match SRAM content */
                status = FLASHIF_WRITINGCTRL_ERROR;
                break;
            }
            /* Increment FLASH destination address */
            destination += 16;
        } else {
            /* Error occurred while writing data in Flash memory */
            status = FLASHIF_WRITING_ERROR;
            break;
        }
    }
    MX_ICACHE_Init();
    HAL_FLASH_Lock();
    return status;
}

/**
  * @brief  Returns the write protection status of user flash area.
  * @param  None
  * @retval 0: No write protected pages inside the user flash area
  *         1: Some pages inside the user flash area are write protected
  */
uint16_t FLASH_GetWriteProtectionStatus(void) {
	uint32_t ProtectedArea = 0x0;
	FLASH_OBProgramInitTypeDef OptionsBytesStructA, OptionsBytesStructB;
	if (APPLICATION_ADDRESS < (FLASH_BASE + FLASH_BANK_SIZE)) {
		OptionsBytesStructA.WRPArea = OB_WRPAREA_BANK1_AREAA;
		/* Get the current configuration of the option bytes of A area */
		HAL_FLASHEx_OBGetConfig(&OptionsBytesStructA);
		OptionsBytesStructB.WRPArea = OB_WRPAREA_BANK1_AREAB;
		/* Get the current configuration of the option bytes of B area */
		HAL_FLASHEx_OBGetConfig(&OptionsBytesStructB);
	} else {
		OptionsBytesStructA.WRPArea = OB_WRPAREA_BANK2_AREAA;
		/* Get the current configuration of the option bytes of A area */
		HAL_FLASHEx_OBGetConfig(&OptionsBytesStructA);
		OptionsBytesStructB.WRPArea = OB_WRPAREA_BANK2_AREAB;
		/* Get the current configuration of the option bytes of B area */
		HAL_FLASHEx_OBGetConfig(&OptionsBytesStructB);
	}
	return (OptionsBytesStructA.WRPLock & OptionsBytesStructB.WRPLock);
	/* Get pages already write protected */
	ProtectedArea = OptionsBytesStructA.WRPLock & OptionsBytesStructB.WRPLock;
	return ProtectedArea;
}

/**
 * @brief  Configure the write protection status of user flash area.
 * @param  modifier DISABLE or ENABLE the protection
 * @retval HAL_StatusTypeDef HAL_OK if change is applied.
 */
HAL_StatusTypeDef FLASH_WriteProtectionConfig(uint32_t modifier) {
	FLASH_OBProgramInitTypeDef config_new, config_old;
	HAL_StatusTypeDef result = HAL_OK;
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();
	/* Unlock the Options Bytes *************************************************/
	HAL_FLASH_OB_Unlock();
	/* Get the current configuration of the option bytes ************************/
	if (APPLICATION_ADDRESS < (FLASH_BASE + FLASH_BANK_SIZE)) {
		config_old.WRPArea = OB_WRPAREA_BANK1_AREAA;
	} else {
		config_old.WRPArea = OB_WRPAREA_BANK2_AREAA;
	}
	HAL_FLASHEx_OBGetConfig(&config_old);
	/* Configure the write protection state *************************************/
	config_new.WRPLock = modifier;
	config_new.OptionType = OPTIONBYTE_WRP;
	/* Keep the current RDP level and user configuration ************************/
	config_new.RDPLevel = config_old.RDPLevel;
	config_new.USERConfig = config_old.USERConfig;
	config_new.WRPArea = config_old.WRPArea;
	/* Program the new option byte configuration ********************************/
	result = HAL_FLASHEx_OBProgram(&config_new);
	config_new.WRPArea = config_new.WRPArea << 1;
	result |= HAL_FLASHEx_OBProgram(&config_new);
	/* Lock the Options Bytes and Flash *****************************************/
	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();
	return result;
}

/* USER CODE END 1 */

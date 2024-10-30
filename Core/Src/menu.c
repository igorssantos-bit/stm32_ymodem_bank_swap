/**
 ******************************************************************************
 * @file    IAP_Main/Src/menu.c
 * @author  MCD Application Team
 * @brief   This file provides the software which contains the main menu routine.
 *          The main menu gives the options of:
 *             - downloading a new binary file,
 *             - uploading internal flash memory,
 *             - executing the binary file already loaded
 *             - configuring the write protection of the Flash sectors where the
 *               user loads his binary file.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/** @addtogroup STM32H7xx_IAP
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "flash.h"
#include "menu.h"
#include "ymodem.h"
#include "stdio.h"
#include "usart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t FlashProtection = 0;
uint8_t aFileName[FILE_NAME_LENGTH];
uint32_t BankActive = 0U, BankInactive = 0U;

/* Private function prototypes -----------------------------------------------*/
void SerialDownload(void);

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Download a file via serial port
 * @param  None
 * @retval None
 */
void SerialDownload(void) {
	uint32_t size = 0;
	COM_StatusTypeDef result;
	printf("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
	result = Ymodem_Receive(&size, BankInactive);
	if (result == COM_OK) {
		printf("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: %s", aFileName);
		printf("\n\r Size: %lu Bytes\r\n", size);
		printf("-------------------\n");
	} else if (result == COM_LIMIT) {
		printf("\n\n\rThe image size is higher than the allowed space memory!\n\r");
	} else if (result == COM_DATA) {
		printf("\n\n\rVerification failed!\n\r");
	} else if (result == COM_ABORT) {
		printf("\r\n\nAborted by user.\n\r");
	} else {
		printf("\n\rFailed to receive the file!\n\r");
	}
}

/**
 * @brief  Display the Main Menu on HyperTerminal
 * @param  None
 * @retval None
 */
void Main_Menu(void) {
	uint8_t key = 0;
	/* Test from which bank the program runs */
	if(Flash_Get_ActiveBank() == FLASH_BANK_2){
		BankActive = FLASH_BANK_2;
		BankInactive = FLASH_BANK_1;
	}else{
		BankActive = FLASH_BANK_1;
		BankInactive = FLASH_BANK_2;
	}
	printf("\r\n======================================================================");
	printf("\r\n=              (C) COPYRIGHT 2017 STMicroelectronics                 =");
	printf("\r\n=                                                                    =");
	printf("\r\n=   STM32U545 On-the-fly update for dual bank demo  (Version 2.0.0)  =");
	printf("\r\n=                                                                    =");
	if (BankActive == FLASH_BANK_2){
		printf("\r\n=                    Program running from Bank 2                     =");
	}else{
		printf("\r\n=                    Program running from Bank 1                     =");
	}
	printf("\r\n======================================================================");
	printf("\r\n\r\n");
	while (1) {
		/* Test if any sector of Flash memory where user application will be loaded is write protected */
//		FlashProtection = FLASH_GetWriteProtectionStatus();
		printf("\r\n=================== Main Menu ============================\r\n\n");
		printf("  Download image to the internal Flash ----------------- 1\r\n\n");
		printf("  Exit menu -------------------------------------------- 3\r\n\n");
//		if(FlashProtection) {
//			printf("  Disable the write protection ------------------------- 4\r\n\n");
//		} else {
//			printf("  Enable the write protection -------------------------- 4\r\n\n");
//		}
		printf("==========================================================\r\n\n");
		/* Clean the input path */
		__HAL_UART_FLUSH_DRREGISTER(&huart1);
	    __HAL_UART_CLEAR_IT(&huart1, UART_CLEAR_OREF);
		/* Receive key */
		HAL_UART_Receive(&huart1, &key, 1, HAL_MAX_DELAY);
		switch (key) {
		case '1': {
			/* Download user application in the Flash */
			SerialDownload();
		}
		break;
		case '3': {
			printf("Press BUTTON_USER to swap Banks\r\n\n");
			return;
		}
		break;
//		case '4': {
//			if (FlashProtection) {
//				/* Disable the write protection */
//				if (FLASH_WriteProtectionConfig(DISABLE) == HAL_OK) {
//					printf("Write Protection disabled...\r\n");
//					printf("System will now restart...\r\n");
//				} else {
//					printf("Error: Flash write un-protection failed...\r\n");
//				}
//			} else {
//				if (FLASH_WriteProtectionConfig(ENABLE) == HAL_OK) {
//					printf("Write Protection enabled...\r\n");
//					printf("System will now restart...\r\n");
//				} else {
//					printf("Error: Flash write protection failed...\r\n");
//				}
//			}
//		}
//		break;
		default:{
			printf("Invalid Number ! ==> The number should be either 1, 2, 3 or 4\r");
		}
		break;
		}
	}
}

/**
 * @}
 */


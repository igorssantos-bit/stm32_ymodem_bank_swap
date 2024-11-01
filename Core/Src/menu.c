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
	printf("Waiting for the file to be sent ... (press 'a' to abort)\r\n");
	result = Ymodem_Receive(&size, BankInactive);
	if (result == COM_OK) {
		printf("\r\n Programming Completed Successfully!\r\n--------------------------------\r\n Name: %s", aFileName);
		printf("\r\n Size: %lu Bytes\r\n", size);
		printf("-------------------\n");
	} else if (result == COM_LIMIT) {
		printf("\r\nThe image size is higher than the allowed space memory!\r\n");
	} else if (result == COM_DATA) {
		printf("\n\r\nVerification failed!\r\n");
	} else if (result == COM_ABORT) {
		printf("\r\n\nAborted by user.\r\n");
	} else {
		printf("\r\nFailed to receive the file!\r\n");
	}
}

/**
 * @brief  Display the Main Menu on HyperTerminal
 * @param  None
 * @retval None
 */
void Main_Menu(void) {
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
		printf("\r\n=================== Main Menu ============================\r\n");
		printf("  Download image to the internal Flash ----------------- 1\r\n");
		printf("  Exit menu -------------------------------------------- 2\r\n");
		printf("==========================================================\r\n");
		/* Receive key */
		bool status = false;
		uint32_t t0 = HAL_GetTick();
		char buff[1];
		/* Wait for the device to be ready to use again */
		while(!status && (HAL_GetTick() - t0) < HAL_MAX_DELAY){
			status = uart_read_byte(&huart_p1, &buff);
			if(status){
				if(buff[0] == '1'){
					SerialDownload();
				}else if(buff[0] == '2'){
					printf("Exiting Main Menu...\r\n\r\n");
					return;
				}else{
					printf("Invalid Number ! ==> The number should be either 1, 2, 3 or 4\r\n");
				}
			}
		}
	}
}

/**
 * @}
 */


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
#include "common.h"
#include "flash.h"
#include "menu.h"
#include "ymodem.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t FlashProtection = 0;
uint8_t aFileName[FILE_NAME_LENGTH];
uint32_t BankActive = 0U, BankInactive = 0U;

/* Private function prototypes -----------------------------------------------*/
void SerialDownload(void);
void SerialUpload(void);

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Download a file via serial port
 * @param  None
 * @retval None
 */
void SerialDownload(void) {
	uint8_t number[11] = {0};
	uint32_t size = 0;
	COM_StatusTypeDef result;
	Serial_PutString((uint8_t *)"Waiting for the file to be sent ... (press 'a' to abort)\n\r");
	result = Ymodem_Receive(&size, BankInactive);
	if (result == COM_OK) {
		Serial_PutString((uint8_t *)"\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
		Serial_PutString(aFileName);
		Int2Str(number, size);
		Serial_PutString((uint8_t *)"\n\r Size: ");
		Serial_PutString(number);
		Serial_PutString((uint8_t *)" Bytes\r\n");
		Serial_PutString((uint8_t *)"-------------------\n");
	} else if (result == COM_LIMIT) {
		Serial_PutString((uint8_t *)"\n\n\rThe image size is higher than the allowed space memory!\n\r");
	} else if (result == COM_DATA) {
		Serial_PutString((uint8_t *)"\n\n\rVerification failed!\n\r");
	} else if (result == COM_ABORT) {
		Serial_PutString((uint8_t *)"\r\n\nAborted by user.\n\r");
	} else {
		Serial_PutString((uint8_t *)"\n\rFailed to receive the file!\n\r");
	}
}

/**
 * @brief  Upload a file via serial port.
 * @param  None
 * @retval None
 */
void SerialUpload(void) {
	uint8_t status = 0;
	Serial_PutString((uint8_t *)"\n\n\rSelect Receive File\n\r");
	HAL_UART_Receive(&hcom_uart[COM1], &status, 1, RX_TIMEOUT);
	if ( status == CRC16) {
		/* Transmit the flash image through ymodem protocol */
		status = Ymodem_Transmit((uint8_t*)FLASH_START_BANK2, (const uint8_t*)"UploadedFlashImage.bin", FLASH_BANK_SIZE);
		if (status != 0) {
			Serial_PutString((uint8_t *)"\n\rError Occurred while Transmitting File\n\r");
		} else {
			Serial_PutString((uint8_t *)"\n\rFile uploaded successfully \n\r");
		}
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
	Serial_PutString((uint8_t *)"\r\n======================================================================");
	Serial_PutString((uint8_t *)"\r\n=              (C) COPYRIGHT 2017 STMicroelectronics                 =");
	Serial_PutString((uint8_t *)"\r\n=                                                                    =");
	Serial_PutString((uint8_t *)"\r\n=   STM32U545 On-the-fly update for dual bank demo  (Version 2.0.0)  =");
	Serial_PutString((uint8_t *)"\r\n=                                                                    =");
	if (BankActive == FLASH_BANK_2){
		Serial_PutString((uint8_t *)"\r\n=                    Program running from Bank 2                     =");
	}else{
		Serial_PutString((uint8_t *)"\r\n=                    Program running from Bank 1                     =");
	}
	Serial_PutString((uint8_t *)"\r\n======================================================================");
	Serial_PutString((uint8_t *)"\r\n\r\n");
	while (1) {
		/* Test if any sector of Flash memory where user application will be loaded is write protected */
//		FlashProtection = FLASH_GetWriteProtectionStatus();
		Serial_PutString((uint8_t *)"\r\n=================== Main Menu ============================\r\n\n");
		Serial_PutString((uint8_t *)"  Download image to the internal Flash ----------------- 1\r\n\n");
//		Serial_PutString((uint8_t *)"  Upload image from the internal Flash ----------------- 2\r\n\n");
		Serial_PutString((uint8_t *)"  Exit menu -------------------------------------------- 3\r\n\n");
//		if(FlashProtection) {
//			Serial_PutString((uint8_t *)"  Disable the write protection ------------------------- 4\r\n\n");
//		} else {
//			Serial_PutString((uint8_t *)"  Enable the write protection -------------------------- 4\r\n\n");
//		}
		Serial_PutString((uint8_t *)"==========================================================\r\n\n");
		/* Clean the input path */
		__HAL_UART_FLUSH_DRREGISTER(&hcom_uart[COM1]);
		/* Receive key */
		HAL_UART_Receive(&hcom_uart[COM1], &key, 1, RX_TIMEOUT);
		switch (key) {
		case '1': {
			/* Download user application in the Flash */
			SerialDownload();
		}
		break;
//		case '2': {
//			/* Upload user application from the Flash */
//			SerialUpload();
//		}
//		break;
		case '3': {
			Serial_PutString((uint8_t *)"Press BUTTON_USER to swap Banks\r\n\n");
			return;
		}
		break;
//		case '4': {
//			if (FlashProtection) {
//				/* Disable the write protection */
//				if (FLASH_WriteProtectionConfig(DISABLE) == HAL_OK) {
//					Serial_PutString((uint8_t *)"Write Protection disabled...\r\n");
//					Serial_PutString((uint8_t *)"System will now restart...\r\n");
//				} else {
//					Serial_PutString((uint8_t *)"Error: Flash write un-protection failed...\r\n");
//				}
//			} else {
//				if (FLASH_WriteProtectionConfig(ENABLE) == HAL_OK) {
//					Serial_PutString((uint8_t *)"Write Protection enabled...\r\n");
//					Serial_PutString((uint8_t *)"System will now restart...\r\n");
//				} else {
//					Serial_PutString((uint8_t *)"Error: Flash write protection failed...\r\n");
//				}
//			}
//		}
//		break;
		default:{
			Serial_PutString((uint8_t *)"Invalid Number ! ==> The number should be either 1, 2, 3 or 4\r");
		}
		break;
		}
	}
}

/**
 * @}
 */


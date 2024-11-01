/**
 ******************************************************************************
 * @file    IAP_Main/Src/ymodem.c
 * @author  MCD Application Team
 * @brief   This file provides all the software functions related to the ymodem
 *          protocol.
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
#include "flash.h"
#include "ymodem.h"
#include "string.h"
#include "main.h"
#include "menu.h"
#include "usart.h"
#include "stdlib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC16_F       /* activate the CRC16 integrity */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit aligned */
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Receive a packet from sender
 * @param  data
 * @param  length
 *     0: end of transmission
 *     2: abort by sender
 *    >0: packet length
 * @param  timeout
 * @retval HAL_OK: normally return
 *         HAL_BUSY: abort by user
 */
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout) {
	uint32_t crc;
	uint32_t packet_size = 0;
	HAL_StatusTypeDef status;
	uint8_t char1;
	*p_length = 0;
	if (uart_read_loop(&huart_p1, &char1, 1, timeout)) {
		status = HAL_OK;
		switch (char1) {
		case SOH: {
			packet_size = PACKET_SIZE;
		}
		break;
		case STX:{
			packet_size = PACKET_1K_SIZE;
		}
		break;
		case EOT:{}
		break;
		case CA:{
			if (uart_read_loop(&huart_p1, &char1, 1, timeout) && (char1 == CA)) {
				packet_size = 2;
			} else {
				status = HAL_ERROR;
			}
		}
		break;
		case ABORT1:{}
		break;
		case ABORT2:{
			status = HAL_BUSY;
		}
		break;
		default:{
			status = HAL_ERROR;
		}
		break;
		}
		*p_data = char1;
		if (packet_size >= PACKET_SIZE) {
			/* Simple packet sanity check */
			if (uart_read_loop(&huart_p1, &p_data[PACKET_NUMBER_INDEX], packet_size + PACKET_OVERHEAD_SIZE, timeout)) {
				status = HAL_OK;
				if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE)) {
					packet_size = 0;
					status = HAL_ERROR;
				} else {
					/* Check packet CRC */
					crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
					crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];
					if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc ) {
						packet_size = 0;
						status = HAL_ERROR;
					}
				}
			} else {
				packet_size = 0;
			}
		}
	}
	*p_length = packet_size;
	return status;
}

/**
 * @brief  Update CRC16 for input byte
 * @param  crc_in input value
 * @param  input byte
 * @retval None
 */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte) {
	uint32_t crc = crc_in;
	uint32_t in = byte | 0x100;
	do {
		crc <<= 1;
		in <<= 1;
		if(in & 0x100) ++crc;
		if(crc & 0x10000) crc ^= 0x1021;
	}
	while(!(in & 0x10000));
	return crc & 0xffffu;
}

/**
 * @brief  Cal CRC16 for YModem Packet
 * @param  data
 * @param  length
 * @retval None
 */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size) {
	uint32_t crc = 0;
	const uint8_t* dataEnd = p_data + size;
	while(p_data < dataEnd) crc = UpdateCRC16(crc, *p_data++);
	crc = UpdateCRC16(crc, 0);
	crc = UpdateCRC16(crc, 0);
	return crc&0xffffu;
}

/* Public functions ---------------------------------------------------------*/
/**
 * @brief  Receive a file using the ymodem protocol with CRC16.
 * @param  p_size The size of the file.
 * @retval COM_StatusTypeDef result of reception/programming
 */
COM_StatusTypeDef Ymodem_Receive (uint32_t *p_size, uint32_t bank) {
	uint32_t i, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0, packets_received = 0;
	uint32_t flashdestination, ramsource, filesize;
	uint8_t *file_ptr;
	uint8_t file_size[FILE_SIZE_LENGTH];
	COM_StatusTypeDef result = COM_OK;
	/* Check the parameters */
	if(!IS_FLASH_BANK_EXCLUSIVE(bank)) return COM_ERROR;
	/* Initialize flashdestination variable */
	if(bank == FLASH_BANK_2){
		flashdestination = FLASH_START_BANK2;
	}else{
		flashdestination = FLASH_START_BANK1;
	}
	/* Ymodem loop */
	while ((session_done == 0) && (result == COM_OK)) {
		packets_received = 0;
		file_done = 0;
		while ((file_done == 0) && (result == COM_OK)) {
			switch (ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT)) {
			case HAL_OK:
				errors = 0;
				switch (packet_length) {
				case 2:
					/* Abort by sender */
					uart_write_byte(&huart_p1, ACK);
					result = COM_ABORT;
					break;
				case 0:
					/* End of transmission */
					uart_write_byte(&huart_p1, ACK);
					file_done = 1;
					break;
				default:
					/* Normal packet */
					if (aPacketData[PACKET_NUMBER_INDEX] != (0xFFU & packets_received)) {
						uart_write_byte(&huart_p1, NAK);
					} else {
						if (packets_received == 0) {
							/* File name packet */
							if (aPacketData[PACKET_DATA_INDEX] != 0) {
								/* File name extraction */
								i = 0;
								file_ptr = aPacketData + PACKET_DATA_INDEX;
								while ( (*file_ptr != 0) && (i < FILE_NAME_LENGTH)) {
									aFileName[i++] = *file_ptr++;
								}
								/* File size extraction */
								aFileName[i++] = '\0';
								i = 0;
								file_ptr ++;
								while ( (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH)) {
									file_size[i++] = *file_ptr++;
								}
								file_size[i++] = '\0';
								filesize = atoi((char *) &file_size);
								/* Test the size of the image to be sent */
								/* Image size is greater than Flash size */
								if (*p_size > FLASH_BANK_SIZE) {
									/* End session */
									uart_write_byte(&huart_p1, CA);
									uart_write_byte(&huart_p1, CA);
									result = COM_LIMIT;
								}
								/* erase user application area */
								FLASH_BankErase(bank);
								*p_size = filesize;
								uart_write_byte(&huart_p1, ACK);
								uart_write_byte(&huart_p1, CRC16);
							} else { /* File header packet is empty, end session */
								uart_write_byte(&huart_p1, ACK);
								file_done = 1;
								session_done = 1;
								break;
							}
						} else { /* Data packet */
							ramsource = (uint32_t) & aPacketData[PACKET_DATA_INDEX];
							/* Write received data in Flash */
							if (FLASH_Write(flashdestination, (uint32_t*) ramsource, packet_length) == FLASHIF_OK) {
								flashdestination += packet_length;
								uart_write_byte(&huart_p1, ACK);
							} else { /* An error occurred while writing to Flash memory */
								/* End session */
								uart_write_byte(&huart_p1, CA);
								uart_write_byte(&huart_p1, CA);
								result = COM_DATA;
							}
						}
						packets_received ++;
						session_begin = 1;
					}
					break;
				}
				break;
				case HAL_BUSY: /* Abort actually */
					uart_write_byte(&huart_p1, CA);
					uart_write_byte(&huart_p1, CA);
					result = COM_ABORT;
					break;
				default:
					if (session_begin > 0) {
						errors ++;
					}
					if (errors > MAX_ERRORS) {
						/* Abort communication */
						uart_write_byte(&huart_p1, CA);
						uart_write_byte(&huart_p1, CA);
					} else {
						uart_write_byte(&huart_p1, CRC16); /* Ask for a packet */
					}
					break;
			}
		}
	}
	return result;
}

/**
 * @}
 */

/*******************(C)COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/

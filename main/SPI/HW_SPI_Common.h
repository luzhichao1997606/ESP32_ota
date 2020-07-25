/*
 * @file: HW_SPI_Common.H
 * @Descripttion: 
 * @brief: 
 * @version: 
 * @author: lzc
 * @attention:
 * @Date: 2020-07-10 16:31:34
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-21 13:28:45
 */ 
#ifndef __HW_SPI_COMMON_H__
#define __HW_SPI_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_log.h"
#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   16
#define PIN_NUM_CE   15
#define nrf_tag	"NRF24L01"
extern spi_device_handle_t spi;

void Spi_init();
void CELow();
void CEHigh();
void CSLow();
void CSHigh();
uint8_t WriteBytes(uint8_t cmd, uint8_t *pBuff, uint8_t length);
uint8_t ReadBytes(uint8_t cmd, uint8_t *pBuff, uint8_t length);
uint8_t spi_transfer_byte(uint8_t byte, spi_device_handle_t device);
uint16_t SpiFlashReadID(void);
bool SpiNrfReadCheck(void);
#endif

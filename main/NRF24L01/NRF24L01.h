/*
 * @file:
 * @Descripttion:
 * @brief:
 * @version:
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-13 09:40:56
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-24 17:26:08
 */
#ifndef __NRF24L01_H__
#define __NRF24L01_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HW_SPI_Common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "freertos/queue.h"
#include "driver/gpio.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "stdlib.h"
#include "HW_SPI_Common.h"
#include "MQTT_Handler.h"
#define NRF_tag                             "NRF24L01"

/******************************************************************************/
/***                        宏定义                                          ***/
/******************************************************************************/
#define RF_LEN_ADDR     5   //TX(RX) 地址宽度 长度3：设置1，长度4：设置2，长度5：设置3
#define RF_LEN_PAYLOAD  8  //发送帧长度

#define RF_BUF_LEN      20
#define TAG_NUM_MAX     240
/* nRF24L01 Instruction Definitions */
#define WRITE_REG       0x20  /**< Register write command */
#define RD_RX_PLOAD_W   0x60  /**< Read RX payload command */
#define RD_RX_PLOAD     0x61  /**< Read RX payload command */
#define WR_TX_PLOAD     0xA0  /**< Write TX payload command */
#define WR_ACK_PLOAD    0xA8  /**< Write ACK payload command */
#define WR_NAC_TX_PLOAD 0xB0  /**< Write ACK payload command */
#define FLUSH_TX        0xE1  /**< Flush TX register command */
#define FLUSH_RX        0xE2  /**< Flush RX register command */
#define REUSE_TX_PL     0xE3  /**< Reuse TX payload command */
#define LOCK_UNLOCK     0x50  /**< Lock/unlcok exclusive features */
#define NOP             0xFF  /**< No Operation command, used for reading status register */

/* nRF24L01 Register Definitions */
#define CONFIG        0x00  /**< nRF24L01 config register */
#define EN_AA         0x01  /**< nRF24L01 enable Auto-Acknowledge register */
#define EN_RXADDR     0x02  /**< nRF24L01 enable RX addresses register */
#define SETUP_AW      0x03  /**< nRF24L01 setup of address width register */
#define SETUP_RETR    0x04  /**< nRF24L01 setup of automatic retransmission register */
#define RF_CH         0x05  /**< nRF24L01 RF channel register */
#define RF_SETUP      0x06  /**< nRF24L01 RF setup register */
#define STATUS        0x07  /**< nRF24L01 status register */
#define OBSERVE_TX    0x08  /**< nRF24L01 transmit observe register */
#define CD            0x09  /**< nRF24L01 carrier detect register */
#define RX_ADDR_P0    0x0A  /**< nRF24L01 receive address data pipe0 */
#define RX_ADDR_P1    0x0B  /**< nRF24L01 receive address data pipe1 */
#define RX_ADDR_P2    0x0C  /**< nRF24L01 receive address data pipe2 */
#define RX_ADDR_P3    0x0D  /**< nRF24L01 receive address data pipe3 */
#define RX_ADDR_P4    0x0E  /**< nRF24L01 receive address data pipe4 */
#define RX_ADDR_P5    0x0F  /**< nRF24L01 receive address data pipe5 */
#define TX_ADDR       0x10  /**< nRF24L01 transmit address */
#define RX_PW_P0      0x11  /**< nRF24L01 \# of bytes in rx payload for pipe0 */
#define RX_PW_P1      0x12  /**< nRF24L01 \# of bytes in rx payload for pipe1 */
#define RX_PW_P2      0x13  /**< nRF24L01 \# of bytes in rx payload for pipe2 */
#define RX_PW_P3      0x14  /**< nRF24L01 \# of bytes in rx payload for pipe3 */
#define RX_PW_P4      0x15  /**< nRF24L01 \# of bytes in rx payload for pipe4 */
#define RX_PW_P5      0x16  /**< nRF24L01 \# of bytes in rx payload for pipe5 */
#define FIFO_STATUS   0x17  /**< nRF24L01 FIFO status register */
#define DYNPD         0x1C  /**< nRF24L01 Dynamic payload setup */
#define FEATURE       0x1D  /**< nRF24L01 Exclusive feature setup */

/******************************************************************************/
/***                        结构体定义                                       ***/
/******************************************************************************/
#if 0 // NRF24L01+

typedef enum // bit2-bit1
{
    TX_POWER_N18DBM = 0x00, // -18dBM
    TX_POWER_N12DBM = 0x02, // -12dBM
    TX_POWER_N6DBM  = 0x04, // -6dBM
    TX_POWER_0DBM   = 0x06, // 0dBM
} en_TxPower;

#else

typedef enum // bit2-bit1
{
    TX_POWER_N12DBM = 0x00, // -12dBM
    TX_POWER_N6DBM  = 0x01, // -6dBM
    TX_POWER_N4DBM  = 0x02, // -4dBM
    TX_POWER_0DBM   = 0x03, // 0dBM
    TX_POWER_1DBM   = 0x04, // 1dBM
    TX_POWER_3DBM   = 0x05, // 3dBM
    TX_POWER_4DBM   = 0x06, // 4dBM
    TX_POWER_7DBM   = 0x07  // 7dBM
} en_TxPower;

#endif

typedef enum // bit5 bit3
{
    TX_SPEED_N1DBM = 0x00, // 1M
    TX_SPEED_2DBM  = 0x04, // 2M
    TX_SPEED_5DBM  = 0x20  // 250K
} en_TxSpeed;

typedef struct NRF24L01_DATA
{
    uint8_t NRF24L01_Buf[1200];
    uint32_t NRF24L01_Time_Count[240];
    uint16_t NRF24L01_Data_Lens ;

} NRF24L01_Data_Set;

typedef struct
{
    int         IRQ_GPIO_Num;
} NRF24L01_t;

extern int GPIO_IRQ  ;
extern uint8_t Clear_Flag ;
extern uint8_t TimeOut_Clear_Flag ;
extern NRF24L01_t dev;
extern uint8_t  DataToSendBuffer[2400] ;
extern xQueueHandle gpio_evt_queue ;

//创建二值信号量句柄
SemaphoreHandle_t xSemaphore_NRF;
extern SemaphoreHandle_t xSemaphore_NRF;
TaskHandle_t NRF_CreatedTask;
extern TaskHandle_t NRF_CreatedTask;
void nrf_spi_master_init(void);
bool nRF24L01_Check(uint8_t Register);
bool nRF24L01_2_Check(uint8_t Register);
void nrf24l01_init();
void nrf24l01_2_init();
void nRF24L01_IRQ();
void nRF24L01_IRQ_2();
void Clear_Buffer_TimeOutTask(void);
void NRF_IRQ_Handler_start_Task(void);
void Clear_ALL_nrf24l01_TempData(void);
#endif

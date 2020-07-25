/*
 * @file:HW_SPI_Common.c
 * @Descripttion:硬件SPI
 * @brief:
 * @version:1.0
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-10 16:31:19
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-21 13:29:41
 */
#include "HW_SPI_Common.h"
spi_device_handle_t spi;

/* Transfer one byte via spi and get result
 * This function is expected to be used in full duplex mode
 * Take care of setting CS pin down before calling this function
 *
 * This function is needed because native esp-idf spi master interface cannot transfer properly
 * more than 1 byte. So we are sending data byte by byte literally */
/**
 * @name: spi_transfer_byte
 * @brief: 通过spi传输一个字节并获得结果
 *此功能有望在全双工模式下使用
 *在调用此功能之前，请注意将CS引脚设置为低电平
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t spi_transfer_byte(uint8_t byte, spi_device_handle_t device)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    t.length = 8;
    t.tx_data[0] = byte;
    spi_device_transmit(device, &t);
    return t.rx_data[0];
}
/**
 * @name: Spi_init
 * @brief: SPI初始化
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void Spi_init()
{
    esp_err_t ret;
    spi_bus_config_t buscfg =
    {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    //Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    spi_device_interface_config_t devcfg =
    {
        .clock_speed_hz = 1 * 1000 * 1000,      //Clock out at 1 MHz
        .mode = 0,                              //SPI mode 0
        .spics_io_num = -1,                     //CS pin
        .queue_size = 8,                        //We want to be able to queue 7 transactions at a time
        .duty_cycle_pos = 128,
        // .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}
/**
 * @name: CELow
 * @brief: 拉低CE
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void CELow()
{
    gpio_pad_select_gpio(PIN_NUM_CE);
    gpio_set_direction(PIN_NUM_CE, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CE, 0);
}
/**
 * @name: CEHigh
 * @brief: 拉高CE
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void CEHigh()
{
    gpio_pad_select_gpio(PIN_NUM_CE);
    gpio_set_drive_capability(PIN_NUM_CE,GPIO_DRIVE_CAP_3);
    gpio_pullup_en(PIN_NUM_CE);
    gpio_set_direction(PIN_NUM_CE, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CE, 1);
}
/**
 * @name: CSLow
 * @brief: 拉低CS
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void CSLow()
{
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CS, 0);
}
/**
 * @name: CSHigh
 * @brief: 拉高CS
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void CSHigh()
{
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CS, 1);
}
/**
 * @name: WriteBytes
 * @brief: spi写数据
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t WriteBytes(uint8_t cmd, uint8_t *pBuff, uint8_t length)
{
    CSLow();
    ets_delay_us(3);
    uint8_t status = spi_transfer_byte(cmd, spi);
    while (pBuff && length--)
    {
        ets_delay_us(1);
        spi_transfer_byte(*pBuff++, spi);
    }
    ets_delay_us(3);
    CSHigh();
    return status;
}
/**
 * @name: WriteOneByte
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t WriteOneByte(uint8_t cmd)
{
    CSLow();
    ets_delay_us(5);
    uint8_t status = spi_transfer_byte(cmd, spi);
    ets_delay_us(5);
    CSHigh();
    return status;
}
/**
 * @name: ReadOneByte
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t ReadOneByte(uint8_t cmd)
{
    uint8_t Resv_Data = 0;
    CSLow();
    ets_delay_us(5);
    Resv_Data = spi_transfer_byte(cmd, spi);
    ets_delay_us(5);
    CSHigh();
    return Resv_Data ;
}
/**
 * @name: ReadBytes
 * @brief: SPI读数据
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t ReadBytes(uint8_t cmd, uint8_t *pBuff, uint8_t length)
{
    CSLow();
    ets_delay_us(5);
    uint8_t status = spi_transfer_byte(cmd, spi);
    while (pBuff && length--)
    {
        ets_delay_us(3);
        *pBuff++ = spi_transfer_byte(0xFF, spi);
    }
    ets_delay_us(5);
    CSHigh();
    return status;
}


/**
 * @name: SpiFlashReadID
 * @brief: 测试函数读取芯片ID W25Q32的ID:0XEF15
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint16_t SpiFlashReadID(void)
{
    uint8_t command = 0x90;
    uint16_t usFlashId = 0;
    uint8_t temp_buff[6] = {0};
    CSLow();
    ReadBytes(command, temp_buff, 6);
    CSHigh();
    usFlashId = (uint16_t)(temp_buff[4] << 8) | (temp_buff[3] << 0);
    return usFlashId;
}
/**
 * @name: SpiFlashReadID
 * @brief: 测试函数读取芯片ID W25Q32的ID:0XEF15
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
bool SpiNrfReadCheck(void)
{
    uint8_t command = 0x10, i;
    uint8_t buf[5] = {0X31, 0X31, 0X31, 0X31, 0X31};
    WriteBytes(command | 0X20, buf, 5);
    buf[2] = 0X39;
    ReadBytes(command, buf, 5);
    ESP_LOGI(nrf_tag, "buf[2] = %x", buf[2]);
    for (i = 0; i < 5; i++)
    {
        if (buf[i] != 0X31)
            return false;
    }
    return true;  // 检测到24L01
}

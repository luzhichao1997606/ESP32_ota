/*
 * @file:
 * @Descripttion:
 * @brief:
 * @version:
 * @author: lzc
 * @attention:
 * @Date: 2020-07-10 18:17:08
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-15 18:21:21
 */
#include "oled_hw_spi.h"
#include "oled_font.h"
static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode = 1;

static const int GPIO_MOSI = 13;
static const int GPIO_SCLK = 14;

static const int SPI_Frequency = 1000000;
/**
 * @name: spi_master_init
 * @brief: SPI初始化
 * @author: lzc
 * @param {dev} SSD1306_t结构体
 * @param {GPIO_CS} SPI的CS引脚
 * @param {GPIO_DC} SPI的DC引脚
 * @param {GPIO_RESET} SPI的RES引脚
 * @return: None
 * @note: 修改记录：初次创建
 */
void ssd1306_spi_master_init(SSD1306_t *dev, int GPIO_CS, int GPIO_DC, int GPIO_RESET)
{
    esp_err_t ret;
    gpio_pad_select_gpio(GPIO_CS);
    gpio_set_direction(GPIO_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_CS, 0);
    gpio_pad_select_gpio(GPIO_DC);
    gpio_set_direction(GPIO_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DC, 0);
    if (GPIO_RESET >= 0)
    {
        gpio_pad_select_gpio(GPIO_RESET);
        gpio_set_direction(GPIO_RESET, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_RESET, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(GPIO_RESET, 1);
    }
    spi_bus_config_t spi_bus_config =
    {
        .sclk_io_num = GPIO_SCLK,
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };
    ret = spi_bus_initialize(HSPI_HOST, &spi_bus_config, 1);
    ESP_LOGI(SSD1306_tag, "spi_bus_initialize=%d", ret);
    assert(ret == ESP_OK);
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(spi_device_interface_config_t));
    devcfg.clock_speed_hz = SPI_Frequency;
    devcfg.spics_io_num = GPIO_CS;
    devcfg.queue_size = 1;
    spi_device_handle_t handle;
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &handle);
    ESP_LOGI(SSD1306_tag, "spi_bus_add_device=%d", ret);
    assert(ret == ESP_OK);
    dev->_dc = GPIO_DC;
    dev->_SPIHandle = handle;
}
/**
 * @name: spi_master_write_byte
 * @brief: spi写数据
 * @author: lzc
 * @param {SPIHandle}  SPI处理句柄
 * @param {Data} 写入的数据
 * @param {DataLength} 数据长度
 * @return: None
 * @note: 修改记录：初次创建
 */
bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t *Data, size_t DataLength)
{
    spi_transaction_t SPITransaction;
    if (DataLength > 0)
    {
        memset(&SPITransaction, 0, sizeof(spi_transaction_t));
        SPITransaction.length = DataLength * 8;
        SPITransaction.tx_buffer = Data;
        spi_device_transmit(SPIHandle, &SPITransaction);
    }
    return true;
}
/**
 * @name: spi_master_write_command
 * @brief: SPI写指令
 * @author: lzc
 * @param {dev} SSD1306_t结构体
 * @param {Command} 指令
 * @return: None
 * @note: 修改记录：初次创建
 */
bool spi_master_write_command(SSD1306_t *dev, uint8_t Command)
{
    static uint8_t CommandByte = 0;
    CommandByte = Command;
    gpio_set_level(dev->_dc, SPI_Command_Mode);
    return spi_master_write_byte(dev->_SPIHandle, &CommandByte, 1);
}
/**
 * @name: spi_master_write_data
 * @brief: SPI写数据
 * @author: lzc
 * @param {dev} SSD1306_t结构体
 * @param {Data} 数据
 * @param {DataLength} 数据长度
 * @return: None
 * @note: 修改记录：初次创建
 */
bool spi_master_write_data(SSD1306_t *dev, const uint8_t *Data, size_t DataLength)
{
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Data, DataLength);
}
/**
 * @name: OLED_init
 * @brief: oled的初始化函数
 * @author: lzc
 * @param {dev} SSD1306_t结构体
 * @param {width} oled的像素长度
 * @param {height} oled的像素高度
 * @return: None
 * @note: 修改记录：初次创建
 */
void OLED_init(SSD1306_t *dev, int width, int height)
{
    dev->_address = SPIAddress;
    dev->_width = width;
    dev->_height = height;
    dev->_pages = 8;
    if (dev->_height == 32) dev->_pages = 4;
    spi_master_write_command(dev, OLED_CMD_DISPLAY_OFF);        // AE
    spi_master_write_command(dev, OLED_CMD_SET_MUX_RATIO);      // A8
    if (dev->_height == 64) spi_master_write_command(dev, 0x3F);
    if (dev->_height == 32) spi_master_write_command(dev, 0x1F);
    spi_master_write_command(dev, OLED_CMD_SET_DISPLAY_OFFSET); // D3
    spi_master_write_command(dev, 0x00);
    spi_master_write_command(dev, OLED_CONTROL_BYTE_DATA_STREAM);   // 40
    spi_master_write_command(dev, OLED_CMD_SET_SEGMENT_REMAP);  // A1
    spi_master_write_command(dev, OLED_CMD_SET_COM_SCAN_MODE);  // C8
    spi_master_write_command(dev, OLED_CMD_DISPLAY_NORMAL);     // A6
    spi_master_write_command(dev, OLED_CMD_SET_DISPLAY_CLK_DIV);    // D5
    spi_master_write_command(dev, 0x80);
    spi_master_write_command(dev, OLED_CMD_SET_COM_PIN_MAP);    // DA
    if (dev->_height == 64) spi_master_write_command(dev, 0x12);
    if (dev->_height == 32) spi_master_write_command(dev, 0x02);
    spi_master_write_command(dev, OLED_CMD_SET_CONTRAST);       // 81
    spi_master_write_command(dev, 0xFF);
    spi_master_write_command(dev, OLED_CMD_DISPLAY_RAM);        // A4
    spi_master_write_command(dev, OLED_CMD_SET_VCOMH_DESELCT);  // DB
    spi_master_write_command(dev, 0x40);
    spi_master_write_command(dev, OLED_CMD_SET_MEMORY_ADDR_MODE);   // 20
    //spi_master_write_command(dev, OLED_CMD_SET_HORI_ADDR_MODE);   // 00
    spi_master_write_command(dev, OLED_CMD_SET_PAGE_ADDR_MODE); // 02
    // Set Lower Column Start Address for Page Addressing Mode
    spi_master_write_command(dev, 0x00);
    // Set Higher Column Start Address for Page Addressing Mode
    spi_master_write_command(dev, 0x10);
    spi_master_write_command(dev, OLED_CMD_SET_CHARGE_PUMP);    // 8D
    spi_master_write_command(dev, 0x14);
    spi_master_write_command(dev, OLED_CMD_DEACTIVE_SCROLL);    // 2E
    spi_master_write_command(dev, OLED_CMD_DISPLAY_NORMAL);     // A6
    spi_master_write_command(dev, OLED_CMD_DISPLAY_ON);     // AF
}
/**
 * @name: spi_contrast
 * @brief: 
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void spi_contrast(SSD1306_t *dev, int contrast)
{
    int _contrast = contrast;
    if (contrast < 0x0) _contrast = 0;
    if (contrast > 0xFF) _contrast = 0xFF;
    spi_master_write_command(dev, OLED_CMD_SET_CONTRAST);           // 81
    spi_master_write_command(dev, _contrast);
}
void spi_hardware_scroll(SSD1306_t *dev, ssd1306_scroll_type_t scroll)
{
    if (scroll == SCROLL_RIGHT)
    {
        spi_master_write_command(dev, OLED_CMD_HORIZONTAL_RIGHT);   // 26
        spi_master_write_command(dev, 0x00); // Dummy byte
        spi_master_write_command(dev, 0x00); // Define start page address
        spi_master_write_command(dev, 0x07); // Frame frequency
        spi_master_write_command(dev, 0x07); // Define end page address
        spi_master_write_command(dev, 0x00); //
        spi_master_write_command(dev, 0xFF); //
        spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);      // 2F
    }
    if (scroll == SCROLL_LEFT)
    {
        spi_master_write_command(dev, OLED_CMD_HORIZONTAL_LEFT);    // 27
        spi_master_write_command(dev, 0x00); // Dummy byte
        spi_master_write_command(dev, 0x00); // Define start page address
        spi_master_write_command(dev, 0x07); // Frame frequency
        spi_master_write_command(dev, 0x07); // Define end page address
        spi_master_write_command(dev, 0x00); //
        spi_master_write_command(dev, 0xFF); //
        spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);      // 2F
    }
    if (scroll == SCROLL_DOWN)
    {
        spi_master_write_command(dev, OLED_CMD_CONTINUOUS_SCROLL);  // 29
        spi_master_write_command(dev, 0x00); // Dummy byte
        spi_master_write_command(dev, 0x00); // Define start page address
        spi_master_write_command(dev, 0x07); // Frame frequency
        //spi_master_write_command(dev, 0x01); // Define end page address
        spi_master_write_command(dev, 0x00); // Define end page address
        spi_master_write_command(dev, 0x3F); // Vertical scrolling offset
        spi_master_write_command(dev, OLED_CMD_VERTICAL);       // A3
        spi_master_write_command(dev, 0x00);
        if (dev->_height == 64)
            spi_master_write_command(dev, 0x40);
        if (dev->_height == 32)
            spi_master_write_command(dev, 0x20);
        spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);      // 2F
    }
    if (scroll == SCROLL_UP)
    {
        spi_master_write_command(dev, OLED_CMD_CONTINUOUS_SCROLL);  // 29
        spi_master_write_command(dev, 0x00); // Dummy byte
        spi_master_write_command(dev, 0x00); // Define start page address
        spi_master_write_command(dev, 0x07); // Frame frequency
        //spi_master_write_command(dev, 0x01); // Define end page address
        spi_master_write_command(dev, 0x00); // Define end page address
        spi_master_write_command(dev, 0x01); // Vertical scrolling offset
        spi_master_write_command(dev, OLED_CMD_VERTICAL);       // A3
        spi_master_write_command(dev, 0x00);
        if (dev->_height == 64)
            spi_master_write_command(dev, 0x40);
        if (dev->_height == 32)
            spi_master_write_command(dev, 0x20);
        spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);      // 2F
    }
    if (scroll == SCROLL_STOP)
    {
        spi_master_write_command(dev, OLED_CMD_DEACTIVE_SCROLL);    // 2E
    }
}
/**
 * @name: spi_display_image
 * @brief: spi驱动显示图片
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void spi_display_image(SSD1306_t *dev, int page, int seg, uint8_t *images, int width)
{
    if (page >= dev->_pages) return;
    if (seg >= dev->_width) return;
    uint8_t columLow = seg & 0x0F;
    uint8_t columHigh = (seg >> 4) & 0x0F;
    // Set Lower Column Start Address for Page Addressing Mode
    //i2c_master_write_byte(cmd, 0x00, true);
    spi_master_write_command(dev, (0x00 + columLow));
    // Set Higher Column Start Address for Page Addressing Mode
    //spi_master_write_command(dev, 0x10, 1);
    spi_master_write_command(dev, (0x10 + columHigh));
    // Set Page Start Address for Page Addressing Mode
    spi_master_write_command(dev, 0xB0 | page);
    spi_master_write_data(dev, images, width);
}
/**
 * @name: spi_display_text
 * @brief: spi驱动显示文本
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void spi_display_text(SSD1306_t *dev, int page, char *text, int text_len, bool invert)
{
    if (page >= dev->_pages) return;
    int _text_len = text_len;
    if (_text_len > 16) _text_len = 16;
    uint8_t seg = 0;
    uint8_t image[8];
    for (uint8_t i = 0; i < _text_len; i++)
    {
        memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
        if (invert) ssd1306_invert(image, 8);
        spi_display_image(dev, page, seg, image, 8);
#if 0
        for (int j = 0; j < 8; j++)
            dev->_page[page]._segs[seg + j] = image[j];
#endif
        seg = seg + 8;
    }
}
/**
 * @name: ssd1306_display_text
 * @brief: ssd1306显示文本
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void ssd1306_display_text(SSD1306_t *dev, int page, char *text, int text_len, bool invert)
{
    if (dev->_address == SPIAddress)
    {
        spi_display_text(dev, page, text, text_len, invert);
    }
}
/**
 * @name: ssd1306_clear_screen
 * @brief: ssd1306清除屏幕
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void ssd1306_clear_screen(SSD1306_t *dev, bool invert)
{
    void (*func)(SSD1306_t *dev, int page, char *text, int text_len, bool invert);
    if (dev->_address == SPIAddress)
    {
        func = spi_display_text;
    }
    char zero[128];
    memset(zero, 0, sizeof(zero));
    for (int page = 0; page < dev->_pages; page++)
    {
        (*func)(dev, page, zero, 128, invert);
    }
}
void ssd1306_contrast(SSD1306_t *dev, int contrast)
{
    if (dev->_address == SPIAddress)
    {
        spi_contrast(dev, contrast);
    }
}
/**
 * @name: ssd1306_invert
 * @brief: ssd1306翻转
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void ssd1306_invert(uint8_t *buf, size_t blen)
{
    uint8_t wk;
    for (int i = 0; i < blen; i++)
    {
        wk = buf[i];
        buf[i] = ~wk;
    }
}
/**
 * @name: ssd1306_clear_line
 * @brief: 清除行
 * @author: lzc
 * @param {SSD1306_t} 结构体
 * @param {page} 将oled分为8个区域以显示。
 * @param {invert}是否反色
 * @return: None
 * @note: 修改记录：初次创建
 */
void ssd1306_clear_line(SSD1306_t *dev, int page, bool invert)
{
    void (*func)(SSD1306_t *dev, int page, char *text, int text_len, bool invert);
    if (dev->_address == SPIAddress)
    {
        func = spi_display_text;
    }
    char zero[128];
    memset(zero, 0, sizeof(zero));
    (*func)(dev, page, zero, 128, invert);
}
/**
 * @name: ssd1306_display_image
 * @brief: ssd1306显示图像
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void ssd1306_display_image(SSD1306_t *dev, int page, int seg, uint8_t *images, int width)
{
    if (dev->_address == SPIAddress)
    {
        spi_display_image(dev, page, seg, images, width);
    }
}
void ssd1306_software_scroll(SSD1306_t *dev, int start, int end)
{
    ESP_LOGD(SSD1306_tag, "software_scroll start=%d end=%d _pages=%d", start, end, dev->_pages);
    if (start < 0 || end < 0)
    {
        dev->_scEnable = false;
    }
    else if (start >= dev->_pages || end >= dev->_pages)
    {
        dev->_scEnable = false;
    }
    else
    {
        dev->_scEnable = true;
        dev->_scStart = start;
        dev->_scEnd = end;
        dev->_scDirection = 1;
        if (start > end) dev->_scDirection = -1;
        for (int i = 0; i < dev->_pages; i++)
        {
            dev->_page[i]._valid = false;
            dev->_page[i]._segLen = 0;
        }
    }
}
void ssd1306_scroll_text(SSD1306_t *dev, char *text, int text_len, bool invert)
{
    ESP_LOGD(SSD1306_tag, "dev->_scEnable=%d", dev->_scEnable);
    if (dev->_scEnable == false) return;
    void (*func)(SSD1306_t *dev, int page, int seg, uint8_t *images, int width);
    if (dev->_address == SPIAddress)
    {
        func = spi_display_image;
    }
    int srcIndex = dev->_scEnd - dev->_scDirection;
    while (1)
    {
        int dstIndex = srcIndex + dev->_scDirection;
        ESP_LOGD(SSD1306_tag, "srcIndex=%d dstIndex=%d", srcIndex, dstIndex);
        dev->_page[dstIndex]._valid = dev->_page[srcIndex]._valid;
        dev->_page[dstIndex]._segLen = dev->_page[srcIndex]._segLen;
        for (int seg = 0; seg < dev->_width; seg++)
        {
            dev->_page[dstIndex]._segs[seg] = dev->_page[srcIndex]._segs[seg];
        }
        ESP_LOGD(SSD1306_tag, "_valid=%d", dev->_page[dstIndex]._valid);
        if (dev->_page[dstIndex]._valid)(*func)(dev, dstIndex, 0, dev->_page[dstIndex]._segs, dev->_page[srcIndex]._segLen);
        if (srcIndex == dev->_scStart) break;
        srcIndex = srcIndex - dev->_scDirection;
    }
    int _text_len = text_len;
    if (_text_len > 16) _text_len = 16;
    uint8_t seg = 0;
    uint8_t image[8];
    for (uint8_t i = 0; i < _text_len; i++)
    {
        memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
        if (invert) ssd1306_invert(image, 8);
        (*func)(dev, srcIndex, seg, image, 8);
        for (int j = 0; j < 8; j++) dev->_page[srcIndex]._segs[seg + j] = image[j];
        seg = seg + 8;
    }
    dev->_page[srcIndex]._valid = true;
    dev->_page[srcIndex]._segLen = seg;
}
void ssd1306_scroll_clear(SSD1306_t *dev)
{
    ESP_LOGD(SSD1306_tag, "dev->_scEnable=%d", dev->_scEnable);
    if (dev->_scEnable == false) return;
    int srcIndex = dev->_scEnd - dev->_scDirection;
    while (1)
    {
        int dstIndex = srcIndex + dev->_scDirection;
        ESP_LOGD(SSD1306_tag, "srcIndex=%d dstIndex=%d", srcIndex, dstIndex);
        ssd1306_clear_line(dev, dstIndex, false);
        dev->_page[dstIndex]._valid = false;
        if (dstIndex == dev->_scStart) break;
        srcIndex = srcIndex - dev->_scDirection;
    }
}
void ssd1306_hardware_scroll(SSD1306_t *dev, ssd1306_scroll_type_t scroll)
{
    if (dev->_address == SPIAddress)
    {
        spi_hardware_scroll(dev, scroll);
    }
}
/**
 * @name: ssd1306_fadeout
 * @brief: 清除屏幕
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void ssd1306_fadeout(SSD1306_t *dev)
{
    void (*func)(SSD1306_t *dev, int page, int seg, uint8_t *images, int width);
    if (dev->_address == SPIAddress)
    {
        func = spi_display_image;
    }
    uint8_t image[1];
    for (int page = 0; page < dev->_pages; page++)
    {
        image[0] = 0xFF;
        for (int line = 0; line < 8; line++)
        {
            image[0] = image[0] << 1;
            for (int seg = 0; seg < 128; seg++)
            {
                (*func)(dev, page, seg, image, 1);
            }
        }
    }
}
/**
 * @name: ssd1306_dump
 * @brief: 打印基础信息
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void ssd1306_dump(SSD1306_t dev)
{
    printf("_address=%x\n", dev._address);
    printf("_width=%x\n", dev._width);
    printf("_height=%x\n", dev._height);
    printf("_pages=%x\n", dev._pages);
}


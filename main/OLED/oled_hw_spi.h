/*
 * @file:
 * @Descripttion:
 * @brief:
 * @version:
 * @author: lzc
 * @attention: 娉ㄦ剰锛�
 * @Date: 2020-07-10 18:17:20
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-15 18:21:07
 */
#ifndef __OLED_HW_SPI_H__
#define __OLED_HW_SPI_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HW_SPI_Common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "stdlib.h"

#include "oled_font.h"
// Following definitions are bollowed from
// http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html

/* Control byte
Co : bit 8 : Continuation Bit
 * 1 = no-continuation (only one byte to follow)
 * 0 = the controller should expect a stream of bytes.
D/C# : bit 7 : Data/Command Select bit
 * 1 = the next byte or byte stream will be Data.
 * 0 = a Command byte or byte stream will be coming up next.
 Bits 6-0 will be all zeros.
Usage:
0x80 : Single Command byte
0x00 : Command Stream
0xC0 : Single Data byte
0x40 : Data Stream
*/
//例子
/*
SSD1306_t dev;
    int center, top, bottom;
    char lineChar[20];
    top = 2;
    center = 3;
    bottom = 8;
    ESP_LOGI(SPI, "SPI_START");
    ssd1306_spi_master_init(&dev, 25, 26, 27);
    OLED_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "SSD1306 128x64", 14, false);
    ssd1306_display_text(&dev, 1, "ABCDEFGHIJKLMNOP", 16, false);
    ssd1306_display_text(&dev, 2, "abcdefghijklmnop", 16, false);
    ssd1306_display_text(&dev, 3, "Hello World!!", 13, false);
    ssd1306_clear_line(&dev, 4, true);
    ssd1306_clear_line(&dev, 5, true);
    ssd1306_clear_line(&dev, 6, true);
    ssd1306_clear_line(&dev, 7, true);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    // Display Count Down
    uint8_t image[24];
    memset(image, 0, sizeof(image));
    ssd1306_display_image(&dev, top, (6 * 8 - 1), image, sizeof(image));
    ssd1306_display_image(&dev, top + 1, (6 * 8 - 1), image, sizeof(image));
    ssd1306_display_image(&dev, top + 2, (6 * 8 - 1), image, sizeof(image));
    for (int font = 0x39; font > 0x30; font--)
    {
        memset(image, 0, sizeof(image));
        ssd1306_display_image(&dev, top + 1, (7 * 8 - 1), image, 8);
        memcpy(image, font8x8_basic_tr[font], 8);
        ssd1306_display_image(&dev, top + 1, (7 * 8 - 1), image, 8);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    // Scroll Up
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "---Scroll  UP---", 16, true);
    ssd1306_software_scroll(&dev, 7, 1);
    for (int line = 0; line < bottom + 10; line++)
    {
        lineChar[0] = 0x01;
        sprintf(&lineChar[1], " Line %02d", line);
        ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    // Scroll Down
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "--Scroll  DOWN--", 16, true);
    ssd1306_software_scroll(&dev, 1, 7);
    for (int line = 0; line < bottom + 10; line++)
    {
        lineChar[0] = 0x02;
        sprintf(&lineChar[1], " Line %02d", line);
        ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    // Page Down
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "---Page  DOWN---", 16, true);
    ssd1306_software_scroll(&dev, 1, 7);
    for (int line = 0; line < bottom + 10; line++)
    {
        if ((line % 7) == 0) ssd1306_scroll_clear(&dev);
        lineChar[0] = 0x02;
        sprintf(&lineChar[1], " Line %02d", line);
        ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    // Horizontal Scroll
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, center, "Horizontal", 10, false);
    ssd1306_hardware_scroll(&dev, SCROLL_RIGHT);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ssd1306_hardware_scroll(&dev, SCROLL_LEFT);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ssd1306_hardware_scroll(&dev, SCROLL_STOP);
    // Vertical Scroll
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, center, "Vertical", 8, false);
    ssd1306_hardware_scroll(&dev, SCROLL_DOWN);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ssd1306_hardware_scroll(&dev, SCROLL_UP);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ssd1306_hardware_scroll(&dev, SCROLL_STOP);
    // Invert
    ssd1306_clear_screen(&dev, true);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, center, "  Good Bye!!", 12, true);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    // Fade Out
    ssd1306_fadeout(&dev);
*/
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_SINGLE   0xC0
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20
#define OLED_CMD_SET_HORI_ADDR_MODE     0x00    // Horizontal Addressing Mode
#define OLED_CMD_SET_VERT_ADDR_MODE     0x01    // Vertical Addressing Mode
#define OLED_CMD_SET_PAGE_ADDR_MODE     0x02    // Page Addressing Mode
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

// Scrolling Command
#define OLED_CMD_HORIZONTAL_RIGHT       0x26
#define OLED_CMD_HORIZONTAL_LEFT        0x27
#define OLED_CMD_CONTINUOUS_SCROLL      0x29
#define OLED_CMD_DEACTIVE_SCROLL        0x2E
#define OLED_CMD_ACTIVE_SCROLL          0x2F
#define OLED_CMD_VERTICAL               0xA3
#define SPIAddress                      0xFF
#define Max_Column                      28
#define SSD1306_tag                             "SSD1306"
typedef enum
{
    SCROLL_RIGHT = 1,
    SCROLL_LEFT = 2,
    SCROLL_DOWN = 3,
    SCROLL_UP = 4,
    SCROLL_STOP = 5
} ssd1306_scroll_type_t;

typedef struct
{
    bool _valid;
    int _segLen; // 0-128
    uint8_t _segs[128];
} PAGE_t;

typedef struct
{
    int _address;
    int _width;
    int _height;
    int _pages;
    int _dc;
    spi_device_handle_t _SPIHandle;
    bool _scEnable;
    int _scStart;
    int _scEnd;
    int _scDirection;
    PAGE_t _page[8];
} SSD1306_t;



void OLED_init(SSD1306_t *dev, int width, int height);
void ssd1306_spi_master_init(SSD1306_t *dev, int GPIO_CS, int GPIO_DC, int GPIO_RESET);

void ssd1306_display_text(SSD1306_t *dev, int page, char *text, int text_len, bool invert);
void ssd1306_clear_screen(SSD1306_t *dev, bool invert);
void ssd1306_contrast(SSD1306_t *dev, int contrast);
void ssd1306_display_image(SSD1306_t *dev, int page, int seg, uint8_t *images, int width);
void ssd1306_clear_line(SSD1306_t *dev, int page, bool invert);
void ssd1306_software_scroll(SSD1306_t *dev, int start, int end);
void ssd1306_scroll_text(SSD1306_t *dev, char *text, int text_len, bool invert);
void ssd1306_scroll_clear(SSD1306_t *dev);
void ssd1306_hardware_scroll(SSD1306_t *dev, ssd1306_scroll_type_t scroll);
void ssd1306_invert(uint8_t *buf, size_t blen);
void ssd1306_fadeout(SSD1306_t *dev);
void ssd1306_dump(SSD1306_t dev);
#endif

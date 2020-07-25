/*
 * @file:
 * @Descripttion:
 * @brief:
 * @version:
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-13 09:40:43
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-25 13:58:33
 */
#include "NRF24L01.h"


#define ESP_INTR_FLAG_DEFAULT   0
#define DEVICE_COUNT            240
//My SPI GPIO
//static const int GPIO_MISO = 25;
//static const int GPIO_MOSI = 23;
//static const int GPIO_SCLK = 19;
//int GPIO_IRQ  = 22;
//DEFINE
static const int GPIO_MISO = 12;
static const int GPIO_MOSI = 13;
static const int GPIO_SCLK = 14;
static const int PIN_NUM_CS_2 = 17;
int GPIO_IRQ   = 34;//RF1
int GPIO_IRQ2  = 39;//RF2
//// #define PIN_NUM_CS   26
//// #define PIN_NUM_CE   21
static const int SPI_Frequency = 1000000;
spi_device_handle_t _SPIHandle;
uint8_t ADDR_Save_Data = 1 ;
uint8_t DataToSendBuffer[2400] ;
uint8_t Buf[RF_BUF_LEN];
xQueueHandle gpio_evt_queue = NULL;
xQueueHandle gpio_data_queue = NULL;
const uint8_t BusAddr[6] = {0x57, 0x53, 0x4E, 0x52, 0x52, 0x52};
/* 任务句柄，操作此句柄前一定要保证相应任务已经创建 */
static TaskHandle_t xHandleTaskMsgPro = NULL;
uint8_t ReadReg(uint8_t reg);
void NRF_ALLReflash_Channel(void);
uint8_t nrf_spi_master_read_bytes(uint8_t cmd, uint8_t *pBuff, uint8_t length);
/***********************结构体和联合体的实例化*****************************/

NRF24L01_Data_Set   NRF_Data_Poll_Struct;
/**
 * @name: HexToStr
 * @brief: 将16进制数转化为字符串
 * @param {uint8_t}    [OUT] pbDest - 存放目标字符串
 *                  [IN] pbSrc - 输入16进制数的起始地址
 *                  [IN] nLen - 16进制数的字节数
 * @return:
 * @note: 修改记录：初次创建
 */
void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen)
{
    char ddl, ddh;
    int i;
    for (i = 0; i < nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i * 2] = ddh;
        pbDest[i * 2 + 1] = ddl;
    }
    pbDest[nLen * 2] = '\0';
}
/**
 * @name: comCalCRC
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t comCalCRC(uint8_t *pbuf, uint16_t u16Len)
{
    unsigned char u8CRC = 0;
    while (u16Len--)
    {
        u8CRC += *pbuf++;
    }
    return u8CRC;
}
/**
 * @name: CSLow
 * @brief: 拉低CS
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void CS2Low()
{
    gpio_set_direction(PIN_NUM_CS_2, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CS_2, 0);
}
/**
 * @name: CSHigh
 * @brief: 拉高CS
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void CS2High()
{
    gpio_set_direction(PIN_NUM_CS_2, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CS_2, 1);
}
/**
 * @name:
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    int gpio_num = (int)(arg);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* 发送任务通知 */
    vTaskNotifyGiveFromISR(xHandleTaskMsgPro, &xHigherPriorityTaskWoken);
    /* 如果 xHigherPriorityTaskWoken = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
/**
 * @name:
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void NRF_IRQ_Handler_Poll_Task(void)
{
    int io_num;
    uint32_t ulNotificationValue;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
    /* Wait for the transmission to complete. */
    while (true)
    {
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        if (ulNotificationValue == 1)
        {
            /* The transmission ended as expected. */
            if (xQueueReceive(gpio_evt_queue, &io_num, 100))
            {
                if (io_num == GPIO_IRQ)
                {
                    nRF24L01_IRQ();
                }
                else if (io_num == GPIO_IRQ2)
                {
                    nRF24L01_IRQ_2();
                }
            }
        }
        else
        {
            /* The call to ulTaskNotifyTake() timed out. */
        }
    }
}
/**
 * @name:NRF24l01_Init_Proc
 * @brief:NRF24l01 初始化错误处理
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
bool Status ;
uint8_t u8RF_Init_Num = 0;
uint8_t u8RF_2_Init_Num = 0;
void NRF24l01_Init_Proc(void)
{
    Status = nRF24L01_Check(TX_ADDR);
    printf("Status 1 = %d\r\n", Status);
    //NRF1检测
    while (!Status)
    {
        nrf24l01_init();
        u8RF_Init_Num++;
        if (u8RF_Init_Num > 10)
        {
            //24L01校验失败，复位模块
            printf("Restarting now.\n");
            fflush(stdout);
            esp_restart();
        }
        Status = nRF24L01_Check(TX_ADDR);
    }
    //NRF2检测
    Status = nRF24L01_2_Check(TX_ADDR);
    printf("Status 2 = %d\r\n", Status);
    while (!Status)
    {
        nrf24l01_2_init();
        u8RF_2_Init_Num++;
        if (u8RF_2_Init_Num > 10)
        {
            //24L01校验失败，复位模块
            printf("Restarting now.\n");
            fflush(stdout);
            esp_restart();
        }
        Status = nRF24L01_2_Check(TX_ADDR);
    }
}
/**
 * @name:
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void NRF_IRQ_Handler_start_Task(void)
{
    nrf_spi_master_init();
    nrf24l01_init();
    nrf24l01_2_init();
    NRF24l01_Init_Proc();
    //建立用于任务通知的任务
    //xTaskCreate(NRF_IRQ_Handler_Poll_Task, "NRF_IRQ_Task", 4096, NULL, 2, &xHandleTaskMsgPro);
    xTaskCreatePinnedToCore((void *)NRF_IRQ_Handler_Poll_Task, "NRF_IRQ_Task", 4096, NULL, 2,
                            &xHandleTaskMsgPro, 1);
    while (1)
    {
        Clear_Buffer_TimeOutTask();         //清除Buffer数据
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
/**
 * @name: nrf_IRQ_Pin_Set
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void nrf_IRQ_Pin_Set(int GPIO_IRQ_Pin)
{
    gpio_config_t io_conf;
    //interrupt of falling edge
    io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = (1ULL << GPIO_IRQ_Pin) ;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    printf("Get Handler \r\n");
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_IRQ_Pin, gpio_isr_handler, (void *)(GPIO_IRQ_Pin));
}
/**
* @name: nrf_spi_master_init
* @brief: SPI初始化
* @author: lzc
* @param {dev} NRF24L01_t 结构体
* @param {GPIO_CS_Pin} SPI的CS引脚
* @param {GPIO_CE_Pin} SPI的DC引脚
* @param {GPIO_IRQ_Pin} SPI的RES引脚
* @return: None
* @note: 修改记录：初次创建
*/
void nrf_spi_master_init(void)
{
    esp_err_t ret;
    spi_bus_config_t spi_bus_config =
    {
        .sclk_io_num = GPIO_SCLK,
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };
    ret = spi_bus_initialize(HSPI_HOST, &spi_bus_config, 1);
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(NRF_tag, "spi_bus_initialize=%d", ret);
    assert(ret == ESP_OK);
    //Initialize the SPI bus
    spi_device_interface_config_t devcfg =
    {
        .clock_speed_hz = SPI_Frequency,      //Clock out at 1 MHz
        .mode = 0,                              //SPI mode 0
        .spics_io_num = -1,                     //CS pin
        .queue_size = 8,                        //We want to be able to queue 7 transactions at a time
        .duty_cycle_pos = 128,
        // .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Attach the LCD to the SPI bus
    spi_device_handle_t handle;
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &handle);
    ESP_ERROR_CHECK(ret);
    assert(ret == ESP_OK);
    _SPIHandle = handle;
}
/**
 * @name: nrf_spi_master_write_bytes
 * @brief: spi写数据
 * @author: lzc
 * @param {cmd}  命令
 * @param {*pBuff} 写入的数据
 * @param {length} 数据长度
 * @return: None
 * @note: 修改记录：初次创建 
 */
bool nrf_spi_master_write_bytes(uint8_t cmd, uint8_t *pBuff, uint8_t length)
{
    CSLow();
    ets_delay_us(3);
    uint8_t status = spi_transfer_byte(cmd | WRITE_REG, _SPIHandle);
    while (pBuff && length--)
    {
        ets_delay_us(1);
        spi_transfer_byte(*pBuff++, _SPIHandle);
    }
    ets_delay_us(3);
    CSHigh();
    return status;
}
/**
 * @name: nrf_spi_master_read_bytes
 * @brief: spi读取数据
 * @author: lzc
 * @param {SPIHandle}  SPI处理句柄
 * @param {Data} 写入的数据
 * @param {DataLength} 数据长度
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t nrf_spi_master_read_bytes(uint8_t cmd, uint8_t *pBuff, uint8_t length)
{
    CSLow();
    ets_delay_us(3);
    uint8_t status = spi_transfer_byte(cmd, _SPIHandle);
    while (pBuff && length--)
    {
        ets_delay_us(1);
        *pBuff++ = spi_transfer_byte(0x00, _SPIHandle);
    }
    ets_delay_us(3);
    CSHigh();
    return status;
}
/**
 * @name: nrf_spi_master_write_bytes_2
 * @brief: spi写数据
 * @author: lzc
 * @param {SPIHandle}  SPI处理句柄
 * @param {Data} 写入的数据
 * @param {DataLength} 数据长度
 * @return: None
 * @note: 修改记录：初次创建
 */
bool nrf_spi_master_write_bytes_2(uint8_t cmd, uint8_t *pBuff, uint8_t length)
{
    CS2Low();
    ets_delay_us(3);
    uint8_t status = spi_transfer_byte(cmd | WRITE_REG, _SPIHandle);
    while (pBuff && length--)
    {
        ets_delay_us(1);
        spi_transfer_byte(*pBuff++, _SPIHandle);
    }
    ets_delay_us(3);
    CS2High();
    return status;
}
/**
 * @name: nrf_spi_master_read_bytes_2
 * @brief: spi读取数据
 * @author: lzc
 * @param {SPIHandle}  SPI处理句柄
 * @param {Data} 写入的数据
 * @param {DataLength} 数据长度
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t nrf_spi_master_read_bytes_2(uint8_t cmd, uint8_t *pBuff, uint8_t length)
{
    CS2Low();
    ets_delay_us(3);
    uint8_t status = spi_transfer_byte(cmd, _SPIHandle);
    while (pBuff && length--)
    {
        ets_delay_us(1);
        *pBuff++ = spi_transfer_byte(0x00, _SPIHandle);
    }
    ets_delay_us(3);
    CS2High();
    return status;
}
/**
 * @name: ReadReg
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t ReadReg(uint8_t reg)
{
    uint8_t result;
    nrf_spi_master_read_bytes(reg, &result, 1);
    return result;
}
/**
 * @name: WriteReg
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t WriteReg(uint8_t reg, uint8_t value)
{
    return nrf_spi_master_write_bytes(reg, &value, 1);
}
/**
 * @name: ReadReg_2
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t ReadReg_2(uint8_t reg)
{
    uint8_t result;
    nrf_spi_master_read_bytes_2(reg, &result, 1);
    return result;
}
/**
 * @name: WriteReg_2
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t WriteReg_2(uint8_t reg, uint8_t value)
{
    return nrf_spi_master_write_bytes_2(reg, &value, 1);
}
/**
 * @name: nRF24L01_Check
 * @brief: 检测24L01——1是否存在
 * @param {type} None
 * @return: {uint8_t} 返回值:TRUE，成功; FALSE，失败
 * @note: 修改记录：初次创建
 */
bool nRF24L01_Check(uint8_t Register)
{
    uint8_t i;
    uint8_t buf[5] = {0X31, 0X31, 0X31, 0X31, 0X31};
    nrf_spi_master_write_bytes(Register, buf, 5); //写入5个字节的地址.
    buf[2] = 0X39;
    nrf_spi_master_read_bytes(Register, buf, 5); //读出写入的地址
    ESP_LOGI(NRF_tag, "buf = %x", buf[2]);
    for (i = 0; i < 5; i++)
    {
        if (buf[i] != 0X31)
            return false;
    }
    return true;  // 检测到24L01
}
/**
 * @name: nRF24L01_2_Check
 * @brief: 检测24L01——2是否存在
 * @param {type} None
 * @return: {uint8_t} 返回值:TRUE，成功; FALSE，失败
 * @note: 修改记录：初次创建
 */
bool nRF24L01_2_Check(uint8_t Register)
{
    uint8_t i;
    uint8_t buf[5] = {0X31, 0X31, 0X31, 0X31, 0X31};
    nrf_spi_master_write_bytes_2(Register, buf, 5); //写入5个字节的地址.
    buf[2] = 0X39;
    nrf_spi_master_read_bytes_2(Register, buf, 5); //读出写入的地址
    ESP_LOGI(NRF_tag, "buf = %x", buf[2]);
    for (i = 0; i < 5; i++)
    {
        if (buf[i] != 0X31)
            return false;
    }
    return true;  // 检测到24L01
}
/**
 * @name: nRF24L01_EnterRxMode
 * @brief:  进入接收模式
 * @author: lzc
 * @param {SPIHandle} spi_device_handle_t结构体
 * @return: None
 * @note: 修改记录：初次创建
 */
void nRF24L01_EnterRxMode()
{
    uint8_t u8channel;
    CELow();
    u8channel = (((ADDR_Save_Data - 1) % 2 == 0) ?
                 (2 * (ADDR_Save_Data - 1)) : (2 * (ADDR_Save_Data - 1) - 1));
    WriteReg(RF_CH, u8channel);          // 选择射频工作频道0(0-127)
    nrf_spi_master_write_bytes(RX_ADDR_P0, BusAddr, RF_LEN_ADDR);   // 设置通道0接收地址
    WriteReg(FLUSH_TX, 0);    // 清除TX FIFO寄存器
    WriteReg(FLUSH_RX, 0);    // 清除RX FIFO寄存器
    WriteReg(STATUS, 0x70);   // 清中断标志
    WriteReg(CONFIG, 0x0F);   // 使能接收模式
    CEHigh();
}
/**
 * @name: nRF24L01_2_EnterRxMode
 * @brief: 进入接收模式
 * @author: lzc
 * @param {SPIHandle} spi_device_handle_t结构体
 * @return: None
 * @note: 修改记录：初次创建
 */
void nRF24L01_2_EnterRxMode()
{
    uint8_t u8channel;
    CELow();
    u8channel = (((ADDR_Save_Data - 1) % 2 == 0) ?
                 (2 * (ADDR_Save_Data - 1) + 2) : (2 * (ADDR_Save_Data - 1) + 1));
    printf("u8channel = %d\r\n", u8channel);
    WriteReg_2(RF_CH, u8channel);          // 选择射频工作频道0(0-127)
    nrf_spi_master_write_bytes_2(RX_ADDR_P0, BusAddr, RF_LEN_ADDR);   // 设置通道0接收地址
    WriteReg_2(FLUSH_TX, 0);    // 清除TX FIFO寄存器
    WriteReg_2(FLUSH_RX, 0);    // 清除RX FIFO寄存器
    WriteReg_2(STATUS, 0x70);   // 清中断标志
    WriteReg_2(CONFIG, 0x0F);   // 使能接收模式
    CEHigh();
}
// nRF24L01+发送接口
void nRF24L01_Tx(const uint8_t *dstAddr, const uint8_t *pbuf, uint8_t u8Len)
{
    uint8_t u8channel;
    u8channel = (((ADDR_Save_Data - 1) % 2 == 0) ?
                 (2 * (ADDR_Save_Data - 1)) : (2 * (ADDR_Save_Data - 1) - 1));
    CELow();
    WriteReg(RF_CH, u8channel);          // 选择射频工作频道0(0-127)
    WriteReg(FLUSH_TX, 0);    // 清除TX FIFO寄存器
    WriteReg(FLUSH_RX, 0);    // 清除RX FIFO寄存器
    nrf_spi_master_write_bytes(TX_ADDR,    dstAddr, RF_LEN_ADDR);  // 设置目标地址
    nrf_spi_master_write_bytes(RX_ADDR_P0, dstAddr, RF_LEN_ADDR);  // 设置接收ACk地址
    //WriteReg(WR_TX_PLOAD, pbuf, RF_LEN_PAYLOAD); // 需要ACK
    nrf_spi_master_write_bytes(WR_NAC_TX_PLOAD, pbuf, RF_LEN_PAYLOAD);   // 不需要ACK
    WriteReg(STATUS, 0x70);   // 清中断标志
    WriteReg(CONFIG, 0x0E);   // 进入发射模式
    CEHigh();              // 至少保持10us以上，将数据发送出去
}
void NRF_LowPower_Mode()
{
    uint8_t u8temp;
    u8temp = ReadReg(CONFIG);
    WriteReg(CONFIG, u8temp & (~(1 << 1)));     //配置工作模式:掉电模式
//  u8temp = halSpiReadByte(CONFIG);
    CELow();
}
/**
 * @name: nrf24l01_init
 * @brief: RF1初始化
 * @author: lzc
 * @param {NRF24L01_t * dev} 结构体
 * @return: None
 * @note: 修改记录：初次创建
 */
void nrf24l01_init()
{
    uint8_t u8channel;
    if (MQTT_Resv_Channel != 0)
    {
        ADDR_Save_Data = MQTT_Resv_Channel ;
    }
    u8channel = (((ADDR_Save_Data - 1) % 2 == 0) ?
                 (2 * (ADDR_Save_Data - 1)) : (2 * (ADDR_Save_Data - 1) - 1));
    CEHigh(); // 使能24L01
    CELow();
    WriteReg(SETUP_RETR, 0x10);             // 不重发  自动重发延时500us + 86us, 自动重发计数10次
    WriteReg(SETUP_AW, RF_LEN_ADDR);        // 设置发送/接收地址长度
    WriteReg(RX_PW_P0, RF_LEN_PAYLOAD);     // 设置通道0数据包长度
    WriteReg(EN_RXADDR, 0x01);              // 接收通道0使能
    WriteReg(EN_AA, 0x00);                  // 关闭自动应答        // 使能通道0接收自动应答
    WriteReg(FEATURE, 0x01);                // 使能W_TX_PAYLOAD_NOACK命令
    WriteReg(RF_CH, u8channel);             // 选择射频工作频道0(0-127)
    WriteReg(RF_SETUP, 0x27);               // 0db, 250Kbps
    printf("NRF MQTT_Resv_Channel -------------: %d", ADDR_Save_Data);
    nRF24L01_EnterRxMode();
    nrf_IRQ_Pin_Set(GPIO_IRQ);
}
/**
 * @name: nRF24L01_2_Init
 * @brief: RF2初始化
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void nrf24l01_2_init()
{
    uint8_t u8channel;
    if (MQTT_Resv_Channel != 0)
    {
        ADDR_Save_Data = MQTT_Resv_Channel ;
    }
    u8channel = (((ADDR_Save_Data - 1) % 2 == 0) ?
                 (2 * (ADDR_Save_Data - 1) + 2) : (2 * (ADDR_Save_Data - 1) + 1));
    CEHigh(); // 使能24L01
    CELow();
    WriteReg_2(SETUP_RETR, 0x10);            // 不重发  自动重发延时500us + 86us, 自动重发计数10次
    WriteReg_2(SETUP_AW, RF_LEN_ADDR);       // 设置发送/接收地址长度
    WriteReg_2(RX_PW_P0, RF_LEN_PAYLOAD);     // 设置通道0数据包长度
    WriteReg_2(EN_RXADDR, 0x01);              // 接收通道0使能
    WriteReg_2(EN_AA, 0x00);                  // 关闭自动应答        // 使能通道0接收自动应答
    WriteReg_2(FEATURE, 0x01);                // 使能W_TX_PAYLOAD_NOACK命令
    WriteReg_2(RF_CH, u8channel);             // 选择射频工作频道0(0-127)
    WriteReg_2(RF_SETUP, 0x27);               // 0db, 250Kbps
    printf("NRF MQTT_Resv_Channel -------------: %d", ADDR_Save_Data);
    nRF24L01_2_EnterRxMode();
    nrf_IRQ_Pin_Set(GPIO_IRQ2);
}
/**
 * @name: Clear_Buffer_TimeOutTask
 * @brief: 清除标志位数据
 * @param {type}
 * @return:
 * @note: 修改记录：初次创建
 */
uint8_t Clear_Flag = 0;
uint8_t TimeOut_Clear_Flag = 0;
void Clear_Buffer_TimeOutTask(void)
{
    uint32_t NewTime = 0;
    NewTime = xTaskGetTickCount();
    for (int i = 0; i < MQTT_Resv_SensorNum; i++)
    {
        //如果超时 MQTT_Resv_SensorCycle（分钟值）
        if (NRF_Data_Poll_Struct.NRF24L01_Time_Count[i] != 0
                && (NewTime - NRF_Data_Poll_Struct.NRF24L01_Time_Count[i] >= (MQTT_Resv_SensorCycle * 6000)))
        {
            //清除每个数组的时钟计时
            NRF_Data_Poll_Struct.NRF24L01_Time_Count[i] = 0 ;
            //将原始数据清除
            for (int j = 0; j < 5; j++)
            {
                NRF_Data_Poll_Struct.NRF24L01_Buf[((i) * 5 + j)] = 0x00 ;
            }
            //将发送数据也清除
            for (uint16_t k = 0; k < 10; k++)
            {
                DataToSendBuffer[((i) * 10 + k)] = 0x30 ;
            }
            //清除标志位置位
            Clear_Flag = 1;
        }
    }
    if (Clear_Flag)
    {
        Clear_Flag = 0;
        TimeOut_Clear_Flag = 1;
        printf("\r\n 超时%d分钟清除数据 ,进入上报处理 \r\n", MQTT_Resv_SensorCycle);
        //进行上报处理
    }
}
/**
 * @name: NRF_Data_Poll
 * @brief: NRF数据处理
 * @param {uint8_t}  数据包指针
 * @return:
 * @note: 修改记录：初次创建
 */
uint8_t Temp_Data[8] ;
void NRF_Data_Poll(uint8_t *Data_Buf)
{
    uint8_t Product_Num = 0;
    if (Data_Buf[1] >= 0x01)
    {
        Product_Num = Data_Buf[1];
        memcpy(Temp_Data, Data_Buf, 8);
        for (uint8_t i = 0; i < 5; i++)
        {
            //把数据赋值给结构体的缓存
            NRF_Data_Poll_Struct.NRF24L01_Buf[((Product_Num - 1) * 5 + i) ] = Temp_Data[i + 2];
            //把时间数据赋值给这个计数缓存
            NRF_Data_Poll_Struct.NRF24L01_Time_Count[(Product_Num - 1)] = xTaskGetTickCount();
            //NRF_Data_Poll_Struct.NRF24L01_Data_Lens ++;
        }
        printf("接收到数据已经存储至 NRF_Data_Poll_Struct.NRF24L01_Buf\r\n");
        HexToStr(DataToSendBuffer, NRF_Data_Poll_Struct.NRF24L01_Buf, 1200);
    }
}
/**
 * @name: nRF24L01_IRQ
 * @brief: RF芯片中断事件
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void nRF24L01_IRQ()
{
    uint8_t u8State;
    u8State = ReadReg(STATUS);
    ESP_LOGI("NRF_IRQ", "Test");
    if (u8State & 0x40)
    {
        // 接收成功
        nrf_spi_master_read_bytes(RD_RX_PLOAD, Buf, RF_LEN_PAYLOAD);
        if (((Buf[0] & 0x3f) == 1)
                && (Buf[RF_LEN_PAYLOAD - 1] == comCalCRC(&Buf[0], RF_LEN_PAYLOAD - 1)))
        {
            /************无线接收数据存储*************/
            if ((Buf[1] >= 1) && (Buf[1] <= TAG_NUM_MAX) /*&& (Buf[1]%2 == 1)*/)
            {
            }
        }
    }
    else if (u8State & 0x10)
    {
        //发射达到最大复发次数
    }
    else if (u8State & 0x20)
    {
        // 发送成功
    }
    for (int i = 0; i < 8; i++)
    {
        printf("NRF24L01 ONE ResvData is -- dev.Buf[%d] : %x \r\n", i, Buf[i]);
    }
    NRF_Data_Poll(Buf);
    nRF24L01_EnterRxMode(); // 进入接收模式
}
/**
 * @name: nRF24L01_IRQ_2
 * @brief: RF芯片中断事件2
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void nRF24L01_IRQ_2()
{
    uint8_t u8State;
    u8State = ReadReg_2(STATUS);
    ESP_LOGI("NRF_IRQ", "Test");
    if (u8State & 0x40)
    {
        // 接收成功
        nrf_spi_master_read_bytes_2(RD_RX_PLOAD, Buf, RF_LEN_PAYLOAD);
        if (((Buf[0] & 0x3f) == 1)
                && (Buf[RF_LEN_PAYLOAD - 1] == comCalCRC(&Buf[0], RF_LEN_PAYLOAD - 1)))
        {
            /************无线接收数据存储*************/
            if ((Buf[1] >= 1) && (Buf[1] <= TAG_NUM_MAX) /*&& (Buf[1]%2 == 1)*/)
            {
            }
        }
    }
    else if (u8State & 0x10)
    {
        //发射达到最大复发次数
    }
    else if (u8State & 0x20)
    {
        // 发送成功
    }
    for (int i = 0; i < 8; i++)
    {
        printf("NRF24L01 TWO ResvData is -- dev.Buf[%d] : %x \r\n", i, Buf[i]);
    }
    NRF_Data_Poll(Buf);
    nRF24L01_2_EnterRxMode(); // 进入接收模式
}
/**
 * @name: Clear_ALL_nrf24l01_TempData
 * @brief: 清除NRF24L01的暂存数据
 * @param {type}
 * @return:
 * @note: 修改记录：初次创建
 */
void Clear_ALL_nrf24l01_TempData(void)
{
    for (uint16_t i = 0; i < DEVICE_COUNT; i++)
    {
        //清除每个数组的时钟计时
        NRF_Data_Poll_Struct.NRF24L01_Time_Count[i] = 0 ;
        //将原始数据清除
        for (int j = 0; j < 5; j++)
        {
            NRF_Data_Poll_Struct.NRF24L01_Buf[((i) * 5 + j)] = 0x00 ;
        }
        //将发送数据也清除
        for (uint16_t k = 0; k < 10; k++)
        {
            DataToSendBuffer[((i) * 10 + k)] = 0x30 ;
        }
    }
    printf("\r\n 清除所有NRF24L01的暂存数据 \r\n");
}
/**
 * @name: NRF_ALLReflash_Channel
 * @brief: 更新NRF24L01通道
 * @param {type}
 * @return:
 * @note: 修改记录：已经进行测试
 */
uint8_t Saved_Channel = 0;
void NRF_ALLReflash_Channel(void)
{
    if (Saved_Channel != MQTT_Resv_Channel)
    {
        /* code */
        ADDR_Save_Data = MQTT_Resv_Channel ;
        nRF24L01_EnterRxMode();                  // 进入接收模式
        nRF24L01_2_EnterRxMode();                // 进入接收模式
        printf("%d 更新NRF通道值  \r\n", ADDR_Save_Data);
    }
    Saved_Channel = ADDR_Save_Data ;
}

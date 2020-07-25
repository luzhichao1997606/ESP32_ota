/*
 * @file: main.c
 * @Descripttion: ethernet_example_main
 * @brief: Ethernet Basic Example
 * @version: 1.0
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-04 16:28:55
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-25 09:03:27
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "sdkconfig.h"
//Ethernet
#include "tcpip_adapter.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "Ethernet_app.h"
//MQTT
#include "MQTT_Handler.h"
//nvs
#include "Storage/Storage_nvs.h"
//LWIP
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
//spi
#include "HW_SPI_Common.h"
//oled
#include "oled_hw_spi.h"
//nrf24l01
#include "NRF24L01.h"
//WIFI
#include "smartconfig.h"
//OTA
#include "https_ota.h"
static const char *TCP  = "TCP_Task";
static const char *SPI  = "SPI_Task";
static const char *NRF  = "NRF_Task";

//创建任务的任务句柄
TaskHandle_t APP_CreatedTask;
TaskHandle_t TCP_CreatedTask;

//主机地址
#define HOST_IP_ADDR    "192.168.3.140"
#define PORT             8080

//静态IP地址
#define STATIC_IP_ADDR  "192.168.3.144"
#define STATIC_SN_ADDR  "255.255.255.0"
#define STATIC_GW_ADDR  "192.168.3.1"

#define CONFIG_EXAMPLE_IPV4             1

NRF24L01_t dev;
int tcpsock = 0;
struct sockaddr_in dest_addr;
//主机ID
uint8_t Read_ID[16] = {0} ;
static const char *payload = "Message from ESP32 ";
/**
 * @name: TCP_Changeip_STATIC
 * @brief: TCP改变IP（静态）
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
static void TCP_Changeip_STATIC(void)
{
    tcpip_adapter_ip_info_t ip_info =
    {
        .ip.addr        = ipaddr_addr(STATIC_IP_ADDR),
        .netmask.addr   = ipaddr_addr(STATIC_SN_ADDR),
        .gw.addr        = ipaddr_addr(STATIC_GW_ADDR),
    };
    //DHCP
    ESP_ERROR_CHECK(tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH));
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &ip_info));
}
/**
 * @name: tcp_app_client_start_Task
 * @brief: TCP连接事件
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
bool ETH_Select_TCP_ConnectWay_Static = false ;
static void tcp_app_client_start_Task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;
    bool Start_Flag = false;
    while (1)
    {
        if (xSemaphoreTake(xSemaphore_Ethernet, (TickType_t) 10) == pdTRUE)
        {
            //获取到信号量 ！
            ESP_LOGI(TCP, "TCP_task get xSemaphoreTake");
            //挂起网口任务
            ESP_LOGI(TCP, "eth_task suspend!");
            vTaskSuspend(eth_CreatedTask);
            //启动
            ESP_LOGI(TCP, "Start static TCP");
            //如果使能静态的IP功能
            if (ETH_Select_TCP_ConnectWay_Static)
            {
                TCP_Changeip_STATIC();
            }
            Start_Flag = true;
        }
        if (Start_Flag)
        {
#ifdef CONFIG_EXAMPLE_IPV4
            struct sockaddr_in dest_addr;
            dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(PORT);
            addr_family = AF_INET;
            ip_protocol = IPPROTO_IP;
            inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
#else // IPV6
            struct sockaddr_in6 dest_addr;
            inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons(PORT);
            addr_family = AF_INET6;
            ip_protocol = IPPROTO_IPV6;
            inet6_ntoa_r(dest_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif
            int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
            if (sock < 0)
            {
                ESP_LOGE(TCP, "Unable to create socket: errno %d", errno);
                break;
            }
            ESP_LOGI(TCP, "Socket created, connecting to %s:%d", HOST_IP_ADDR, PORT);
            int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err != 0)
            {
                ESP_LOGE(TCP, "Socket unable to connect: errno %d", errno);
                break;
            }
            ESP_LOGI(TCP, "Successfully connected");
            while (1)
            {
                int err = send(sock, payload, strlen(payload), 0);
                if (err < 0)
                {
                    ESP_LOGE(TCP, "Error occurred during sending: errno %d", errno);
                    break;
                }
                int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                // Error occurred during receiving
                if (len < 0)
                {
                    ESP_LOGE(TCP, "recv failed: errno %d", errno);
                    break;
                }
                // Data received
                else
                {
                    rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                    ESP_LOGI(TCP, "Received %d bytes from %s:", len, addr_str);
                    ESP_LOGI(TCP, "%s", rx_buffer);
                }
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
            if (sock != -1)
            {
                ESP_LOGE(TCP, "Shutting down socket and restarting...");
                shutdown(sock, 0);
                close(sock);
            }
        }
    }
    vTaskDelete(NULL);
}


/**
 * @name: app_Task
 * @brief: 初始化任务，主要用于初始化一些信号量
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
static void app_Task()
{
    //创建二值信号量
    xSemaphore_Ethernet = xSemaphoreCreateBinary(); // @suppress("Type cannot be resolved")
    xSemaphore_WIFI     = xSemaphoreCreateBinary(); // @suppress("Type cannot be resolved")
    ETH_Select_TCP_ConnectWay_Static = true ;
    printf("初始化任务完成");
    vTaskDelete(APP_CreatedTask);
}
/**
 * @name: app_main
 * @brief: 主函数
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
bool Falg = false;
void app_main()
{
    /* */
    memcpy(Read_ID, (uint8_t *)"20010335", 8);
    /* 初始化线程 */
    xTaskCreate(app_Task, "APP_Task", 4096, NULL, 0, &APP_CreatedTask);
    /* WIFI线程 */
    //initialise_wifi();
    //while (xSemaphoreTake(xSemaphore_WIFI, (TickType_t) 10) != pdTRUE);
    /* 网口线程 */
    xTaskCreate(eth_app_start_Task, "eth_app_Task", 4096, NULL, 1, &eth_CreatedTask);
    while (!Connect_Success);
    Falg = true;
    printf("此为 OTA 程序，版本为1.0 , 准备进行OTA升级 \r\n");
    //printf("此为OTA升级后程序，版本为2.0 \r\n");
    if (Falg)
    {
        //NRF线程
        //xTaskCreate(NRF_IRQ_Handler_start_Task, "NRF_IRQ_Handler_Task", 4096, (void *)&dev, 2, &NRF_CreatedTask);
        /* MQTT线程 */
        xTaskCreate(mqtt_app_start_Task, "MQTT_app_Task", 4096, NULL, 1, &MQTT_CreatedTask);
        //TODO:tcp配置为静态IP
        /* TCP线程 */
        //xTaskCreate(tcp_app_client_start_Task, "tcp_client", 4096, &ETH_Select_TCP_ConnectWay_Static, 0, &TCP_CreatedTask);
    }
}

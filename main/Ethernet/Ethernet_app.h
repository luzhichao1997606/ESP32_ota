/*
 * @file: 
 * @Descripttion: 
 * @brief: 
 * @version: 
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-10 15:36:45
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-24 17:24:55
 */ 
#ifndef __ETHERNET_APP_H__
#define __ETHERNET_APP_H__
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
//Ethernet
#include "tcpip_adapter.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"


//创建二值信号量句柄
SemaphoreHandle_t xSemaphore_Ethernet;   
extern SemaphoreHandle_t xSemaphore_Ethernet;
//任务句柄
TaskHandle_t eth_CreatedTask;
extern TaskHandle_t eth_CreatedTask;
extern bool Connect_Success ;
///////////////////////////////////////////////////////////////////////////////////
//xTaskCreate(eth_app_start_Task, "eth_app_Task", 4096, NULL, 1, &eth_CreatedTask);
//////////////////////////////////////////////////////////////////////////////////
void eth_app_start_Task();  //网口函数的任务

#endif

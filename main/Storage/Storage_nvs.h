/*
 * @file: H文件（Storage_nvs）
 * @Descripttion: 
 * @brief: 
 * @version: 
 * @author: lzc
 * @attention:  NONE
 * @Date: 2020-07-10 11:43:27
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-10 15:19:30
 */  
#ifndef __STORAGE_NVS_H__
#define __STORAGE_NVS_H__


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"


void nvs_write_data_to_flash_Test(void);

void nvs_read_data_from_flash_Test(void);
///< nvs存储
void nvs_write_U8data_to_flash(const char * Storage_Name,const char * Value_Name , uint8_t Value);
///< nvs读取
uint8_t nvs_read_U8data_from_flash(const char *Storage_Name, const char *Value_Name);
#endif

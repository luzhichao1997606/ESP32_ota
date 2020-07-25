/*
 * @file: 
 * @Descripttion: 
 * @brief: 
 * @version: 
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-10 15:57:17
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-24 16:53:24
 */ 
#ifndef __MQTT_HANDLER_H__
#define __MQTT_HANDLER_H__

#include <stdio.h>
#include <string.h>
#include "tcpip_adapter.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "Ethernet_app.h"
#include "smartconfig.h"
#include "cJSON.h"
#include "mqtt_client.h"
#include "Storage_nvs.h"
#include "NRF24L01.h"
//DNS

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

//OTA
#include "https_ota.h"

#define MQTT_Publish_Type_SendData      0
#define MQTT_Publish_Type_HeartBeat     1
#define MQTT_Publish_Type_CountLess40   2
/******************************************************************************/
/***                        结构体和联合体                                   ***/
/******************************************************************************/
typedef enum
{
   NETINFO_STATIC = 1,    ///< Static IP configuration by manually.
   NETINFO_DHCP           ///< Dynamic IP configruation from a DHCP sever
}dhcp_mode;

typedef struct
{
    //MQTT
    uint8_t     MQTT_Resv_Cycle;            // 上报周期，单位分钟，1~255
    uint8_t     MQTT_Resv_AlarmTime;        // 报警持续时间，单位分钟，0~255，0不报警，255持续报警
    uint8_t     MQTT_Resv_Channel;          // 工作信道（传参到NRF24L01）
    uint8_t     MQTT_Resv_SensorNum ;       // 1-240 该数传设备下面的采集模块数量(配置数组)
    uint8_t     MQTT_Resv_SensorCycle ;     // 传感器上报周期，单位分钟，10~255，最低10分钟(清除数组数据)
    uint8_t     POWER_ON_COUNT  ;
    //TCP
    uint8_t     SIP_SaveData[4];            //静态IP
    uint8_t     GW_SaveData[4];             //默认网关
    uint8_t     YIP_SaveData[4];            //服务器IP
    uint8_t     SN_SaveData[4];             //子网掩码

    uint16_t    SPORT_SaveData;             //静态端口
    uint16_t    YPORT_SaveData;             //服务器端口

    dhcp_mode   CONFIG_SaveData;            //配置存储信息

} para_cfg_t;

/***********************结构体和联合体的实例化*****************************/
para_cfg_t          MQTT_Save_Data ;
/******************************************************************************/
/***                            外部调用声明                                 ***/
/******************************************************************************/
//Alarm 订阅解析
extern unsigned char MQTT_Resv_Alarm ;
extern char *MQTT_Resv_AlarmData ;
//Read_Data 订阅解析
extern unsigned char MQTT_Resv_Read_data ;
//Updata 订阅解析
extern unsigned char MQTT_Resv_Cycle ;
extern unsigned char MQTT_Resv_AlarmTime ;
extern unsigned char MQTT_Resv_Channel ;
extern unsigned char MQTT_Resv_SensorNum ;
extern unsigned char MQTT_Resv_SensorCycle ;

extern uint8_t  Read_ID[16] ;
extern TaskHandle_t eth_CreatedTask;  

TaskHandle_t MQTT_CreatedTask;
extern TaskHandle_t MQTT_CreatedTask;

extern const char *storage_Name ;
extern const char *Page_Name    ;
extern bool Falg;
extern uint8_t  DataToSendBuffer[2400] ;  
void mqtt_app_start_Task();
#endif
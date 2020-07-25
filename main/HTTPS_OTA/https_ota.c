/*
 * @file:
 * @Descripttion:
 * @brief:
 * @version:
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-18 17:37:34
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-24 17:40:40
 */
#include "https_ota.h"

#define OTA_URL_SIZE 256
char OTA_Adder_IP[] = "https://47.98.136.66:8070/" ;
char OTA_BIN_File[] = "ethernet_basic.bin" ;
char OTA_URL[] = "" ;
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");
/**
 * @name:
 * @brief:
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(OTA_TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(OTA_TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(OTA_TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(OTA_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(OTA_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(OTA_TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(OTA_TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}
/**
 * @name:Stop_Taskwdt_OTATask
 * @brief:关闭任务看门狗
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void Stop_Taskwdt_OTATask()
{
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
}
/**
 * @name:simple_ota_example_task
 * @brief:OTA任务
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
bool OTA_Flag = false;
uint8_t OTA_wdt_Status = 0;
void simple_ota_example_task(void *pvParameter)
{
    ESP_LOGI(OTA_TAG, "Starting OTA example");
    strcat(OTA_URL, OTA_Adder_IP);
    strcat(OTA_URL, OTA_BIN_File);
    printf("OTA_URL is %s \r\n", OTA_URL);
    printf("挂起其他无关任务\r\n");
    //挂起MQTT
    vTaskSuspend(MQTT_CreatedTask);
    //vTaskSuspend(NRF_CreatedTask); 
    esp_http_client_config_t config =
    {
        .url = OTA_URL,
        .cert_pem = (char *)server_cert_pem_start,
        .event_handler = _http_event_handler,
    };
#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0)
    {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    }
    else
    {
        ESP_LOGE(OTA_TAG, "Configuration mismatch: wrong firmware upgrade image url");
        abort();
    }
#endif
#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif
    //设置Task看门狗的超时时间-（10分钟）
    esp_task_wdt_init(600, OTA_Flag);
    esp_task_wdt_add(&OTA_Handler);
    OTA_wdt_Status = esp_task_wdt_status(&OTA_Handler);
    //喂狗
    esp_task_wdt_reset();
    printf("OTA_wdt_Status is %d \r\n", OTA_wdt_Status);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK)
    {
        esp_restart();
    }
    else
    {
        ESP_LOGE(OTA_TAG, "Firmware upgrade failed");
    }
    while (1)
    {
        //喂狗
        esp_task_wdt_reset();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
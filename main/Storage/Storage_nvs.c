/*
 * @file:Storage_nvs.c
 * @Descripttion:存储NVS数据
 * @brief:
 * @version:
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-10 11:43:12
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-14 11:39:46
 */
#include "Storage_nvs.h"

/**
 * @name:nvs_write_data_to_flash_Test
 * @brief:存储NVS数据(测试)
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
void nvs_write_data_to_flash_Test(void)
{
    esp_err_t err = nvs_flash_init();
    //错误处理
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");
        // Write
        printf("Updating restart counter in NVS ... ");
        restart_counter++;
        err = nvs_set_i32(my_handle, "restart_counter", restart_counter);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Commit written value.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Close
        nvs_close(my_handle);
    }
    printf("\n");
}
/**
 * @name: nvs_read_data_from_flash_Test
 * @brief: 获取数据
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void nvs_read_data_from_flash_Test(void)
{
    esp_err_t err = nvs_flash_init();
    //错误处理
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");
        // Read
        printf("Reading restart counter from NVS ... ");
        err = nvs_get_i32(my_handle, "restart_counter", &restart_counter);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("Restart counter = %d\n", restart_counter);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        // Close
        nvs_close(my_handle);
    }
    printf("\n");
}

/**
 * @name:nvs_write_U8data_to_flash
 * @brief:存储NVS数据(UINT8_T)
 * @author: lzc
 * @param {Storage_Name} 存储块名
 * @param {Value_Name} 存储页名
 * @param {Value} u8的值
 * @return: None
 * @note: 修改记录：初次创建
 */
void nvs_write_U8data_to_flash(const char *Storage_Name, const char *Value_Name, uint8_t Value)
{
    esp_err_t err = nvs_flash_init();
    //错误处理
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open(Storage_Name, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");
        // Write
        printf("Updating %s in NVS ... \r\n", Value_Name);
        printf("%s in NVS value now is %d... \r\n", Value_Name,Value);
        err = nvs_set_u8(my_handle, Value_Name, Value);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Commit written value.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Close
        nvs_close(my_handle);
    }
    printf("\n");
}


/**
 * @name: nvs_read_U8data_from_flash
 * @brief: 获取数据--（u8）
 * @author: lzc
 * @param {Storage_Name} 存储块名
 * @param {Value_Name} 存储页名
 * @return: None
 * @note: 修改记录：初次创建
 */
uint8_t nvs_read_U8data_from_flash(const char *Storage_Name, const char *Value_Name)
{
    esp_err_t err = nvs_flash_init();
    uint8_t Return_Data = 0;
    //错误处理
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    // Open
    printf("\n");
    printf("Opening %s Non-Volatile Storage (NVS) handle... ",Storage_Name);
    nvs_handle_t my_handle;
    err = nvs_open(Storage_Name, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");
        // Read
        printf("Reading %s from NVS ... ",Value_Name);
        err = nvs_get_u8(my_handle, Value_Name, &Return_Data);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("Return_Data = %d\n", Return_Data);
            nvs_close(my_handle);
            return  Return_Data;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        // Close
        nvs_close(my_handle);
    }
    printf("\n");
    return  0;
}
/**
 * @name: nvs_read_MQTT
 * @brief: 读取函数
 * @author: lzc
 * @param {MQTT_Save_Data} 要读取的结构体指针
 * @param {len} 长度
 * @return: None
 * @note: 修改记录：初次创建
 */
//esp_err_t nvs_read_MQTT(struct  para_cfg_t *MQTT_Save_Data, uint32_t *len)
//{
//    nvs_handle_t nvs_handle;
//    esp_err_t err;
//    //首先一部操作打开存储空间
//    // Open
//    //para(空间名字（字符串），操作类型（只读还是读写），操作句柄)
//    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
//    if (err != ESP_OK)
//    {
//        printf("NVS_READ：存储空间打开失败\n");
//        return err;
//    }
////注意这一步十分重要，因为在NVS存储已知长度类型的数据时，我们可以明确的传入已知的长度。
////但是这个地方对于我们传入的数组或者说结构体，我们不知道明确长度，于是我们采用下面的操作来获取要读取的数据的长度。
//// Read the size of memory space required for blob
//    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
//    err = nvs_get_blob(nvs_handle, "MQTT_Save_Data", NULL, &required_size);
//    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
////再来读取对应的值
//    err = nvs_get_blob(nvs_handle, "MQTT_Save_Data", MQTT_Save_Data, &required_size);
//    if (err == ESP_ERR_NVS_NOT_FOUND)
//    {
//        printfln("NVS_READ：key doesn’t exist");
//        return err;
//    }
//    else if (err == ESP_ERR_NVS_INVALID_HANDLE)
//    {
//        printfln("NVS_READ：handle has been closed or is NULL");
//        return err;
//    }
//    else if (err == ESP_ERR_NVS_INVALID_NAME)
//    {
//        printfln("NVS_READ：name doesn’t satisfy constraints ");
//        return err;
//    }
//    else if (err == ESP_ERR_NVS_INVALID_LENGTH)
//    {
//        printfln("NVS_READ：length is not sufficient to store data");
//        return err;
//    }
//    else
//    {
//        printfln("NVS_READ：读取成功");
//    }
//    //关闭句柄
//    nvs_close(nvs_handle);
//    return ESP_OK;
//}
//
//
///**
// * @name: nvs_write_MQTT
// * @brief: 存储函数
// * @author: lzc
// * @param {MQTT_Save_Data} 要写入的结构体指针。
// * @param {len}长度
// * @return: None
// * @note: 修改记录：初次创建
// */
//esp_err_t nvs_write_MQTT(struct  para_cfg_t *MQTT_Save_Data,  uint32_t len)
//{
//    nvs_handle_t nvs_handle;
//    esp_err_t err;
//    printfln("NVS_WRITE：存储MQTT信息\n");
//    // Open
//    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
//    if (err != ESP_OK)
//    {
//        printf("NVS_WRITE：存储空间打开失败\n");
//        return err;
//    }
//    err = nvs_set_blob(nvs_handle, "MQTT_Save_Data", MQTT_Save_Data, len);
//    if (err != ESP_OK)
//    {
//        printf("NVS_WRITE：存储空间存储失败\n");
//        return err;
//    }
//    err = nvs_commit(nvs_handle);
//    if (err != ESP_OK)
//    {
//        printf("NVS_WRITE：存储空间提交失败\n");
//        return err;
//    }
//    err = nvs_close(nvs_handle);
//    if (err != ESP_OK)
//    {
//        printf("NVS_WRITE：存储空间关闭失败\n");
//        return err;
//    }
//    return ESP_OK;
//}

/*
 * @file:
 * @Descripttion:
 * @brief:
 * @version:
 * @author: lzc
 * @attention: 注意：
 * @Date: 2020-07-10 15:56:49
 * @LastEditors: lzc
 * @LastEditTime: 2020-07-25 14:02:05
 */
#include "MQTT_Handler.h"

//RelayPIN
#define         Relay_PIN           4
#define         OneMinVal           (100 * 60)

/**MQTT**/
//如果检测到存储的flash页空则先把默认数据赋值
//也可以把读取的数据取出作为全局变量

//Alarm 订阅解析
unsigned char MQTT_Resv_Alarm = 0;
char *MQTT_Resv_AlarmData ;
//Read_Data 订阅解析
unsigned char MQTT_Resv_Read_data =   0;
//Updata 订阅解析
unsigned char MQTT_Resv_Cycle       = 5;      // 上报周期，单位分钟，1~255
unsigned char MQTT_Resv_AlarmTime   = 5;      // 报警持续时间，单位分钟，0~255，0不报警，255持续报警
unsigned char MQTT_Resv_Channel     = 30;     // 工作信道（传参到NRF24L01）
unsigned char MQTT_Resv_SensorNum   = 240;    // 1-240 该数传设备下面的采集模块数量(配置数组)
unsigned char MQTT_Resv_SensorCycle = 2;     // 传感器上报周期，单位分钟，10~255，最低10分钟(清除数组数据)
//OTA_Enable 订阅解析
unsigned char MQTT_Resv_OTA = 0;
//TAG
static const char *MQTT             = "MQTT_Task";

//NVS
const char *storage_Name            = "MQTT_Save_Data";

const char *Page_Name_Cycle         = "Cycle";
const char *Page_Name_AlarmTime     = "AlarmTime";
const char *Page_Name_Channel       = "Channel";
const char *Page_Name_SensorNum     = "SensorNum";
const char *Page_Name_SensorCycle   = "SensorCycle";


//硬件和软件的版本
const char SoftWareVer[] = "1.1.0";
const char HardWareVer[] = "1.0.0";
//MQTT读取Buf
unsigned char tempBuffer[50];
//MQTT 发送数据
unsigned char SendBuffer[600];//40*5*2
unsigned char W5500_NOPHY_TryGPRS_Flag = 0 ;
unsigned char W5500_DHCP_Flag = 0 ;
//MQTT主题
char Topic[50] = "/WSN-LW/";    //用于发送
char SubTopic[50] = "/WSN-LW/"; //用于订阅

//初次上电心跳包发送标志位
uint8_t HeartBeat_FirstPowerON_PublishFlag ;

//AT-----MQTT接受数据
uint8_t CSQ_Val = 0;
char IMSI_DATA[15] = {0};

/* DNS 流程--wifi*/
// 1、定义一个hints结构体，用来设置函数的getaddrinfo()的使用方式
const struct addrinfo hints =
{
    .ai_family = AF_INET,           /* 指定返回地址的协议簇，AF_INET(IPv4)、AF_INET6(IPv6)、AF_UNSPEC(IPv4 and IPv6)*/
    .ai_socktype = SOCK_STREAM,     /* 设定返回地址的socket类型，流式套接字 */
};
char IPV4_IP_Addr[100];                      /* 用来存储IP地址字符串 */
char *Get_DNS_ADDR_IP4(void)
{
    struct sockaddr_in  *ipv4 = NULL;   /* IPv4地址结构体指针 */
    // 2、使用getaddrinfo()开始解析,定义一个struct addrinfo结构体指针，用来获取解析结果
    struct addrinfo *result;
    int err;
    err = getaddrinfo("www.dawen.ltd", "1883", &hints, &result);
    if (err != 0)       /* 返回值不为0，函数执行失败*/
        printf("getaddrinfo err: %d \n", err);
    // 3、将获取到的信息打印出来
    if (result->ai_family == AF_INET)
        {
            ipv4 = (struct sockaddr_in *)result->ai_addr;
            inet_ntop(result->ai_family, &ipv4->sin_addr, IPV4_IP_Addr, sizeof(IPV4_IP_Addr));
            printf("[IPv4]%s [port]%d \n", IPV4_IP_Addr, ntohs(ipv4->sin_port));
        }
    else
        printf("got IPv4 err !!!\n");
    // 4、释放addrinfo 内存
    freeaddrinfo(result);
    return IPV4_IP_Addr;
}
/**
 * @name: Write_MQTT_SaveData2NVS
 * @brief: 写MQTT数据到NVS
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void Write_MQTT_SaveData2NVS(void)
{
    nvs_write_U8data_to_flash(storage_Name, Page_Name_Cycle, MQTT_Save_Data.MQTT_Resv_Cycle);
    nvs_write_U8data_to_flash(storage_Name, Page_Name_AlarmTime, MQTT_Save_Data.MQTT_Resv_AlarmTime);
    nvs_write_U8data_to_flash(storage_Name, Page_Name_Channel, MQTT_Save_Data.MQTT_Resv_Channel);
    nvs_write_U8data_to_flash(storage_Name, Page_Name_SensorNum, MQTT_Save_Data.MQTT_Resv_SensorNum);
    nvs_write_U8data_to_flash(storage_Name, Page_Name_SensorCycle, MQTT_Save_Data.MQTT_Resv_SensorCycle);
    ESP_LOGI(MQTT, "Write MQTT_SaveData to NVS finish!");
}
/**
 * @name: Reade_MQTT_SaveData_From_NVS
 * @brief: 读取MQTT数据从NVS
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
void Reade_MQTT_SaveData_From_NVS(void)
{
    MQTT_Resv_Cycle         = nvs_read_U8data_from_flash(storage_Name, Page_Name_Cycle);
    MQTT_Resv_AlarmTime     = nvs_read_U8data_from_flash(storage_Name, Page_Name_AlarmTime);
    MQTT_Resv_Channel       = nvs_read_U8data_from_flash(storage_Name, Page_Name_Channel);
    MQTT_Resv_SensorNum     = nvs_read_U8data_from_flash(storage_Name, Page_Name_SensorNum);
    MQTT_Resv_SensorCycle   = nvs_read_U8data_from_flash(storage_Name, Page_Name_SensorCycle);
    ESP_LOGI(MQTT, "Read MQTT_SaveData from NVS finish!");
}
/**
 * @name: Creat_json_MQTT_SendData
 * @brief: 创建一个json格式的数据上传的数据格式
 * @author: lzc
 * @param {Pub_State}  0 : SendDate 1 : Heartbeat
 * @return: None
 * @note: 修改记录：初次创建
 */
size_t HeartBeat_lenght = 0;                // @suppress("Type cannot be resolved")
size_t SendData_lenght = 0;                 // @suppress("Type cannot be resolved")
const unsigned char PacksSensorNum = 40;    // @suppress("Type cannot be resolved")
char DataRiver[401]  ;                      // @suppress("Type cannot be resolved")
cJSON *cjson_Object;                        // @suppress("Type cannot be resolved")
uint32_t NewTime = 0;                       // @suppress("Type cannot be resolved")
uint16_t Pack_Num_Last = 0;                 //传感器少于40
uint8_t SensorNum_least = 0;
char *Creat_json_MQTT_SendData(unsigned char Pub_State, unsigned char Pack_NUM) // @suppress("Type cannot be resolved")
{
    static char *out ; // @suppress("Type cannot be resolved")
    cjson_Object = cJSON_CreateObject(); //创建根数据对象
    //char DataRiver_TempForLess[(PacksSensorNum * 10) + 1]  ;
    switch (Pub_State)
        {
            //MQTT数据上报-传感器大于40个
            case MQTT_Publish_Type_SendData:
                //若传感器数量错误，给一个默认值
                if (MQTT_Resv_SensorNum == 0)
                    {
                        MQTT_Resv_SensorNum = 120;
                    }
                memset(DataRiver, 0x00, 401);
                memcpy(DataRiver, DataToSendBuffer + ((PacksSensorNum * 10) * (Pack_NUM - 1)), (PacksSensorNum * 10));
                //发测试数据的
                if (0)
                    {
                        //001B701C24
                        //for (uint16_t j = 0; j < 40; j++) // @suppress("Type cannot be resolved")
                        //{
                        //    DataRiver[(j * 10) + 0] = '0';
                        //    DataRiver[(j * 10) + 1] = '0';
                        //    DataRiver[(j * 10) + 2] = '1';
                        //    DataRiver[(j * 10) + 3] = 'b';
                        //    DataRiver[(j * 10) + 4] = '7';
                        //    DataRiver[(j * 10) + 5] = '0';
                        //    DataRiver[(j * 10) + 6] = '1';
                        //    DataRiver[(j * 10) + 7] = 'c';
                        //    DataRiver[(j * 10) + 8] = '2';
                        //    DataRiver[(j * 10) + 9] = '4';
                        //}
                    }
                //字符串休止'/0'
                DataRiver[(PacksSensorNum * 10)] = '\0' ;
                //如果数据>40 但是 还是不能被40整除。
                if (MQTT_Resv_SensorNum % PacksSensorNum
                        && (((Pack_NUM * 40) - MQTT_Resv_SensorNum) == MQTT_Resv_SensorNum % PacksSensorNum))
                    {
                        SensorNum_least = MQTT_Resv_SensorNum % PacksSensorNum ;
                        memset(DataRiver, 0x00, (SensorNum_least) * 10);
                        memcpy(DataRiver, DataToSendBuffer + ((PacksSensorNum * 10) * (Pack_NUM - 1)), (SensorNum_least * 10));
                        //字符串休止'/0'
                        DataRiver[(SensorNum_least * 10)] = '\0' ;
                        cJSON_AddItemToObject(cjson_Object, "SensorNum", cJSON_CreateNumber(SensorNum_least));
                        cJSON_AddItemToObject(cjson_Object, "SensorStart", cJSON_CreateNumber(((Pack_NUM - 1)*PacksSensorNum) + 1));
                        cJSON_AddItemToObject(cjson_Object, "SensorData", cJSON_CreateString(DataRiver));
                    }
                else
                    {
                        cJSON_AddItemToObject(cjson_Object, "SensorNum", cJSON_CreateNumber(PacksSensorNum));
                        cJSON_AddItemToObject(cjson_Object, "SensorStart", cJSON_CreateNumber(((Pack_NUM - 1)*PacksSensorNum) + 1));
                        cJSON_AddItemToObject(cjson_Object, "SensorData", cJSON_CreateString(DataRiver));
                    }
                //out = cJSON_Print(cjson_Object); //将json形式打印成正常字符串形式
                out = cJSON_PrintUnformatted(cjson_Object); //将json形式打印成正常字符串形式
                printf("json Data : %s\r\n", out);
                printf("\r\nDataToSendBuffer : %s \r\n", DataRiver)  ;
                SendData_lenght = strlen(out)  ;
                printf("\r\n SendData_lenght : %d \r\n", SendData_lenght);
                return out;
            //MQTT心跳数据上报
            case MQTT_Publish_Type_HeartBeat:
                //NewTime = xTaskGetTickCount();
                NewTime ++ ;
                cJSON_AddItemToObject(cjson_Object, "Heartbeat", cJSON_CreateNumber(NewTime));
                //第一次上电添加硬件和软件信息
                if (HeartBeat_FirstPowerON_PublishFlag)
                    {
                        //软件版本
                        cJSON_AddItemToObject(cjson_Object, "SoftWareversion", cJSON_CreateString(SoftWareVer));
                        //硬件版本
                        cJSON_AddItemToObject(cjson_Object, "HardWareVersion", cJSON_CreateString(HardWareVer));
                        if (W5500_NOPHY_TryGPRS_Flag)
                            {
                                //CSQ
                                cJSON_AddItemToObject(cjson_Object, "CSQ", cJSON_CreateNumber(CSQ_Val));
                                //IMSI
                                cJSON_AddItemToObject(cjson_Object, "IMSI", cJSON_CreateString(IMSI_DATA));
                            }
                        HeartBeat_FirstPowerON_PublishFlag = 0;
                    }
                out = cJSON_PrintUnformatted(cjson_Object); //将json形式打印成正常字符串形式
                printf("json Data : %s\n", out);
                // 释放内存
                HeartBeat_lenght = strlen(out);
                printf("HeartBeat_lenght : %d \r\n", HeartBeat_lenght);
                return out;
            //传感器数量少于40
            case MQTT_Publish_Type_CountLess40 :
                //若传感器数量错误，给一个默认值
                printf("PacksSensorNum Less than 40 \r\n ");
                if (MQTT_Resv_SensorNum == 0)
                    {
                        MQTT_Resv_SensorNum = 40;
                    }
                memset(DataRiver, 0x00, Pack_Num_Last * 10);
                memcpy(DataRiver, DataToSendBuffer + ((Pack_Num_Last * 10) * (Pack_NUM - 1)), (Pack_Num_Last * 10));
                //字符串休止'/0'
                DataRiver[(Pack_Num_Last * 10)] = 0x00 ;
                cJSON_AddItemToObject(cjson_Object, "SensorNum", cJSON_CreateNumber(Pack_Num_Last));
                cJSON_AddItemToObject(cjson_Object, "SensorStart", cJSON_CreateNumber(((Pack_NUM - 1)*Pack_Num_Last) + 1));
                cJSON_AddItemToObject(cjson_Object, "SensorData", cJSON_CreateString(DataRiver));
                out = cJSON_PrintUnformatted(cjson_Object); //将json形式打印成正常字符串形式
                printf("json Data : %s\r\n", out);
                printf("\r\n DataRiver : %s \r\n", DataRiver)  ;
                SendData_lenght = strlen(out) ;
                printf("\r\nSendData_lenght : %d \r\n", SendData_lenght);
                return out;
            default:
                break;
        }
    return 0;
}

/**
 * @name: Unpack_json_MQTT_ResvData
 * @brief: 解包函数
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
int Unpack_json_MQTT_ResvData(char *ResvData)
{
    cJSON *json;
    cJSON *json_value;
    char TempBuffer_Json[5] ;
    int Str_len ;
    json = cJSON_Parse(ResvData);
    if (NULL == json)
        {
            printf("Error before: %s\n", cJSON_GetErrorPtr());
            return -1;
        }
    //获取数据长度
    Str_len = strlen(ResvData);
    printf("Str_len of ResvData : %d\r\n", Str_len);
    //获取前几位的数据
    strncpy(TempBuffer_Json, ResvData, 5);
    //Alarm 订阅解析
    if (strchr(TempBuffer_Json, 'A') != NULL)
        {
            json_value = cJSON_GetObjectItem(json, "Alarm");
            if (json_value->type == cJSON_Number)
                {
                    MQTT_Resv_Alarm = json_value->valueint ;
                    printf("Alarm: %d\n", MQTT_Resv_Alarm);
                }
        }
    //Read_Data 订阅解析(OK)
    if (strchr(TempBuffer_Json, 'R') != NULL)
        {
            json_value = cJSON_GetObjectItem(json, "Read_data");
            if (json_value->type == cJSON_Number)
                {
                    // MQTT_Resv_Read_data = json_value->valueint;
                    MQTT_Resv_Read_data = 1 ;
                    printf("Read_data: %d\n", MQTT_Resv_Read_data);
                }
        }
    //Updata 订阅解析
    if (strchr(TempBuffer_Json, 'C') != NULL)
        {
            json_value = cJSON_GetObjectItem(json, "Cycle");
            //更新存储
            if (json_value->type == cJSON_Number)
                {
                    // 上报周期，单位分钟，1~255(OK)
                    MQTT_Resv_Cycle = json_value->valueint;
                    MQTT_Save_Data.MQTT_Resv_Cycle = MQTT_Resv_Cycle;
                    printf("Cycle is %d\n", MQTT_Resv_Cycle);
                }
            json_value = cJSON_GetObjectItem(json, "AlarmTime");
            if (json_value->type == cJSON_Number)
                {
                    // 报警持续时间，单位分钟，0~255，0不报警，255持续报警
                    MQTT_Resv_AlarmTime = json_value->valueint;
                    MQTT_Save_Data.MQTT_Resv_AlarmTime = MQTT_Resv_AlarmTime;
                    printf("AlarmTime is %d\n", MQTT_Resv_AlarmTime);
                }
            json_value = cJSON_GetObjectItem(json, "Channel");
            if (json_value->type == cJSON_Number)
                {
                    // 工作信道
                    MQTT_Resv_Channel = json_value->valueint;
                    MQTT_Save_Data.MQTT_Resv_Channel = MQTT_Resv_Channel;
                    printf("Channel is %d\n", (uint8_t)MQTT_Resv_Channel);
                }
            json_value = cJSON_GetObjectItem(json, "SensorNum");
            if (json_value->type == cJSON_Number)
                {
                    // 1-240 该数传设备下面的采集模块数量(OK?)
                    MQTT_Resv_SensorNum = json_value->valueint;
                    MQTT_Save_Data.MQTT_Resv_SensorNum = MQTT_Resv_SensorNum;
                    printf("SensorNum is %d\n", MQTT_Resv_SensorNum);
                }
            json_value = cJSON_GetObjectItem(json, "SensorCycle");
            if (json_value->type == cJSON_Number)
                {
                    // 传感器上报周期，单位分钟，10~255，最低10分钟
                    // 将数据本地保存，防止无线没收到时数据丢失
                    MQTT_Resv_SensorCycle = json_value->valueint;
                    MQTT_Save_Data.MQTT_Resv_SensorCycle = MQTT_Resv_SensorCycle;
                    printf("SensorCycle is %d\n", MQTT_Resv_SensorCycle);
                }
            //存储
            Write_MQTT_SaveData2NVS();
            MQTT_Save_Data.MQTT_Resv_Cycle          = 0;
            MQTT_Save_Data.MQTT_Resv_AlarmTime      = 0;
            MQTT_Save_Data.MQTT_Resv_Channel        = 0;
            MQTT_Save_Data.MQTT_Resv_SensorNum      = 0;
            MQTT_Save_Data.MQTT_Resv_SensorCycle    = 0;
            MQTT_Save_Data.POWER_ON_COUNT           = 1;
        }
    ///* OTA升级线程 */
    if (strchr(TempBuffer_Json, 'O') != NULL)
        {
            json_value = cJSON_GetObjectItem(json, "OTA_Enable");
            //更新存储
            if (json_value->type == cJSON_Number)
                {
                    // 上报周期，单位分钟，1~255(OK)
                    MQTT_Resv_OTA = 1;
                    printf("MQTT_Resv_OTA is %d\n", MQTT_Resv_OTA);
                }
        }
    cJSON_Delete(json);
    return 0;
}

/**
 * @name: mqtt_event_handler_cb
 * @brief: MQTT处理回调函数
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */

char MQTT_Resv_Data_Poll[100]  ;
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    char *MQTT_Heartbeat_First_Pack ;
    char *MQTT_Resv_Data_ALL = {0};
    // your_context_t *context = event->context;
    switch (event->event_id)
        {
            case MQTT_EVENT_CONNECTED://连接MQTT成功
                ESP_LOGI(MQTT, "MQTT_EVENT_CONNECTED");
                /// 发布
                //上电第一包数据（心跳包）
                HeartBeat_FirstPowerON_PublishFlag = 1;
                MQTT_Heartbeat_First_Pack =  Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat, 0);
                //合成MQTT主题（心跳）
                sprintf(Topic, "/WSN-LW/");
                strcat(Topic, &Read_ID);
                strcat(Topic, "/event/Heartbeat");
                msg_id =    esp_mqtt_client_publish(client, Topic,
                                                    MQTT_Heartbeat_First_Pack,
                                                    0,
                                                    0,
                                                    0);
                //释放内存
                cJSON_Delete(cjson_Object);
                ESP_LOGI(MQTT, "mqtt_json data is %s", MQTT_Heartbeat_First_Pack);
                ESP_LOGI(MQTT, "sent publish successful, msg_id=%d", msg_id);
                /// 订阅
                //合成MQTT主题（获取数据）
                sprintf(SubTopic, "/WSN-LW/");
                strcat(SubTopic, &Read_ID);
                strcat(SubTopic, "/service/Read_data");
                msg_id = esp_mqtt_client_subscribe(client, SubTopic, 0);
                ESP_LOGI(MQTT, "sent subscribe Read_data successful, msg_id=%d", msg_id);
                memset(SubTopic, 0x00, strlen(SubTopic));
                //合成MQTT主题（Alarm）
                sprintf(SubTopic, "/WSN-LW/");
                strcat(SubTopic, &Read_ID);
                strcat(SubTopic, "/service/Alarm");
                msg_id = esp_mqtt_client_subscribe(client, SubTopic, 0);
                ESP_LOGI(MQTT, "sent subscribe Read_data successful, msg_id=%d", msg_id);
                memset(SubTopic, 0x00, strlen(SubTopic));
                //合成MQTT主题（Update）
                sprintf(SubTopic, "/WSN-LW/");
                strcat(SubTopic, &Read_ID);
                strcat(SubTopic, "/service/Update");
                msg_id = esp_mqtt_client_subscribe(client, SubTopic, 0);
                ESP_LOGI(MQTT, "sent subscribe Update successful, msg_id=%d", msg_id);
                memset(SubTopic, 0x00, strlen(SubTopic));
                //合成MQTT主题（OTA）
                sprintf(SubTopic, "/WSN-LW/");
                strcat(SubTopic, &Read_ID);
                strcat(SubTopic, "/service/OTA");
                msg_id = esp_mqtt_client_subscribe(client, SubTopic, 0);
                ESP_LOGI(MQTT, "sent subscribe Update successful, msg_id=%d", msg_id);
                memset(SubTopic, 0x00, strlen(SubTopic));
                break;
            case MQTT_EVENT_DISCONNECTED://断开MQTT
                ESP_LOGI(MQTT, "MQTT_EVENT_DISCONNECTED");
                break;
            case MQTT_EVENT_SUBSCRIBED://订阅成功
                printf("_---------订阅--------\n");
                ESP_LOGI(MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                // msg_id = esp_mqtt_client_publish(client, "/World", "data", 0, 0, 0);
                // ESP_LOGI(MQTT, "sent publish successful, msg_id=%d", msg_id);
                break;
            case MQTT_EVENT_UNSUBSCRIBED://取消订阅
                ESP_LOGI(MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
                break;
            case MQTT_EVENT_PUBLISHED://发布成功
                printf("_--------发布----------\n");
                ESP_LOGI(MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
                break;
            case MQTT_EVENT_DATA://数据接收
                ESP_LOGI(MQTT, "MQTT_EVENT_DATA");
                printf("主题长度:%d 数据长度:%d\n", event->topic_len, event->data_len);
                //数据接收
                printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
                printf("DATA=%.*s\r\n", event->data_len, event->data);
                //由于MQTT接收的数据除去JSON部分会有乱码所以得先把后面的数据去除。
                MQTT_Resv_Data_ALL = event->data;
                //清除之前的旧数据
                memset(MQTT_Resv_Data_Poll, 0x00, strlen(MQTT_Resv_Data_Poll));
                //获取前几位的数据
                strncpy(MQTT_Resv_Data_Poll, MQTT_Resv_Data_ALL, event->data_len);
                printf("MQTT_Resv_Data_Poll is %s", MQTT_Resv_Data_Poll);
                Unpack_json_MQTT_ResvData(MQTT_Resv_Data_Poll);
                break;
            case MQTT_EVENT_ERROR://MQTT错误
                ESP_LOGI(MQTT, "MQTT_EVENT_ERROR");
                break;
            default:
                ESP_LOGI(MQTT, "Other event id:%d", event->event_id);
                break;
        }
    return ESP_OK;
}
/**
 * @name: mqtt_event_handler
 * @brief: MQTT处理函数
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)// @suppress("Type cannot be resolved")
{
    ESP_LOGD(MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

/**
 * @name: Check_Buffer_EmptyOrNot
 * @fuc: 检查缓存区的数据是否为空
 * @msg:
 * @param {Buffer}
 * @return:
 */
uint16_t Check_Buffer_EmptyOrNot(uint8_t *Buffer)
{
    uint16_t Count_Send_Empty_Val = 0;
    uint16_t i = 0;
    for (i = 0; i < MQTT_Resv_SensorNum; i++)
        {
            //前两位数据非空
            if (Buffer[(i * 10)] != '0' || Buffer[(i * 10) + 1] != '0'
                    || Buffer[(i * 10) + 5] != '0' || Buffer[(i * 10) + 8] != '0')
                {
                    Count_Send_Empty_Val ++;
                }
        }
    //调试用
    //printf("Count_Send_Empty_Val %d\r\n", Count_Send_Empty_Val);
    return Count_Send_Empty_Val;
}
/**
 * @name: mqtt_app_start_Task
 * @brief: MQTT_连接事件
 * @author: lzc
 * @param {type} None
 * @return: None
 * @note: 修改记录：初次创建
 */
uint32_t Heart_Beat_Count = 0;
uint32_t Data_Send_Count = 0;
uint32_t Count_15min_Val = 0 ;
uint32_t MQTT_Relay_AlarmCount = 0;
bool Receive_Over = false;
void mqtt_app_start_Task()
{
    int msg_id ;
    uint8_t Pack_Num = 0;
    char *MQTT_SendData_Payload;
    char *MQTT_HrartBeat_Payload;
    bool MQTT_Start_Flag = false;
    bool MQTT_Relay_AlarmCount_flag = false;
    //printf("DNS ADDR IS %s \r\n",  Get_DNS_ADDR_IP4());
    esp_mqtt_client_config_t mqtt_cfg =
    {
        //.host = "118.31.227.219",//MQTT 地址
        //.host = "47.98.136.66",
        .host = Get_DNS_ADDR_IP4(),//"www.dawen.ltd",//
        .port = 1883,   //MQTT端口
        .username = "",//用户名
        .password = "",//密码
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);//初始化MQTT
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);//注册事件
    while (1)
        {
            //接收到网口的二值信号量
            if (xSemaphoreTake(xSemaphore_Ethernet, (TickType_t) 10) == pdTRUE && Falg)
                {
                    //获取到信号量 ！
                    ESP_LOGI(MQTT, "MQTT_task获取二值信号量");
                    //挂起网口任务
                    ESP_LOGI(MQTT, "网口任务已经挂起");
                    vTaskSuspend(eth_CreatedTask);
                    //启动MQTT
                    ESP_LOGI(MQTT, "启动MQTT服务");
                    esp_mqtt_client_start(client);
                    MQTT_Start_Flag = true;
                    Reade_MQTT_SaveData_From_NVS();
                    //清除NRF24L01数组中的数据
                    Clear_ALL_nrf24l01_TempData();
                }
            //接收到WIFI的二值信号量
            if (xSemaphoreTake(xSemaphore_WIFI, (TickType_t) 10) == pdTRUE && Falg)
                {
                    //获取到信号量 ！
                    ESP_LOGI(MQTT, "MQTT_task获取二值信号量");
                    //挂起网口任务
                    ESP_LOGI(MQTT, "wifi任务已经删除");
                    vTaskDelete(wifi_CreatedTask);
                    Falg = false ;
                    //启动MQTT
                    ESP_LOGI(MQTT, "启动MQTT服务");
                    esp_mqtt_client_start(client);
                    MQTT_Start_Flag = true;
                    Reade_MQTT_SaveData_From_NVS();
                    //清除NRF24L01数组中的数据
                    Clear_ALL_nrf24l01_TempData();
                    Count_15min_Val = xTaskGetTickCount();
                }
            ESP_LOGI(MQTT, "mqtt_app_start_Task IS running %d", xTaskGetTickCount());
            if (MQTT_Start_Flag)
                {
                    //OTA部分的代码下发
                    if (MQTT_Resv_OTA)
                        {
                            MQTT_Resv_OTA = 0;
                            printf("\r\n Ready OTA !\r\n");
                            /* OTA升级线程 */
                            xTaskCreate(&simple_ota_example_task, "ota_example_task", 8192, NULL, 5, &OTA_Handler);
                        }
                    //判断是否到了上报时间-发送心跳包
                    if (xTaskGetTickCount() - Heart_Beat_Count >= OneMinVal)
                        {
                            sprintf(Topic, "/WSN-LW/");
                            strcat(Topic, &Read_ID);
                            strcat(Topic, "/event/Heartbeat");
                            // 发送心跳包
                            Heart_Beat_Count = xTaskGetTickCount();
                            MQTT_HrartBeat_Payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat, 0);
                            msg_id = esp_mqtt_client_publish(client, Topic, MQTT_HrartBeat_Payload, 0, 0, 0);
                            // 释放内存
                            cJSON_Delete(cjson_Object);
                            ESP_LOGI(MQTT, "mqtt_json data is %s", MQTT_HrartBeat_Payload);
                            vTaskDelay(1000 / portTICK_PERIOD_MS);
                        }
                    //15分钟计时完成，数据全部接收完毕
                    if ((xTaskGetTickCount() - Count_15min_Val >= 15 * OneMinVal)
                            || (Check_Buffer_EmptyOrNot(DataToSendBuffer) == MQTT_Resv_SensorNum) || (Receive_Over))
                        {
                            Receive_Over = true;
                            //数据上报事件发送,超时数据清除 ,到设定上报的时间
                            if (xTaskGetTickCount() - Data_Send_Count >= (MQTT_Resv_Cycle * OneMinVal)
                                    || (MQTT_Resv_Read_data) || (TimeOut_Clear_Flag))
                                {
                                    TimeOut_Clear_Flag = 0;
                                    MQTT_Resv_Read_data = 0;
                                    Data_Send_Count = xTaskGetTickCount();
                                    //计算包数
                                    if (MQTT_Resv_SensorNum >= 40)
                                        {
                                            Pack_Num = MQTT_Resv_SensorNum / PacksSensorNum;
                                            if (MQTT_Resv_SensorNum % PacksSensorNum)
                                                {
                                                    Pack_Num ++;
                                                }
                                            printf("Pack_Num is ---------------: %d\r\n", Pack_Num);
                                        }
                                    else if (MQTT_Resv_SensorNum < 40)
                                        {
                                            Pack_Num = 1;
                                            Pack_Num_Last = MQTT_Resv_SensorNum;
                                        }
                                    //分包发送
                                    for (unsigned char i = 1; i <= Pack_Num; i++)
                                        {
                                            if (MQTT_Resv_SensorNum >= 40)
                                                {
                                                    MQTT_SendData_Payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData, i) ;
                                                }
                                            else
                                                {
                                                    MQTT_SendData_Payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_CountLess40, i) ;
                                                }
                                            sprintf(Topic, "/WSN-LW/");
                                            strcat(Topic, &Read_ID);
                                            strcat(Topic, "/event/Data");
                                            //发送数据包
                                            msg_id = esp_mqtt_client_publish(client, Topic, MQTT_SendData_Payload, 0, 0, 0);
                                            ESP_LOGI(MQTT, "mqtt_json data is %s", MQTT_SendData_Payload);
                                            vTaskDelay(1000 / portTICK_PERIOD_MS);
                                            // 释放内存
                                            cJSON_Delete(cjson_Object);
                                        }
                                }
                        }
                    //如果接收到MQTT的Alarm的消息
                    if (MQTT_Resv_Alarm && !MQTT_Relay_AlarmCount_flag)
                        {
                            gpio_set_direction(Relay_PIN, GPIO_MODE_OUTPUT);
                            gpio_set_level(Relay_PIN, 1);
                            printf("\r\n RELAY ALARM !!!!!!!!!!!!!!!!!!\r\n");
                            MQTT_Relay_AlarmCount_flag = true;
                            //MQTT_Relay_AlarmCount_flag = 1;
                            MQTT_Relay_AlarmCount = xTaskGetTickCount();
                        }
                    //如果Alarm下发停止或者到了时间
                    else if ((! MQTT_Resv_Alarm
                              || xTaskGetTickCount() - MQTT_Relay_AlarmCount >= (MQTT_Resv_AlarmTime * OneMinVal))
                             && MQTT_Relay_AlarmCount_flag)
                        {
                            gpio_set_direction(Relay_PIN, GPIO_MODE_OUTPUT);
                            gpio_set_level(Relay_PIN, 0);
                            printf("\r\n  Clear RELAY ALARM !!!!!!!!!!!!!!!!!!\r\n");
                            MQTT_Resv_Alarm = 0 ;
                            //MQTT_Relay_AlarmCount_flag = 1;
                            MQTT_Relay_AlarmCount_flag = false;
                        }
                }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}

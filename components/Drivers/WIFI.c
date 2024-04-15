#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <lwip/netdb.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "WIFI.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_smartconfig.h"


#define EXAMPLE_ESP_MAXIMUM_RETRY  3

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

#define SC_CONNECTED_BIT        BIT0
#define SC_ESPTOUCH_DONE_BIT    BIT1

static const char *TAG = "SmartConfig";
static EventGroupHandle_t Def_wifi_event_group;
static EventGroupHandle_t SC_wifi_event_group;
static const char *WIFI_TAG = "wifi Process:";
static int Def_wifi_retry_num = 0;


static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(WIFI_TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } 
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(WIFI_TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (Def_wifi_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
        {            
            ESP_LOGI(WIFI_TAG, "retry to connect to the AP");
           	vTaskDelay(2000/portTICK_PERIOD_MS);//延时一段时间后重试
            esp_wifi_connect();
            Def_wifi_retry_num++;

        }
        else 
        {
            xEventGroupSetBits(Def_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGI(WIFI_TAG,"connect to the AP fail");
        }

    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        Def_wifi_retry_num = 0;
        xEventGroupSetBits(Def_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}



static void smartconfig_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1) 
    {
        uxBits = xEventGroupWaitBits(SC_wifi_event_group, SC_CONNECTED_BIT | SC_ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & SC_CONNECTED_BIT) 
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & SC_ESPTOUCH_DONE_BIT) 
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}


static void sc_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        esp_wifi_connect();
        xEventGroupClearBits(SC_wifi_event_group, SC_CONNECTED_BIT);
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(SC_wifi_event_group, SC_CONNECTED_BIT);
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) 
    {
        ESP_LOGI(TAG, "Scan done");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) 
    {
        ESP_LOGI(TAG, "Found channel");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) 
    {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }
        ESP_LOGI(TAG, "SSID:%s", evt->ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", evt->password);

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) 
    {
        xEventGroupSetBits(SC_wifi_event_group, SC_ESPTOUCH_DONE_BIT);
    }
}



void SmartConfig_Init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    ESP_ERROR_CHECK(esp_netif_init());
    Def_wifi_event_group = xEventGroupCreate();
    SC_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    wifi_config_t Savedconfig;

    // 获取已经保存WIFI的配置
    esp_wifi_get_config(ESP_IF_WIFI_STA, &Savedconfig);

    // bzero(&Savedconfig, sizeof(wifi_config_t));

     if(strlen((char*)Savedconfig.sta.ssid) > 0)
    {
        ESP_LOGI(TAG, "Last Connect WiFi SSID:%s", Savedconfig.sta.ssid);
        ESP_LOGI(TAG, "Last Connect WiFi Password:%s", Savedconfig.sta.password);

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;

        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            &instance_got_ip));

        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &Savedconfig) );
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    else
    {
            SC_wifi_event_group = xEventGroupCreate();
        // 直接使用SC
        xEventGroupSetBits(Def_wifi_event_group, WIFI_FAIL_BIT);
    }

    EventBits_t bits = xEventGroupWaitBits(Def_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) 
    {
        ESP_LOGI(WIFI_TAG, "Connect Sucess ");
    } 
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGW(WIFI_TAG, "SmartConfig Mode Start !!");
        ESP_LOGW(TAG, "Please Make Your Phone Connnect Targer WIFI ");
        ESP_LOGW(TAG, "if You has Connectted Targer WIFI,Then Turn On Your Phone [ GPS ] and Open [ ESPTOUCH APP]");

        // ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_stop() );

        ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL) );
        ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &sc_event_handler, NULL) );
        ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL) );

        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK( esp_wifi_start() ); 
    } 
    else 
    {
        ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
    }
}
































































































































#if 0

// ESP32系列--第四篇 WiFi概述
// https://blog.csdn.net/tianizimark/article/details/124689134?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-124689134-blog-129145100.235%5Ev33%5Epc_relevant_default_base3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-124689134-blog-129145100.235%5Ev33%5Epc_relevant_default_base3&utm_relevant_index=2
void WIFI_Init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    Def_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());//LWIP
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));


    wifi_config_t wifi_APconfig =
    {
        //AP参数
        .ap.ssid = "ESP32-S3-AP",
        .ap.password = "12345678",
        .ap.ssid_len = 0,
        .ap.channel = 1,
        .ap.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        .ap.ssid_hidden = false,
        .ap.max_connection = 5,
        .ap.beacon_interval = 100,
    };

    wifi_config_t wifi_STAconfig =
    {
        //STA参数
        .sta.ssid = "Redmi_AX3000",
        .sta.password = "9.1.502.",
        .sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        .sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH,

    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_STAconfig));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP,&wifi_APconfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(Def_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) 
    {
        ESP_LOGI(WIFI_TAG, "connected to ap ");
    } 
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(WIFI_TAG, "Failed to connect to ");
    } 
    else 
    {
        ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
    }

    //start_webserver();

    // esp_wifi_start();

    // esp_wifi_stop();

    // esp_wifi_connect();//连接目标热点
    // esp_wifi_disconnect();//断开热点
    // esp_wifi_scan_start();
    // esp_wifi_scan_get_ap_records();
}
#endif


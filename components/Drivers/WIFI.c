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
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "TemperatureSensor.h"

#if 1
// MAX retry Times 
#define EXAMPLE_ESP_MAXIMUM_RETRY  3

//MAX WIFI name lists
#define SCAN_LIST_SIZE  20

//spiffs path
#define INDEX_HTML_PATH "/spiffs/index.html"

#define WIFI_AP_CONNECTED_BIT       BIT0
#define WIFI_STA_CONNECTED_BIT      BIT1

static EventGroupHandle_t Def_wifi_event_group;
static const char *WIFI_TAG = "wifi Process";
static int Def_wifi_retry_num = 0;
httpd_handle_t server = NULL;
int led_state = 0;
char *index_html;
struct async_resp_arg 
{
    httpd_handle_t hd;
    int fd;
};


static void initi_web_page_buffer(void)
{
    struct stat st;
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    ESP_LOGI(WIFI_TAG, "Mount SPIFS");
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    size_t total = 0, used = 0;
    //Get spiffs all size and remain size
    esp_err_t ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(WIFI_TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } 
    else 
    {
        ESP_LOGI(WIFI_TAG, "Partition size: total: %d, used: %d", total, used);
    }


    if (stat(INDEX_HTML_PATH, &st))
    {
        ESP_LOGE(WIFI_TAG, "index.html not found");
        return;
    }

    ESP_LOGI(WIFI_TAG, "index.html Szie %ld",st.st_size);

    if(st.st_size > 20*1024)
    {
        ESP_LOGE(WIFI_TAG, "index.html Too Large,OverSzie 20 KB");
        return;
    }

    index_html = malloc(st.st_size*2);

    if((index_html == NULL))
    {
        ESP_LOGE(WIFI_TAG, "Memery Not enough");
        return;
    }

    memset((void *)index_html, 0, st.st_size*2);

    FILE *fp = fopen(INDEX_HTML_PATH, "r");

    if (fread(index_html, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(WIFI_TAG, "fread failed");
    }
    fclose(fp);

    ESP_LOGI(WIFI_TAG, "Unmount SPIFS");
    ESP_ERROR_CHECK(esp_vfs_spiffs_unregister(NULL));
}

esp_err_t get_req_handler(httpd_req_t *req)
{
    int response;

    response = httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);//上传html界面
    return response;
}

static void ws_async_send(void *arg)
{
    httpd_ws_frame_t ws_pkt;
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;

    led_state = !led_state;
    // gpio_set_level(LED_PIN, led_state);
    
    char buff[4];
    float Temp;
    xQueueReceive(TemperatureSensor_Queue,(void *)&Temp,0);
    sprintf(buff,"%3.1f",Temp);
    
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)buff;
    ws_pkt.len = strlen(buff);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = max_clients;
    int client_fds[max_clients];

    esp_err_t ret = httpd_get_client_list(server, &fds, client_fds);

    if (ret != ESP_OK) {
        return;
    }

    for (int i = 0; i < fds; i++) {
        int client_info = httpd_ws_get_fd_info(server, client_fds[i]);
        if (client_info == HTTPD_WS_CLIENT_WEBSOCKET) {
            httpd_ws_send_frame_async(hd, client_fds[i], &ws_pkt);
        }
    }
    free(resp_arg);
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    return httpd_queue_work(handle, ws_async_send, resp_arg);
}

static esp_err_t handle_ws_req(httpd_req_t *req)
{
    char Type[2];
    int32_t TypeNum;
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(WIFI_TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len)
    {
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL)
        {
            ESP_LOGE(WIFI_TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(WIFI_TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(WIFI_TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    else
    {
        ESP_LOGI(WIFI_TAG, "frame len is 0");
        return ESP_FAIL;
    }

        // esp_wifi_disconnect();
        // vTaskDelay(100);
        // wifi_config_t wifi_STAconfig =
        // {
        //     //STA参数
        //     .sta.ssid = "618_封闭�?",
        //     .sta.password = "618618618",
        //     .sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        //     .sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        // };
        // ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_STAconfig));
        // esp_wifi_connect();
        // ESP_LOGI(WIFI_TAG, "===== Connect");

    ESP_LOGI(WIFI_TAG, "frame len is %d", ws_pkt.len);

    // mmecpy(Type,ws_pkt.payload,sizeof(Type));
    // TypeNum = atoi(Type);
    // ESP_LOGI(WIFI_TAG, "Type Num %d", ws_pkt.payload[0]);
    // switch (ws_pkt.type)
    // {
    //     case HTTPD_WS_TYPE_CONTINUE:
    //         /* code */
    //         break;
    //     case HTTPD_WS_TYPE_TEXT:
    //         // if(strcmp((char *)ws_pkt.payload, "toggle") == 0)
    //         // {

    //         // }
    //         // else if()
    //         // {

    //         // }
    //         // break;
    //     case HTTPD_WS_TYPE_BINARY:
    //         /* code */
    //         break;
    //     case HTTPD_WS_TYPE_CLOSE:
    //         /* code */
    //         break;
    //     case HTTPD_WS_TYPE_PING:
    //         /* code */
    //         break;
    //     case HTTPD_WS_TYPE_PONG:
    //         /* code */
    //         break;                   
    //     default:
    //         break;
    // }

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT )
        // strcmp((char *)ws_pkt.payload, "toggle") == 0)
    {
        free(buf);
        return trigger_async_send(req->handle, req);
    }
    return ESP_OK;
}

httpd_handle_t setup_websocket_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_req_handler,
        .user_ctx = NULL};

    httpd_uri_t ws = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = handle_ws_req,
        .user_ctx = NULL,
        .is_websocket = true};

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &ws);
    }

    return server;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) 
    {
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                wifi_event_sta_disconnected_t *DisConevent = (wifi_event_sta_disconnected_t *)event_data;

                ESP_LOGI(WIFI_TAG, "Disconnnect reason: 0x%x",DisConevent->reason);
                if (Def_wifi_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
                {            
                    ESP_LOGI(WIFI_TAG, "retry to connect to the WIFI");
                    vTaskDelay(1000/portTICK_PERIOD_MS);
                    esp_wifi_connect();
                    Def_wifi_retry_num++;
                }
                else
                {
                    Def_wifi_retry_num = 0XFF;
                }
                break; 

            case WIFI_EVENT_AP_STADISCONNECTED:
                ESP_LOGI(WIFI_TAG, "客户端断开");
                if(index_html != NULL)
                {
                    free(index_html);
                    index_html = NULL;
                }
                break;

            case WIFI_EVENT_AP_STACONNECTED:
                ESP_LOGI(WIFI_TAG, "客户端连接");
                break; 

            default:
                break;
        }
    } 
    else if (event_base == IP_EVENT) 
    {
        switch (event_id)
        {
             case IP_EVENT_STA_GOT_IP:
                ip_event_got_ip_t* IPevent = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(WIFI_TAG, "Get IP:" IPSTR, IP2STR(&IPevent->ip_info.ip));
                Def_wifi_retry_num = 0;
                xEventGroupSetBits(Def_wifi_event_group, WIFI_STA_CONNECTED_BIT);
                break;
             case IP_EVENT_AP_STAIPASSIGNED:
                ip_event_got_ip_t *APConevent = (ip_event_got_ip_t *)event_data;
                ESP_LOGI(WIFI_TAG, "客户端-IP:" IPSTR, IP2STR(&APConevent->ip_info.ip));
                xEventGroupSetBits(Def_wifi_event_group, WIFI_AP_CONNECTED_BIT);//客户端连接成功
                break;

            default:
                break;
        }
    }
}

static void Web_task()
{
    while(1) 
    {
        xEventGroupWaitBits(Def_wifi_event_group,WIFI_AP_CONNECTED_BIT,pdTRUE,pdFALSE,portMAX_DELAY);
        led_state = 0;
        ESP_LOGI(WIFI_TAG, "WebSocket Web Server is running\n");      
        initi_web_page_buffer();
        setup_websocket_server();
        break;
    }
    vTaskDelete(NULL);
}


static void wifi_scan(void)
{
    uint16_t number = SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(WIFI_TAG, "Total WIFI scanned = %u", ap_count);
    for (int i = 0; (i < SCAN_LIST_SIZE) && (i < ap_count); i++) 
    {
        ESP_LOGI(WIFI_TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(WIFI_TAG, "RSSI \t\t%d dB", ap_info[i].rssi);
        ESP_LOGI(WIFI_TAG, "Channel \t\t%d\n", ap_info[i].primary);
    }
    ESP_ERROR_CHECK( esp_wifi_stop() );
    ESP_LOGI(WIFI_TAG, "Scan Done");
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

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    // wifi_scan();//Scan WIFI

    wifi_config_t wifi_STAconfig =
    {
        //STA参数
        .sta.ssid = "MeterSoft",
        .sta.password = "MeterSoft",
        .sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        .sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
    };

    wifi_config_t wifi_APconfig =
    {
        //AP Parament
        .ap.ssid = "ESP32_S3_AP",
        .ap.password = "00000000",
        .ap.ssid_len = 0,
        .ap.channel = 1,
        .ap.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        .ap.ssid_hidden = false,
        .ap.max_connection = 5,
        .ap.beacon_interval = 200,
    };

    // get last connect info
    wifi_config_t Savedconfig;
    esp_wifi_get_config(ESP_IF_WIFI_STA, &Savedconfig);
    ESP_LOGI(WIFI_TAG, "Last Connect WiFi SSID:%s", Savedconfig.sta.ssid);
    ESP_LOGI(WIFI_TAG, "Last Connect WiFi Password:%s", Savedconfig.sta.password);


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));// AP+STA Mode
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_STAconfig));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_APconfig));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL));
    ESP_ERROR_CHECK(esp_wifi_start());// Start WIFI

    xTaskCreate(Web_task, "Web_task", 4096*2, NULL, 10, NULL);

    // EventBits_t bits = xEventGroupWaitBits(Def_wifi_event_group,
    //         WIFI_STA_CONNECTED_BIT,
    //         pdTRUE,
    //         pdFALSE,
    //         portMAX_DELAY);

    // if (bits & WIFI_STA_CONNECTED_BIT) 
    // {
    //     ESP_LOGI(WIFI_TAG, "Connect Sucess ");
    // } 
    // else 
    // {
    //     ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
    // }
}

#endif



#if 0
#define SC_CONNECTED_BIT        BIT0
#define SC_ESPTOUCH_DONE_BIT    BIT1

// ESP_LOGW(WIFI_TAG, "SmartConfig Mode Start !!");
// ESP_LOGW(WIFI_TAG, "Please Make Your Phone Connnect Targer WIFI ");
// ESP_LOGW(WIFI_TAG, "if You has Connectted Targer WIFI,Then Turn On Your Phone [ GPS ] and Open [ ESPTOUCH APP]");
// ESP_ERROR_CHECK( esp_wifi_stop() );
// //Create SmartConfig Event Process
// ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL) );
// ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &sc_event_handler, NULL) );
// ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL) );

// // ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
// ESP_ERROR_CHECK( esp_wifi_start() ); 
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
            ESP_LOGI(WIFI_TAG, "WiFi Connected to ap");
        }
        if(uxBits & SC_ESPTOUCH_DONE_BIT) 
        {
            ESP_LOGI(WIFI_TAG, "smartconfig over");
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
        ESP_LOGI(WIFI_TAG, "Scan done");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) 
    {
        ESP_LOGI(WIFI_TAG, "Found channel");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) 
    {
        ESP_LOGI(WIFI_TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }
        ESP_LOGI(WIFI_TAG, "SSID:%s", evt->ssid);
        ESP_LOGI(WIFI_TAG, "PASSWORD:%s", evt->password);

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) 
    {
        xEventGroupSetBits(SC_wifi_event_group, SC_ESPTOUCH_DONE_BIT);
    }
}
#endif


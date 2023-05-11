#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "HTTP_Server.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_MAXIMUM_RETRY  3
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "wifi station + AP";
static int s_retry_num = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } 
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
     {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }

}






// ESP32ϵ��--����ƪ WiFi����
// https://blog.csdn.net/tianizimark/article/details/124689134?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-124689134-blog-129145100.235%5Ev33%5Epc_relevant_default_base3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-124689134-blog-129145100.235%5Ev33%5Epc_relevant_default_base3&utm_relevant_index=2
void WIFI_Init()
{

    // wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    // uint16_t number = WIFI_MAX_SCAN_NUM;
    // wifi_ap_record_t ap_info[WIFI_MAX_SCAN_NUM];
    // uint16_t ap_count = 0;
    // memset(ap_info, 0, sizeof(ap_info));


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());
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
        //AP����
        .ap.ssid = "ESP32-S3-AP",
        .ap.password = "12345678",
        .ap.ssid_len = 0,
        .ap.channel = 1,
        .ap.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        .ap.ssid_hidden = false,
        .ap.max_connection = 4,
        .ap.beacon_interval = 100,
    };

    wifi_config_t wifi_STAconfig =
    {
        //STA����
        .sta.ssid = "CMCC-idhx",
        .sta.password = "uf9mije6",
        .sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        .sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH,

    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_STAconfig));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP,&wifi_APconfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap ");
    } 
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to ");
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }



    // esp_wifi_start();

    // esp_wifi_stop();

    // esp_wifi_connect();//����Ŀ���ȵ�
    // esp_wifi_disconnect();//�Ͽ��ȵ�
    // esp_wifi_scan_start();
    // esp_wifi_scan_get_ap_records();

}






void HTTP_Server_Init(void)
{


}



/* URI ���������ڿͻ��˷��� GET /uri ����ʱ������ */
esp_err_t get_handler(httpd_req_t *req)
{
    /* ���ͻؼ򵥵���Ӧ���ݰ� */
    const char resp[] = "URI GET Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI ���������ڿͻ��˷��� POST/uri ����ʱ������ */
esp_err_t post_handler(httpd_req_t *req)
{
    /* ���� HTTP POST �������ݵ�Ŀ�껺����
     * httpd_req_recv() ֻ���� char* ���ݣ���Ҳ������
     * ������������ݣ���Ҫ����ת����
     * �����ַ������ݣ�null ��ֹ���ᱻʡ�ԣ�
     * content_len ������ַ����ĳ��� */
    char content[100];

    /* ������ݳ��ȴ��ڻ�������ض� */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* ���� 0 ��ʾ�����ѹر� */
        /* ����Ƿ�ʱ */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* ����ǳ�ʱ�����Ե��� httpd_req_recv() ����
             * ���������������ֱ��
             * ��Ӧ HTTP 408������ʱ��������ͻ��� */
            httpd_resp_send_408(req);
        }
        /* ��������˴��󣬷��� ESP_FAIL ����ȷ��
         * �ײ��׽��ֱ��ر� */
        return ESP_FAIL;
    }

    /* ���ͼ򵥵���Ӧ���ݰ� */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* GET /uri �� URI ����ṹ */
httpd_uri_t uri_get = {
    .uri      = "/uri",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* POST/uri �� URI ����ṹ */
httpd_uri_t uri_post = {
    .uri      = "/uri",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

/* ���� Web �������ĺ��� */
httpd_handle_t start_webserver(void)
{
    /* ����Ĭ�ϵ����ò��� */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* �ÿ� esp_http_server ��ʵ����� */
    httpd_handle_t server = NULL;

    /* ���� httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* ע�� URI ������� */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
    /* �������������ʧ�ܣ����صľ���� NULL */
    return server;
}

/* ֹͣ Web �������ĺ��� */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* ֹͣ httpd server */
        httpd_stop(server);
    }
}




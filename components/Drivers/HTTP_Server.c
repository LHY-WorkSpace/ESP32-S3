#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "HTTP_Server.h"




// ESP32ϵ��--����ƪ WiFi����
// https://blog.csdn.net/tianizimark/article/details/124689134?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-124689134-blog-129145100.235%5Ev33%5Epc_relevant_default_base3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-124689134-blog-129145100.235%5Ev33%5Epc_relevant_default_base3&utm_relevant_index=2
void WIFI_Init()
{

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    uint16_t number = WIFI_MAX_SCAN_NUM;
    wifi_ap_record_t ap_info[WIFI_MAX_SCAN_NUM];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));


    wifi_config_t wifi_config =
    {
        //AP����
        .ap.ssid = "BT-7274-AP",
        .ap.password = "123456",
        .ap.ssid_len = 0,
        .ap.channel = 1,
        .ap.authmode = WIFI_AUTH_WPA2_PSK,
        .ap.ssid_hidden = false,
        .ap.max_connection = 4,
        .ap.beacon_interval = 100,

        //STA����
        .sta.ssid = "BT-7274-STA",
        .sta.password = "654321",
        .sta.bssid_set = true,

    };

    esp_wifi_init(&wifi_cfg);
    esp_wifi_set_mode(WIFI_MODE_APSTA);//STA/AP
    //esp_wifi_get_mode();��ȡ��ǰģʽ
    esp_wifi_set_config(WIFI_IF_STA,&wifi_config);
    esp_wifi_set_config(WIFI_IF_AP,&wifi_config);

    esp_wifi_start();

    // esp_wifi_stop();

    esp_wifi_connect();//����Ŀ���ȵ�
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




#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "HTTP_Server.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "LVGL_UI.h"


//================= WIFI ===============================
#define EXAMPLE_ESP_MAXIMUM_RETRY  3
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "wifi station + AP";
static int s_retry_num = 0;


//================= Client ===============================

// #if defined(CONFIG_EXAMPLE_IPV4)
// #define SERVER_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
// #elif defined(CONFIG_EXAMPLE_IPV6)
// #define SERVER_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
// #else
// #define SERVER_IP_ADDR "192.168.1.2"
// #endif

#define CONFIG_EXAMPLE_IPV4
#define SERVER_IP_ADDR "192.168.1.2"
#define PORT 6666
static const char *Client_TAG = "Client Info";
static const char *payload = " ESP32 Send this Message";








// https://www.cnblogs.com/kerwincui/p/13958590.html
/* URI 处理函数，在客户端发起 GET /uri 请求时被调用 */
esp_err_t get_handler(httpd_req_t *req)
{
    /* 发送回简单的响应数据包 */
    const char resp[] = "URI GET Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI 处理函数，在客户端发起 POST/uri 请求时被调用 */
esp_err_t post_handler(httpd_req_t *req)
{
    /* 定义 HTTP POST 请求数据的目标缓存区
     * httpd_req_recv() 只接收 char* 数据，但也可以是
     * 任意二进制数据（需要类型转换）
     * 对于字符串数据，null 终止符会被省略，
     * content_len 会给出字符串的长度 */
    char content[100];

    /* 如果内容长度大于缓冲区则截断 */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 返回 0 表示连接已关闭 */
        /* 检查是否超时 */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* 如果是超时，可以调用 httpd_req_recv() 重试
             * 简单起见，这里我们直接
             * 响应 HTTP 408（请求超时）错误给客户端 */
            httpd_resp_send_408(req);
        }
        /* 如果发生了错误，返回 ESP_FAIL 可以确保
         * 底层套接字被关闭 */
        return ESP_FAIL;
    }

    /* 发送简单的响应数据包 */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* GET /uri 的 URI 处理结构 */
httpd_uri_t uri_get = {
    .uri      = "/uri",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* POST/uri 的 URI 处理结构 */
httpd_uri_t uri_post = {
    .uri      = "/uri",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

/* 启动 Web 服务器的函数 */
httpd_handle_t start_webserver(void)
{
    /* 生成默认的配置参数 */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* 置空 esp_http_server 的实例句柄 */
    httpd_handle_t server = NULL;

    /* 启动 httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* 注册 URI 处理程序 */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
    /* 如果服务器启动失败，返回的句柄是 NULL */
    return server;
}

/* 停止 Web 服务器的函数 */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* 停止 httpd server */
        httpd_stop(server);
    }
}












static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
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

    s_wifi_event_group = xEventGroupCreate();
    
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
    if (bits & WIFI_CONNECTED_BIT) 
    {
        ESP_LOGI(TAG, "connected to ap ");
    } 
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to ");
    } 
    else 
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    //start_webserver();

    // esp_wifi_start();

    // esp_wifi_stop();

    // esp_wifi_connect();//连接目标热点
    // esp_wifi_disconnect();//断开热点
    // esp_wifi_scan_start();
    // esp_wifi_scan_get_ap_records();

}










void HTTP_Server_Init(void)
{


}











static void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = SERVER_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) 
    {
#if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
        struct sockaddr_in6 dest_addr = { 0 };
        inet6_aton(host_ip, &dest_addr.sin6_addr);
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_STREAM, &ip_protocol, &addr_family, &dest_addr));
#endif
        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);

        if (sock < 0) 
        {
            ESP_LOGE(Client_TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(Client_TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));

        if (err != 0) 
        {
            ESP_LOGE(Client_TAG, "Socket unable to connect: errno %d", errno);
            break;
        }
        ESP_LOGI(Client_TAG, "Successfully connected");

        err = send(sock, payload, strlen(payload), 0);

        if (err < 0) 
        {
            ESP_LOGE(Client_TAG, "Error occurred during sending: errno %d", errno);
            break;
        }

        while (1) 
        {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
            ESP_LOGI(Client_TAG, "Received %d bytes from %s:", len, host_ip);
            ESP_LOGI(Client_TAG, "%s", rx_buffer);
            
            // Error occurred during receiving
            if (len < 0) 
            {
                ESP_LOGE(Client_TAG, "recv failed: errno %d", errno);
                break;
            }
            // Data received
            else
            {
                    switch (rx_buffer[0])
                    {
                    case 'P':
                        RotateEye(0x01,1);
                        break;
                      case 'N':
                        RotateEye(0xFF,1);
                        break;   
                      case '1':
                        ChangeEyeFocalize('1'- 0x30);
                        break;  
                                       
                    default:
                        ChangeEyeFocalize(rx_buffer[0]- 0x30);
                        break;
                    }



                // if(rx_buffer[0] == 'N')
                // {
                //     printf("ESP32 Get!!\n");
                //     RotateEye();
                // }
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) 
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}





void HTTP_Clent_Init(void)
{
    WIFI_Init();
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}





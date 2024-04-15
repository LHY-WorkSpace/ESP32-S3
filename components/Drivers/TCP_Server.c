#include <stdio.h>
#include <inttypes.h>
#include <lwip/netdb.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "TCP_Server.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"


//================= Server ===============================
#define OPEN_PORT                   6666
#define KEEPALIVE_IDLE              5
#define KEEPALIVE_INTERVAL          5
#define KEEPALIVE_COUNT             5
static const char *Server_TAG = "Server:";

#define TCP_SERVER_QUEUE_LEN   (1)
QueueHandle_t Tcp_Server_Queue;


static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];
    int ip_protocol = 0;
    unsigned char Buff[12];

    do 
    {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) 
        {
            ESP_LOGE(Server_TAG, "Error occurred during receiving: errno %d", errno);
        } 
        else if (len == 0) 
        {
            ESP_LOGW(Server_TAG, "Connection closed");
        } 
        else 
        {
            switch (rx_buffer[0])
            {
                case 'R':
                    ip_protocol = atoi(&rx_buffer[1]);
                    memcpy(Buff,(unsigned char *)&ip_protocol,sizeof(ip_protocol));
                    xQueueOverwrite(Tcp_Server_Queue,Buff);
                    break;
                case 'G':
                    ip_protocol = atoi(&rx_buffer[1]);
                    memcpy((Buff+4),(unsigned char *)&ip_protocol,sizeof(ip_protocol));
                    xQueueOverwrite(Tcp_Server_Queue,Buff);
                    break;   
                case 'B':
                    ip_protocol = atoi(&rx_buffer[1]);
                    memcpy((Buff+8),(unsigned char *)&ip_protocol,sizeof(ip_protocol));
                    xQueueOverwrite(Tcp_Server_Queue,Buff);
                    break;  
                default:
                    memset(Buff,0,sizeof(Buff));
                    xQueueOverwrite(Tcp_Server_Queue,Buff);               
                break;
            }
            printf("Get %d\r\n",ip_protocol);

            // rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            // ESP_LOGI(Server_TAG, "Received %d bytes: %s", len, rx_buffer);

            // // send() can return less bytes than supplied length.
            // // Walk-around for robust implementation.
            // int to_write = len;
            // while (to_write > 0) 
            // {
            //     int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
            //     if (written < 0) 
            //     {
            //         ESP_LOGE(Server_TAG, "Error occurred during sending: errno %d", errno);
            //     }
            //     to_write -= written;
            // }
        }

    } while (len > 0);
}



static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(OPEN_PORT);
        ip_protocol = IPPROTO_IP;
    }


    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(Server_TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
// #if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
//     // Note that by default IPV6 binds to both protocols, it is must be disabled
//     // if both protocols used at the same time (used in CI)
//     setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
// #endif

    ESP_LOGI(Server_TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(Server_TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(Server_TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(Server_TAG, "Socket bound, port %d", OPEN_PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(Server_TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) 
    {

        ESP_LOGI(Server_TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) 
        {
            ESP_LOGE(Server_TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) 
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        ESP_LOGI(Server_TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}


void TCP_Server_Init(void)
{
    Tcp_Server_Queue = xQueueCreate(TCP_SERVER_QUEUE_LEN,sizeof(int)*3);
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
}




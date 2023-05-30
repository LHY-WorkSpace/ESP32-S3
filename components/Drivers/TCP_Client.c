#include <stdio.h>
#include <inttypes.h>
#include <lwip/netdb.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "TCP_Client.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "LVGL_UI.h"

//================= Client ===============================
#define CONFIG_EXAMPLE_IPV4
#define SERVER_IP_ADDR "192.168.1.2"
#define SERVER_PORT 6666
static const char *Client_TAG = "Client:";
static const char *payload = " ESP32 Online!\r\n";



static void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = SERVER_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    int err,sock;
    struct sockaddr_in dest_addr;

    while (1) 
    {
        while (1)
        {
            dest_addr.sin_addr.s_addr = inet_addr(host_ip);
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(SERVER_PORT);
            addr_family = AF_INET;
            ip_protocol = IPPROTO_IP;

            sock =  socket(addr_family, SOCK_STREAM, ip_protocol);

            if (sock < 0) 
            {
                ESP_LOGE(Client_TAG, "Unable to create socket: errno %d", errno);
            }
            else
            {
                ESP_LOGI(Client_TAG, "Socket created, connecting to %s:%d", host_ip, SERVER_PORT);

                err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));

                if (err != 0) 
                {
                    ESP_LOGE(Client_TAG, "Socket unable to connect: errno %d", errno);
                }
                else
                {
                    break;
                }
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    

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
            // ESP_LOGI(Client_TAG, "Received %d bytes from %s:", len, host_ip);
            // ESP_LOGI(Client_TAG, "%s", rx_buffer);
            // Error occurred during receiving
            if (len < 0) 
            {
                ESP_LOGE(Client_TAG, "RX Error :%d", errno);
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
                        printf("times:%d\r\n",(uint8_t)(rx_buffer[0]- 0x30));
                        ChangeEyeFocalize(rx_buffer[0]- 0x30);
                        break;
                    }
                err = send(sock, rx_buffer, strlen(rx_buffer), 0);
                if(err <0)
                {
                    send(sock, rx_buffer, strlen(rx_buffer), 0);
                }
            }

            vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        if (sock != -1) 
        {
            ESP_LOGE(Client_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    vTaskDelete(NULL);
}



void TCP_Client_Init(void)
{
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}




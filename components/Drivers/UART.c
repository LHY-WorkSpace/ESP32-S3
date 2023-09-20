/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"


#define UART_N  (UART_NUM_1)
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

#define RX_BUF_SIZE (2048)
#define TX_BUF_SIZE (2048)


void UART_Init(void) 
{
    const uart_config_t uart_config = 
    {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_N, RX_BUF_SIZE * 2, TX_BUF_SIZE * 2, 0, NULL, 0);
    uart_param_config(UART_N, &uart_config);
    uart_set_pin(UART_N, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_N, data, len);
    return txBytes;
}

void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";

    while (1) 
    {
        sendData(TX_TASK_TAG, "Hello world");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";

    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) 
    {
        const int rxBytes = uart_read_bytes(UART_N, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) 
        {
            data[rxBytes] = 0;
        }
    }
    free(data);
}

// void app_main(void)
// {
//     UART_Init();
//     xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
//     xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
// }

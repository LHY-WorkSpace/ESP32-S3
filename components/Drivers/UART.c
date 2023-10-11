/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "TemperatureSensor.h"
#include "DataType.h"
#include "led_strip.h"
#define UART_N  (UART_NUM_1)
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

#define RX_BUF_SIZE (4*1024)
#define TX_BUF_SIZE (4*1024)

#define RX_FLAG (Bit_0)
#define TX_FLAG (Bit_1)


EventGroupHandle_t USART_EventGroup;
QueueHandle_t Event_Queue;
extern EventGroupHandle_t LED_EventGroup;

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


	Event_Queue = xQueueCreate(2,RX_BUF_SIZE);
	if( Event_Queue == NULL)
	{
		printf("Event_Queue  Err\n");
	}

    //32 Bits you can use
    USART_EventGroup = xEventGroupCreate();

    if(USART_EventGroup == NULL)
    {
        printf("USART_EventGroup Err!!\n");
        while (1);
    }
}
    int rxBytes = 0;
static void tx_task(void *arg)
{
    int txBytes = 0;
	float Sta;
	UBaseType_t MsgNum;
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) 
    {
        //读后清零:pdTRUE
        //等待所有位触发:pdFALSE
		MsgNum = uxQueueMessagesWaiting(TemperatureSensor_Queue);
		if(MsgNum != 0)
		{
            memset(data,0,sizeof(RX_BUF_SIZE));
            // 	printf("Msg Num %d \n",MsgNum);
            xQueueReceive(TemperatureSensor_Queue,(void *)data,0);
            memcpy((uint8_t*)&Sta,data,sizeof(float));
            sprintf((char *)data,"%f",*((float *)data));
            if(Sta >= 35.5)
            {
                xEventGroupSetBits(LED_EventGroup,Bit_0);    
            }
            // xEventGroupWaitBits(USART_EventGroup,TX_FLAG,pdTRUE,pdFALSE,portMAX_DELAY);
            txBytes = uart_write_bytes(UART_N, data, sizeof(float));
        }
        else
        {
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
    }
}

static void rx_task(void *arg)
{
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    int length = 0;

    while (1) 
    {
        uart_get_buffered_data_len(UART_N,(size_t*)&length);
        memset(data,0,sizeof(RX_BUF_SIZE));
        if (length != 0)
        {
            rxBytes = length;
            uart_read_bytes(UART_N, data, RX_BUF_SIZE, 0);
            // memcpy(data,0,sizeof(RX_BUF_SIZE));
            // xEventGroupSetBits(USART_EventGroup,TX_FLAG);
		    xQueueSendToBack(Event_Queue,(void *)data,0);
            printf("RX %d Bytes   %d\n!!\n",rxBytes,length);
        }
        else
        {
            vTaskDelay(50/portTICK_PERIOD_MS);
        }
    }
    free(data);
}

void UART_Task(void)
{
    xTaskCreate(rx_task, "uart_rx_task", 1024*15, NULL, 10, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*4, NULL, 9, NULL);
    // vTaskDelete(NULL);
}

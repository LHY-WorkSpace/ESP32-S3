#include "main.h"
#if 0
// TimerHandle_t Timer1;

// void Watch()
// {
// 	BaseType_t Sta;
// 	float Temp = 0.0;
// 	UBaseType_t MsgNum;

//     while (1)
//     {    

// 		// MsgNum = uxQueueMessagesWaiting(TemperatureSensor_Queue);
// 		// if(MsgNum != 0)
// 		// {
// 		// 	printf("Msg Num %d \n",MsgNum);
// 			Sta = xQueueReceive(TemperatureSensor_Queue,(void *)&Temp,portMAX_DELAY);
// 			if( Sta != pdPASS)
// 			{
// 				printf("RX Err !\n");
// 			}
// 			else
// 			{
// 				printf("Temperature is %.1f \n", Temp);
// 			}
// 		// }


//         // vTaskDelay(20/portTICK_PERIOD_MS);
//         // taskYIELD();
        
//     }
// 	vTaskDelete(NULL);
// }

// QueueHandle_t AAAAA_q;
// QueueHandle_t SSSSS_q;
// SemaphoreHandle_t Semaphore;
MessageBufferHandle_t Sbufft;
void AAAAA()
{
	char AA[100];
	int i=0;
	int Len;
	while (1)
	{
		memset(AA,0,sizeof(AA));
		Len = sprintf(AA,"A%d Run\r\n",i);
		printf("AA %d ===\r\n",Len);
		xMessageBufferSend(Sbufft,AA,Len,portMAX_DELAY);
		printf("Free %d\r\n",xMessageBufferSpacesAvailable(Sbufft));
		// xSemaphoreGive(Semaphore);
		vTaskDelay(5000/portTICK_PERIOD_MS);
		i++;
	}
}


void SSSSS()
{
	// printf("S Start\r\n");
	char SS[100];
	int len = 0;
	while (1)
	{
		memset(SS,0,sizeof(SS));
		len = xMessageBufferReceive(Sbufft,SS,sizeof(SS),portMAX_DELAY);
		printf("SS %d----\r\n%s",len,SS);
		// xQueuePeek(AAAAA_q,ggggggg,portMAX_DELAY);//portMAX_DELAY
		// printf("SSSSS Get  Data\r\n");
		// vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}


// void DDDDD()
// {
// 	printf("D Start\r\n");
// 	char ggggggg[10];
// 	while (1)
// 	{
// 		printf("DDDDD Get  Flag\r\n");

// 		printf("DDDDD Get  Data\r\n");
// 		vTaskDelay(100/portTICK_PERIOD_MS);

// 	}
// }


// void SW_Timer_CB( TimerHandle_t xTimer )
// {
//     printf("SW Timer Run !\n");
// }

#endif

// https://blog.csdn.net/m0_50064262/article/details/120250151
void app_main(void)
{
    printf("System Online !\n");
	// GPIO_Init();

    // cJSON* cjson_root = cJSON_CreateObject();

    // /* 添加一条字符串类型的JSON数据(添加一个链表节点) */
    // cJSON_AddStringToObject(cjson_root, "name", "Jack");

    // /* 添加一条整数类型的JSON数据(添加一个链表节点) */
    // cJSON_AddNumberToObject(cjson_root, "age", 22);








    // SmartConfig_Init();
	// OTA_Init();
    //Initialize NVS
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);

    // dpp_enrollee_init();
	http_task();
	// SDIO_Init();
	// LED_Init();
	// MPU6050_Test();
	// MorseCode_Init();
	// vTaskDelay(500);
	// vTaskDelay(500);
	// vTaskDelay(500);
	// Key_Init();
    // UART_Init();
	// TemperatureSensor_Init();
	// WIFI_Init();
	// TCP_Client_Init();
	// TCP_Server_Init();
	//UDP_Client_Init();

	// FOC_GPIO_Init();
	// Timer_Init();
	// AS5600_Init();
	// LVGL_Init();
	// Camera_Init();
	// ADC_Init();

	// xTaskCreatePinnedToCore( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL,0);
	// xTaskCreatePinnedToCore( (TaskFunction_t)LED_Task,"LED_Task",4000,NULL,12,NULL,0);

	// Timer1 = xTimerCreate("Timer1",pdMS_TO_TICKS( 500 ),pdTRUE,(void *)0,SW_Timer_CB);
	// xTimerStart(Timer1,0);
	// xTaskCreate( (TaskFunction_t)LVGL_Task,"LVGL_Task",4096*3,NULL,4,NULL);
    // xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, 12, NULL);
	// xTaskCreate( (TaskFunction_t)LED_Task,"LED_Task",4096,NULL,5,NULL);
	// xTaskCreate( (TaskFunction_t)MPU6050_Test,"LED_Task",4096,NULL,5,NULL);
	// xTaskCreate( (TaskFunction_t)TemperatureSensor_Task,"Temperature",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)LEDWave_Task,"Wave_Task",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)ADC_Task,"ADC_Task",4096,NULL,12,NULL);
	// UART_Task();

	// xTaskCreate( (TaskFunction_t)Foc_CTL,"FOC_Task",4096*2,NULL,11,NULL);
  	// xTaskCreate( (TaskFunction_t)IIC_Test,"EE_Task",4500,NULL,11,NULL);
	// xTaskCreate( (TaskFunction_t)Watch,"Watch",4096,NULL,13,NULL);
	// xTaskCreate( (TaskFunction_t)UART_Task,"UART_Task",4096,NULL,6,NULL);


	// Sbufft = xMessageBufferCreate(100);
	// Semaphore = xSemaphoreCreateBinary();
	// xSemaphoreGive(Semaphore);
	// xTaskCreate( (TaskFunction_t)AAAAA,"AAAAA",4096,NULL,11,NULL);
	// xTaskCreate( (TaskFunction_t)SSSSS,"SSSSS",4096,NULL,11,NULL);
	// xTaskCreate( (TaskFunction_t)DDDDD,"DDDDD",4096,NULL,11,NULL);
	// vTaskList(ggggggg);
	// printf("%s\r",ggggggg);
}
 






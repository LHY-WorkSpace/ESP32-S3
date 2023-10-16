#include "main.h"


TimerHandle_t Timer1;



void Watch()
{
	BaseType_t Sta;
	float Temp = 0.0;
	UBaseType_t MsgNum;

    while (1)
    {    

		// MsgNum = uxQueueMessagesWaiting(TemperatureSensor_Queue);
		// if(MsgNum != 0)
		// {
		// 	printf("Msg Num %d \n",MsgNum);
			Sta = xQueueReceive(TemperatureSensor_Queue,(void *)&Temp,portMAX_DELAY);
			if( Sta != pdPASS)
			{
				printf("RX Err !\n");
			}
			else
			{
				printf("Temperature is %.1f \n", Temp);
			}
		// }


        // vTaskDelay(20/portTICK_PERIOD_MS);
        // taskYIELD();
        
    }
	vTaskDelete(NULL);
}



void SW_Timer_CB( TimerHandle_t xTimer )
{
    printf("SW Timer Run !\n");
}

// https://blog.csdn.net/m0_50064262/article/details/120250151
void app_main(void)
{
    printf("System Online !\n");

	// GPIO_Init();
	SDIO_Init();
	LED_Init();
	// Key_Init();
    // UART_Init();
	// TemperatureSensor_Init();
	// WIFI_Init();
	// TCP_Client_Init();
	// TCP_Server_Init();
	//UDP_Client_Init();

	// FOC_GPIO_Init();
	// Timer_Init();
	AS5600_Init();
	LVGL_Init();
	// ADC_Init();

	// xTaskCreatePinnedToCore( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL,0);
	// xTaskCreatePinnedToCore( (TaskFunction_t)LED_Task,"LED_Task",4000,NULL,12,NULL,0);

	// Timer1 = xTimerCreate("Timer1",pdMS_TO_TICKS( 500 ),pdTRUE,(void *)0,SW_Timer_CB);
	// xTimerStart(Timer1,0);
	xTaskCreate( (TaskFunction_t)LVGL_Task,"LVGL_Task",4096*3,NULL,11,NULL);
    // xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, 12, NULL);
	// xTaskCreate( (TaskFunction_t)LED_Task,"LED_Task",4096,NULL,5,NULL);
	// xTaskCreate( (TaskFunction_t)TemperatureSensor_Task,"Temperature",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)LEDWave_Task,"Wave_Task",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)ADC_Task,"ADC_Task",4096,NULL,12,NULL);
	// UART_Task();

	// xTaskCreate( (TaskFunction_t)Foc_CTL,"FOC_Task",4096,NULL,11,NULL);
  	// xTaskCreate( (TaskFunction_t)IIC_Test,"EE_Task",4500,NULL,11,NULL);
	// xTaskCreate( (TaskFunction_t)Watch,"Watch",4096,NULL,13,NULL);
	// xTaskCreate( (TaskFunction_t)UART_Task,"UART_Task",4096,NULL,6,NULL);


}
 






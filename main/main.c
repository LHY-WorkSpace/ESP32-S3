#include "main.h"








// https://blog.csdn.net/m0_50064262/article/details/120250151
void app_main(void)
{
    printf("System Online !\n");

	// GPIO_Init();
	//SDIO_Init();
	LED_Init();

	// WIFI_Init();
	// TCP_Client_Init();
	// TCP_Server_Init();
	//UDP_Client_Init();

	FOC_GPIO_Init();
	// Timer_Init();
	AS5600_Init();
	Timer_Init();
	// LVGL_Init();
	// ADC_Init();

	// xTaskCreatePinnedToCore( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL,0);
	// xTaskCreatePinnedToCore( (TaskFunction_t)LED_Task,"LED_Task",4000,NULL,12,NULL,0);

	// xTaskCreate( (TaskFunction_t)LVGL_Task,"LVGL_Task",4096,NULL,11,NULL);
	// xTaskCreate( (TaskFunction_t)LED_Task,"LED_Task",4096,NULL,12,NULL);
	//xTaskCreate( (TaskFunction_t)TemperatureSensor_Task,"Temperature",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)LEDWave_Task,"Wave_Task",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)ADC_Task,"ADC_Task",4096,NULL,12,NULL);


	// xTaskCreate( (TaskFunction_t)FOC_Task,"FOC_Task",4096,NULL,12,NULL);
	xTaskCreate( (TaskFunction_t)Foc_CTL,"FOC_Task",4096,NULL,12,NULL);
  	//xTaskCreate( (TaskFunction_t)IIC_Test,"EE_Task",4500,NULL,11,NULL);
	// xTaskCreate( (TaskFunction_t)AS5600_Test,"AS5600_Task",4096,NULL,12,NULL);
}
 






#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Timer.h"
#include "LVGL_UI.h"
#include "GPIO.h"
#include "led_strip.h"



void LED_Task()
{
    TickType_t Time;	
    Time=xTaskGetTickCount();

    while (1)
    {
		LED_ON(10,10,10);
		vTaskDelayUntil(&Time,100/portTICK_PERIOD_MS);
		LED_OFF();
		vTaskDelayUntil(&Time,100/portTICK_PERIOD_MS);
		LED_ON(10,10,10);
		vTaskDelayUntil(&Time,100/portTICK_PERIOD_MS);
		LED_OFF();
		vTaskDelayUntil(&Time,1500/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);
}





void app_main(void)
{
    printf("System Online !\n");

	// GPIO_Init();
	Timer_Init();
	LED_Init();
	LVGL_Init();

	MainUICreate();



	// xTaskCreatePinnedToCore( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL,0);
	// xTaskCreatePinnedToCore( (TaskFunction_t)LED_Task,"LED_Task",4000,NULL,12,NULL,0);

	xTaskCreate( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL);
	xTaskCreate( (TaskFunction_t)LED_Task,"LED_Task",4000,NULL,12,NULL);
}
















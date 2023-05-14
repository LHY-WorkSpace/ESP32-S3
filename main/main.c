#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Timer.h"
#include "LVGL_UI.h"
#include "GPIO.h"
#include "led_strip.h"
#include "FOC.h"
#include "HTTP_Server.h"
#include "TemperatureSensor.h"
#include "MathFun.h"




float Val[3];
void SetAngle(int32_t Phe,float Angle)
{
	switch (Phe)
	{
		case 0:
			Val[0] = FastSin(DEGTORAD(Angle-120.0f));
			break;
		case 1:
			Val[1] = FastSin(DEGTORAD(Angle));
			break;	
		case 2:
			Val[2] = FastSin(DEGTORAD(Angle+120.0f));
			break;	
		default:
			break;
	}
}



void LEDWave_Task()
{
    TickType_t Time;
    Time=xTaskGetTickCount();
	static float angleTemp = 0.0f;
    while (1)
    {    
		for (int32_t i = 0; i < 3; i++)
		{
			SetAngle(i,angleTemp);
		}

		LED_ON((uint32_t)(Val[0]*100+100)/10,(uint32_t)(Val[1]*100+100)/10,(uint32_t)(Val[2]*100+100)/10);
		
		angleTemp ++;
		if(angleTemp > 359.0f)
		{
			angleTemp = 0.0f;
		}

        vTaskDelayUntil(&Time,5/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);

}






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




// https://blog.csdn.net/m0_50064262/article/details/120250151
void app_main(void)
{
    printf("System Online !\n");

	// GPIO_Init();
	Timer_Init();
	LED_Init();
	// WIFI_Init();
	// LVGL_Init();

	// MainUICreate();


	// xTaskCreatePinnedToCore( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL,0);
	// xTaskCreatePinnedToCore( (TaskFunction_t)LED_Task,"LED_Task",4000,NULL,12,NULL,0);
	// FOC_main();
	// xTaskCreate( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL);
	//xTaskCreate( (TaskFunction_t)LED_Task,"LED_Task",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)TemperatureSensor_Task,"Temperature",4096,NULL,12,NULL);
	xTaskCreate( (TaskFunction_t)LEDWave_Task,"Wave_Task",4096,NULL,12,NULL);

	

}
















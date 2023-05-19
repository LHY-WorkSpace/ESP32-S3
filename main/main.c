#include "main.h"

#define iq	(10.0f)
#define id	(10.0f)

float IA(float Angle)
{
	float Temp;
	Temp = id * FastCos(DEGTORAD(Angle)) + iq * FastSin(DEGTORAD(Angle));
	return Temp;
}

float IB(float Angle)
{
	float Temp;
	Temp = iq *( 0.866f * FastCos(DEGTORAD(Angle)) + 0.5f * FastSin(DEGTORAD(Angle)) ) + id * (0.866f * FastSin(DEGTORAD(Angle)) - 0.5f * FastCos(DEGTORAD(Angle)));
	return Temp;
}

float IC(float Angle)
{
	float Temp;
	Temp = iq *( 0.5f * FastSin(DEGTORAD(Angle)) - 0.866f * FastCos(DEGTORAD(Angle))) + id * ( 0.5f * FastCos(DEGTORAD(Angle)) + 0.866f * FastSin(DEGTORAD(Angle)) );
	return Temp;
}



void Foc_CTL()
{
	static float Angle  = 1.0f;
    TickType_t Time;	
	float I[3];

    Time = xTaskGetTickCount();
    while (1)
    {
		I[0] = IA(Angle);
		I[1] = IB(Angle);
		I[2] = IC(Angle);
		
		Angle++;

		printf("Angle:%.2f Ia:%.2f Ib:%.2f Ic:%.2f\r\n",Angle,I[0],I[1],I[2]);

		if(Angle >= 360.0f)
		{
			Angle = 0.0f;
		}
		vTaskDelayUntil(&Time,50/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);







}












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
		SetEyeBgColorRGB((uint8_t)(Val[0]*100+100),(uint8_t)(Val[1]*100+100),(uint8_t)(Val[2]*100+100));
		angleTemp ++;
		if(angleTemp > 359.0f)
		{
			angleTemp = 0.0f;
		}

        vTaskDelayUntil(&Time,20/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);

}






void LED_Task()
{
    TickType_t Time;	
    Time=xTaskGetTickCount();

    while (1)
    {
		LED_ON(5,5,5);
		vTaskDelayUntil(&Time,100/portTICK_PERIOD_MS);
		LED_OFF();
		vTaskDelayUntil(&Time,100/portTICK_PERIOD_MS);
		LED_ON(5,5,5);
		vTaskDelayUntil(&Time,100/portTICK_PERIOD_MS);
		LED_OFF();
		vTaskDelayUntil(&Time,3000/portTICK_PERIOD_MS);
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
	FOC_GPIO_Init();
	ERRER();
	// LVGL_Init();

	// MainUICreate();

	// xTaskCreatePinnedToCore( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL,0);
	// xTaskCreatePinnedToCore( (TaskFunction_t)LED_Task,"LED_Task",4000,NULL,12,NULL,0);
	// FOC_main();
	// xTaskCreate( (TaskFunction_t)LVGL_Task,"LVGL_Task",4500,NULL,11,NULL);
	xTaskCreate( (TaskFunction_t)LED_Task,"LED_Task",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)TemperatureSensor_Task,"Temperature",4096,NULL,12,NULL);
	// xTaskCreate( (TaskFunction_t)LEDWave_Task,"Wave_Task",4096,NULL,12,NULL);

	// xTaskCreate( (TaskFunction_t)Foc_CTL,"FOC_Task",4096,NULL,12,NULL);
  //xTaskCreate( (TaskFunction_t)IIC_Test,"LVGL_Task",4500,NULL,11,NULL);
}







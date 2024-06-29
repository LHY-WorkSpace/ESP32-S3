#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "driver/temperature_sensor.h"
#include "TemperatureSensor.h"

QueueHandle_t TemperatureSensor_Queue;
static float tsens_out;
static temperature_sensor_handle_t temp_handle = NULL;
static temperature_sensor_config_t temp_sensor = 	
{
	.range_min = -10,
	.range_max = 80,
};

void TemperatureSensor_Init()
{

	ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor, &temp_handle));
	ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));

	TemperatureSensor_Queue = xQueueCreate(2,sizeof(tsens_out));

	if( TemperatureSensor_Queue == NULL)
	{
		printf("TemperatureSensor_Task  Err\n");
	}
}


void TemperatureSensor_Task()
{
	TickType_t Time;
    Time=xTaskGetTickCount();

    while (1)
    {
		ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));
		printf("Temperature is %.1f \n", tsens_out);
		xQueueSendToBack(TemperatureSensor_Queue,(void *)&tsens_out,0);
		vTaskDelayUntil(&Time,1000/portTICK_PERIOD_MS);
    }

	ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));
	temperature_sensor_uninstall(temp_handle);
	vTaskDelete(NULL);

}



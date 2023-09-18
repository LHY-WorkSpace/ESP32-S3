#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "driver/temperature_sensor.h"

QueueHandle_t TemperatureSensor_Queue;


void TemperatureSensor_Task()
{
	TickType_t Time;
	float tsens_out;
	BaseType_t Sta;	
    Time=xTaskGetTickCount();
	temperature_sensor_handle_t temp_handle = NULL;
	temperature_sensor_config_t temp_sensor = 
	{
		.range_min = -10,
		.range_max = 80,
	};
	ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor, &temp_handle));
	ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));

	TemperatureSensor_Queue = xQueueCreate(3,sizeof(tsens_out));

	if( TemperatureSensor_Queue == NULL)
	{
		printf("TemperatureSensor_Task  Err\n");
	}

    while (1)
    {
		ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));
		// printf("Temperature is %.1f \n", tsens_out);
		printf("Tx Ready\r\n ");
		Sta = xQueueSendToBack(TemperatureSensor_Queue,(void *)&tsens_out,0);
		if( Sta != pdPASS)
		{
			printf("Tx Err\r\n ");
		}
		else
		{
			printf("Tx Ok\r\n ");
		}

		vTaskDelayUntil(&Time,3000/portTICK_PERIOD_MS);
    }

	ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));
	temperature_sensor_uninstall(temp_handle);
	vTaskDelete(NULL);

}



#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/temperature_sensor.h"



void TemperatureSensor_Task()
{
	TickType_t Time;
	float tsens_out;	
    Time=xTaskGetTickCount();
	temperature_sensor_handle_t temp_handle = NULL;
	temperature_sensor_config_t temp_sensor = 
	{
		.range_min = -10,
		.range_max = 80,
	};
	ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor, &temp_handle));
	ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));

    while (1)
    {
		ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));
		printf("Temperature is %.1f \n", tsens_out);
		// set_MeterValue((int32_t )tsens_out);
		vTaskDelayUntil(&Time,1000/portTICK_PERIOD_MS);
    }

	ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));
	temperature_sensor_uninstall(temp_handle);
	vTaskDelete(NULL);

}



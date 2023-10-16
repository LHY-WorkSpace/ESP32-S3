/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "esp_log.h"
#include "esp_check.h"
#include "led_strip.h"
#include "led_strip_interface.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MathFun.h"
#include "freertos/event_groups.h"
#include "DataType.h"
static const char *TAG = "led_strip";
static led_strip_handle_t led_strip;
EventGroupHandle_t LED_EventGroup;



#define LED_GPIO    (48)
#define LED_NUM     (1)


esp_err_t led_strip_set_pixel(led_strip_handle_t strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue)
{
    ESP_RETURN_ON_FALSE(strip, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    return strip->set_pixel(strip, index, red, green, blue);
}

esp_err_t led_strip_set_pixel_rgbw(led_strip_handle_t strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue, uint32_t white)
{
    ESP_RETURN_ON_FALSE(strip, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    return strip->set_pixel_rgbw(strip, index, red, green, blue, white);
}

esp_err_t led_strip_refresh(led_strip_handle_t strip)
{
    ESP_RETURN_ON_FALSE(strip, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    return strip->refresh(strip);
}

esp_err_t led_strip_clear(led_strip_handle_t strip)
{
    ESP_RETURN_ON_FALSE(strip, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    return strip->clear(strip);
}

esp_err_t led_strip_del(led_strip_handle_t strip)
{
    ESP_RETURN_ON_FALSE(strip, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    return strip->del(strip);
}



void LED_Init()
{
    led_strip_config_t strip_config = 
    {
        .strip_gpio_num = LED_GPIO,
        .max_leds = LED_NUM, // at least one LED on board
    };

    led_strip_rmt_config_t rmt_config = 
    {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    led_strip_clear(led_strip);

    LED_EventGroup = xEventGroupCreate();
}

void LED_ON(uint32_t red, uint32_t green, uint32_t blue)
{

    printf("LED_Val %ld %ld %ld\n",red,green,blue);
    led_strip_set_pixel(led_strip, 0, red, green, blue);
    led_strip_refresh(led_strip);
}


void LED_OFF()
{
    led_strip_clear(led_strip);
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
		vTaskDelayUntil(&Time,2000/portTICK_PERIOD_MS);

    }
	vTaskDelete(NULL);
}
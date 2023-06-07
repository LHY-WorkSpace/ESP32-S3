
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_port_disp.h"
#include "LVGL_UI.h"
#include "Timer.h"
#include "Face.h"

static void MainUICreate(void)
{
    //lv_meter_1();
    // MeterTest();
    Eye_Main();
    // lv_demo_benchmark();
    // lv_demo_music();

}


void LVGL_Init()
{
    Timer_Init();
    lv_init();
	lv_port_disp_init();
    MainUICreate();

}


void LVGL_Task()
{
    TickType_t Time;
    Time=xTaskGetTickCount();
    while (1)
    {    
        //SPI�ײ����ٽ籣����Ƶ�����ô˺����ᵼ�³�ʱ��ռ��CPU��������������ִ��
		lv_task_handler();
        vTaskDelayUntil(&Time,10/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);
}













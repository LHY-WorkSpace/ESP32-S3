
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "LVGL_UI.h"
#include "Timer.h"
#include "Face.h"
#include "ui.h"

static lv_obj_t * meter;
static void set_value(void * indic, int32_t v)
{
    lv_meter_set_indicator_end_value(meter, indic, v);
}
void MeterTest()
{
    meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, 240, 240);
    lv_obj_set_style_bg_color(meter,lv_color_black(),LV_PART_MAIN);
    /*Remove the circle from the middle*/
    lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);
    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 11, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 1, 2, 30, lv_color_hex3(0xeee), 15);
    lv_meter_set_scale_range(meter, scale, 0, 100, 270, 90);
    /*Add a three arc indicator*/
    lv_meter_indicator_t * indic1 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_indicator_t * indic2 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_GREEN), -10);
    lv_meter_indicator_t * indic3 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_BLUE), -20);
    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_repeat_delay(&a, 100);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_time(&a, 250);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_set_var(&a, indic1);
    lv_anim_start(&a);
    lv_anim_set_time(&a, 500);
    lv_anim_set_playback_time(&a, 1000);
    lv_anim_set_var(&a, indic2);
    lv_anim_start(&a);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_playback_time(&a, 2000);
    lv_anim_set_var(&a, indic3);
    lv_anim_start(&a);
}





static void MainUICreate(void)
{
    //lv_meter_1();
    // MeterTest();
    // Eye_Main();
    ui_init();
    // lv_demo_benchmark();
    // lv_demo_music();

}


void LVGL_Init()
{
    Timer_Init();
    lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
    MainUICreate();

}


void LVGL_Task()
{
    TickType_t Time;
    Time=xTaskGetTickCount();
    while (1)
    {    
        //SPI底层有临界保护，频繁调用此函数会导致长时间占用CPU，导致其他任务不执行
		lv_task_handler();
        vTaskDelayUntil(&Time,5/portTICK_PERIOD_MS);
        // taskYIELD();
        
    }
	vTaskDelete(NULL);
}













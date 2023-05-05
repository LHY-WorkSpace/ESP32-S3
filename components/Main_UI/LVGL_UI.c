
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"





static lv_obj_t * meter;
static void set_value(void * indic, int32_t v)
{
    lv_meter_set_indicator_end_value(meter, indic, v);
}
void MeterTest()
{
    meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, 200, 200);
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




// static lv_obj_t * meter;
// static void set_value(void * indic, int32_t v)
// {
//     lv_meter_set_indicator_value(meter, indic, v);
//     printf("%ld \r\n",v);
// }

// void lv_meter_1(void)
// {

//     meter = lv_meter_create(lv_scr_act());
//     lv_obj_center(meter);
//     lv_obj_set_size(meter, 240, 240);

//     /*Add a scale first*/
//     lv_meter_scale_t * scale = lv_meter_add_scale(meter);
//     lv_meter_set_scale_ticks(meter, scale, 51, 2, 10, lv_palette_main(LV_PALETTE_GREEN));
//     lv_meter_set_scale_major_ticks(meter, scale, 5, 4, 15, lv_color_white(), 10);
//     lv_obj_set_style_text_color(meter,lv_palette_main(LV_PALETTE_GREEN),0);
//     lv_obj_set_style_bg_color(meter,lv_color_black(),0);

//     lv_obj_set_style_bg_color(meter,lv_color_black(),LV_PART_INDICATOR);

//     lv_meter_indicator_t * indic;

//     /*Add a blue arc to the start*/
//     indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
//     lv_meter_set_indicator_start_value(meter, indic, 0);
//     lv_meter_set_indicator_end_value(meter, indic, 10);

//     /*Make the tick lines blue at the start of the scale*/
//     indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE), false, 1);
//     lv_meter_set_indicator_start_value(meter, indic, 0);
//     lv_meter_set_indicator_end_value(meter, indic, 10);

//     /*Add a red arc to the end*/
//     indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
//     lv_meter_set_indicator_start_value(meter, indic, 80);
//     lv_meter_set_indicator_end_value(meter, indic, 100);

//     /*Make the tick lines red at the end of the scale*/
//     indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 1);
//     lv_meter_set_indicator_start_value(meter, indic, 80);
//     lv_meter_set_indicator_end_value(meter, indic, 100);

//     /*Add a needle line indicator*/
//     indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_CYAN), -10);


//     lv_obj_t *lable = lv_label_create(meter);
//     lv_label_set_text(lable,"Speed");
//     lv_obj_align(lable,LV_ALIGN_BOTTOM_MID,0,-50);
//     lv_obj_set_style_text_color(lable,lv_palette_main(LV_PALETTE_BLUE),0);


//     /*Create an animation to set the value*/
//     lv_anim_t a;
//     lv_anim_init(&a);
//     lv_anim_set_exec_cb(&a, set_value);
//     lv_anim_set_var(&a, indic);
//     lv_anim_set_values(&a, 0, 100);
//     lv_anim_set_time(&a, 3000);
//     lv_anim_set_repeat_delay(&a, 300);
//     lv_anim_set_playback_time(&a, 1000);
//     lv_anim_set_playback_delay(&a, 500);
//     lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
//     lv_anim_start(&a);
// }



void MainUICreate(void)
{
    // lv_meter_1();
    MeterTest();
}

void LVGL_Task()
{
    TickType_t Time;
    Time=xTaskGetTickCount();
    while (1)
    {    
		lv_task_handler();
        vTaskDelayUntil(&Time,5/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);
}
















#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "LVGL_UI.h"
#include "Timer.h"
#include "Face.h"
#include "ui.h"
#include "led_strip.h"

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


void img()
{
    uint8_t Code=0;
	static lv_obj_t *bgbg;

    bgbg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bgbg,240,240);
    lv_obj_set_style_bg_color(bgbg,lv_color_black(),LV_PART_MAIN);
    lv_obj_set_style_radius(bgbg,0,LV_PART_MAIN);
    lv_obj_set_style_border_side(bgbg,LV_BORDER_SIDE_FULL,LV_PART_MAIN);
    lv_obj_set_style_border_color(bgbg,lv_color_white(),LV_PART_MAIN);
    lv_obj_set_style_border_width(bgbg,0,LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bgbg,LV_SCROLLBAR_MODE_OFF);

	lv_fs_dir_t d;
    Code = lv_fs_dir_open(&d, "A:SD/Img");
	if (Code == LV_FS_RES_OK)
	{
		char b[200];
        uint8_t i;

        for ( i = 0; i < 5; i++)
        {
            memset(b, 0, 200);
            if (lv_fs_dir_read(&d, b) == LV_FS_RES_OK)
            {
                printf("%s\r\n", b);
            }
        }
        lv_obj_t * img;

        // img = lv_img_create(bgbg);
        // lv_img_set_src(img,"A:SD/Img/321.gif");

        img = lv_gif_create(bgbg);
        lv_gif_set_src(img,"A:SD/Img/321.gif");

        lv_obj_center(img);

	}
	else
	{
		printf("Err Code %d\r\n",Code);
	}		
	lv_fs_dir_close(&d);

    // LV_IMG_DECLARE(GGG321);
    //     lv_obj_t * img;

    //     // img = lv_img_create(bgbg);
    //     // lv_img_set_src(img,"A:SD/Img/321.gif");

    //     img = lv_gif_create(bgbg);
    //     lv_gif_set_src(img,&GGG321);
    //     lv_obj_center(img);

}



void QR_Code()
{
	static lv_obj_t *bgbg;
    bgbg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bgbg,240,240);
    lv_obj_set_style_bg_color(bgbg,lv_color_black(),LV_PART_MAIN);
    lv_obj_set_style_radius(bgbg,0,LV_PART_MAIN);
    lv_obj_set_style_border_side(bgbg,LV_BORDER_SIDE_FULL,LV_PART_MAIN);
    lv_obj_set_style_border_color(bgbg,lv_color_white(),LV_PART_MAIN);
    lv_obj_set_style_border_width(bgbg,0,LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bgbg,LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *QR;
    char Data[] = "123";
    QR = lv_qrcode_create(bgbg,100,lv_color_black(),lv_color_white());
    lv_qrcode_update(QR,Data,sizeof(Data));
}


static void cw_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);        //获取事件
    lv_obj_t * obj = lv_event_get_target(e);            //获取当前事件
    lv_color_t ledColor;

    switch (code)
    {
        case LV_EVENT_VALUE_CHANGED:
            ledColor = lv_colorwheel_get_rgb(obj);
            LED_ON(ledColor.ch.red,((ledColor.ch.green_h << 3) + ledColor.ch.green_l),ledColor.ch.blue);
            
            break;
        default:
            break;
    }
    // printf("event_cb %d\r\n",code);
}

void lv_colorwheel_1()
{
    lv_obj_t * cw;

    cw = lv_colorwheel_create(lv_scr_act(), true);

    // cw = lv_slider_create(lv_scr_act());
    lv_obj_set_size(cw, 200, 200);
    // lv_slider_set_range(cw, 0, 10);
    // lv_obj_add_flag(cw, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_center(cw);
    lv_obj_add_event_cb(cw, cw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);   //设置回调

    lv_group_t * group;
    //创建一个group
    group= lv_group_create();
    //将group加载到输入设备中。注意，下面这个函数的第一个参数是注册输入设备时返回的指向新输入设备的指针，不要写错了
    lv_group_add_obj(group,cw);
    lv_group_focus_obj(cw);
    lv_indev_set_group(indev_encoder,group);

}



static void MainUICreate(void)
{
    //lv_meter_1();
    // MeterTest();
    // Eye_Main();
    // ui_init();
    // img();
    // lv_demo_benchmark();
    // lv_demo_music();
    lv_colorwheel_1();

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













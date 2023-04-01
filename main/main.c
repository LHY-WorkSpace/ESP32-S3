/* LVGL Example project
 *
 * Basic project to test LVGL on ESP32 based projects.
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_timer.h"

/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "lvgl_helpers.h"

#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    #if defined CONFIG_LV_USE_DEMO_WIDGETS
        #include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"
    #elif defined CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER
        #include "lv_examples/src/lv_demo_keypad_encoder/lv_demo_keypad_encoder.h"
    #elif defined CONFIG_LV_USE_DEMO_BENCHMARK
        #include "lv_examples/src/lv_demo_benchmark/lv_demo_benchmark.h"
    #elif defined CONFIG_LV_USE_DEMO_STRESS
        #include "lv_examples/src/lv_demo_stress/lv_demo_stress.h"
    #else
        #error "No demo application selected."
    #endif
#endif

/*********************
 *      DEFINES
 *********************/
#define TAG "demo"
#define LV_TICK_PERIOD_MS 1


#if 0



lv_obj_t *ALL_Bg;
lv_obj_t *Main;

// 逆时针（如下图）：
// x1=xcos(β)-ysin(β);
// y1=ycos(β)+xsin(β);

// 顺时针（图未给出）：
// x1=xcos(β)+ysin(β);
// y1=ycos(β)-xsin(β);


//旋转半径
#define R_LEN (45)
#define EYE_SIZE_W    (60)

#define ROTATEDIR_FORWARD     ( 1)//顺时针
#define ROTATEDIR_OPPOSITE    (-1)//逆时针

lv_obj_t *Eye_Group[4];
lv_anim_t Anima_Eye_Group[4];
lv_obj_t *Eye_base[4];//瞳孔
lv_anim_t Anima_Eye_Size[4];
uint8_t RotateDir = ROTATEDIR_FORWARD;
/*
	|
 2  |  3
---------
 1  |  0
	|
*/
 //口字形
int8_t Eye_Position[4][3]=
{
    { R_LEN,  R_LEN, 0},
    {-R_LEN,  R_LEN, 1},
    {-R_LEN, -R_LEN, 2},
    { R_LEN, -R_LEN, 3},
};

 ///// 十字型
// int8_t Eye_Position[4][3]=
// {
//     {0,      -R_LEN, 0},     //上
//     {R_LEN,  0,      1},     //右
//     {0,      R_LEN,  2},     //下
//     {-R_LEN, 0,      3},     //左
// };


static void Animation_CB(void *var, int32_t v)
{
    uint8_t i,j;
    int16_t x1,y1;

    lv_obj_t *Eye_tmp = (lv_obj_t *)var;

    for ( i = 0; i < 4; i++)
    {
        if(Eye_tmp == Eye_Group[i])
        {
			if( RotateDir == 1)
			{
				//顺时针
				x1=(Eye_Position[Eye_Position[i][2]][0])*FastCos(DEGTORAD(v))-(Eye_Position[Eye_Position[i][2]][1])*FastSin(DEGTORAD(v));
				y1=(Eye_Position[Eye_Position[i][2]][1])*FastCos(DEGTORAD(v))+(Eye_Position[Eye_Position[i][2]][0])*FastSin(DEGTORAD(v));  
			}
			else
			{
				//逆时针
				x1=(Eye_Position[Eye_Position[i][2]][0])*FastCos(DEGTORAD(v))+(Eye_Position[Eye_Position[i][2]][1])*FastSin(DEGTORAD(v));
				y1=(Eye_Position[Eye_Position[i][2]][1])*FastCos(DEGTORAD(v))-(Eye_Position[Eye_Position[i][2]][0])*FastSin(DEGTORAD(v));  
			}

			lv_obj_align_to(Eye_Group[i],Main,LV_ALIGN_CENTER,x1,y1);

			if( v == 90)
			{
				Eye_Position[i][2] = (Eye_Position[i][2] +RotateDir)%4;

            	lv_anim_set_repeat_count(&Anima_Eye_Size[i], 1);
				//旋转结束后调整眼部焦距
				lv_anim_start(&Anima_Eye_Size[i]);
			}
        }
    }
}

void Eye0_Create()
{
	Eye_Group[0] = lv_obj_create(Main);
	// Eye_Group[0] = lv_btn_create(Main);
	lv_obj_set_size(Eye_Group[0],EYE_SIZE_W,EYE_SIZE_W);
	lv_obj_set_style_bg_color(Eye_Group[0],lv_color_white(),LV_PART_MAIN);
	lv_obj_set_style_radius(Eye_Group[0],EYE_SIZE_W/2,LV_PART_MAIN);
	lv_obj_set_style_outline_width(Eye_Group[0],3,LV_PART_MAIN);
	lv_obj_set_style_outline_color(Eye_Group[0],lv_palette_main(LV_PALETTE_RED),LV_PART_MAIN);


	lv_obj_t * label = lv_label_create(Eye_Group[0]);

	lv_label_set_text_fmt(label, "%"LV_PRIu32, 0);
	lv_obj_align_to(label,Eye_Group[0],LV_ALIGN_CENTER,0,0);

}


void Eye1_Create()
{
	Eye_Group[1] = lv_obj_create(Main);
	lv_obj_set_size(Eye_Group[1],EYE_SIZE_W,EYE_SIZE_W);
	lv_obj_set_style_bg_color(Eye_Group[1],lv_color_white(),LV_PART_MAIN);
	lv_obj_set_style_radius(Eye_Group[1],EYE_SIZE_W/2,LV_PART_MAIN);
	lv_obj_set_style_outline_width(Eye_Group[1],3,LV_PART_MAIN);
	lv_obj_set_style_outline_color(Eye_Group[1],lv_palette_main(LV_PALETTE_BLUE),LV_PART_MAIN);

	lv_obj_t * label = lv_label_create(Eye_Group[1]);

	lv_label_set_text_fmt(label, "%"LV_PRIu32, 1);
	lv_obj_align_to(label,Eye_Group[1],LV_ALIGN_CENTER,0,0);

}


void Eye2_Create()
{
	Eye_Group[2] = lv_obj_create(Main);
	lv_obj_set_size(Eye_Group[2],EYE_SIZE_W,EYE_SIZE_W);
	lv_obj_set_style_bg_color(Eye_Group[2],lv_color_white(),LV_PART_MAIN);
	lv_obj_set_style_radius(Eye_Group[2],EYE_SIZE_W/2,LV_PART_MAIN);
	lv_obj_set_style_outline_width(Eye_Group[2],3,LV_PART_MAIN);
	lv_obj_set_style_outline_color(Eye_Group[2],lv_palette_main(LV_PALETTE_GREEN),LV_PART_MAIN);

	lv_obj_t * label = lv_label_create(Eye_Group[2]);

	lv_label_set_text_fmt(label, "%"LV_PRIu32, 2);
	lv_obj_align_to(label,Eye_Group[2],LV_ALIGN_CENTER,0,0);

}


void Eye3_Create()
{
	Eye_Group[3] = lv_obj_create(Main);
	lv_obj_set_size(Eye_Group[3],EYE_SIZE_W,EYE_SIZE_W);
	lv_obj_set_style_bg_color(Eye_Group[3],lv_color_white(),LV_PART_MAIN);
	lv_obj_set_style_radius(Eye_Group[3],EYE_SIZE_W/2,LV_PART_MAIN);
	lv_obj_set_style_outline_width(Eye_Group[3],3,LV_PART_MAIN);
	lv_obj_set_style_outline_color(Eye_Group[3],lv_palette_main(LV_PALETTE_DEEP_ORANGE),LV_PART_MAIN);

	lv_obj_t * label = lv_label_create(Eye_Group[3]);

	lv_label_set_text_fmt(label, "%"LV_PRIu32, 3);
	lv_obj_align_to(label,Eye_Group[3],LV_ALIGN_CENTER,0,0);

}


void ChangeSizeAnimaCB(void *var, int32_t v)
{
	uint8_t i,j;

    lv_obj_t *Eye_tmp = (lv_obj_t *)var;

    for ( i = 0; i < 4; i++)
    {
        if(Eye_tmp == Eye_base[i])
        {
			lv_obj_set_size(Eye_tmp,v,v);
			lv_obj_set_style_radius(Eye_tmp,v/2,LV_PART_MAIN);
			lv_obj_align_to(Eye_tmp,Eye_Group[i],LV_ALIGN_CENTER,0,0);
        }
    }
}

void ChangeSizeAnima()
{
	uint8_t i=0;

	for (i = 0; i < 4; i++)
	{
		
		lv_anim_init(&Anima_Eye_Size[i]);
        lv_anim_set_var(&Anima_Eye_Size[i],Eye_base[i]);
        lv_anim_set_values(&Anima_Eye_Size[i],40,30);
        lv_anim_set_time(&Anima_Eye_Size[i], 200);
		lv_anim_set_delay(&Anima_Eye_Size[i], 500);//1500
        lv_anim_set_exec_cb(&Anima_Eye_Size[i], ChangeSizeAnimaCB);

        lv_anim_set_path_cb(&Anima_Eye_Size[i],lv_anim_path_ease_in_out);
		// lv_anim_set_path_cb(&Anima_Eye1,lv_anim_path_ease_in_out);
		// lv_anim_set_path_cb(&Anima_Eye1,lv_anim_path_linear);
		// lv_anim_set_path_cb(&Anima,lv_anim_path_ease_in);
		// lv_anim_set_path_cb(&Anima,lv_anim_path_ease_out);//==


		lv_anim_set_playback_time(&Anima_Eye_Size[i],200);
		lv_anim_set_playback_delay(&Anima_Eye_Size[i],200);
        lv_anim_set_repeat_delay(&Anima_Eye_Size[i],500);
        lv_anim_set_repeat_count(&Anima_Eye_Size[i], 1);

	}

    for ( i = 0; i < 4; i++)
    {
        lv_anim_start(&Anima_Eye_Size[i]);
    }
}


void Eye_Create()
{
    uint8_t i=0;
	lv_obj_t *Eye_in;

    ALL_Bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(ALL_Bg,240,240);
    lv_obj_set_style_bg_color(ALL_Bg,lv_color_black(),LV_PART_MAIN);
    lv_obj_set_style_radius(ALL_Bg,0,LV_PART_MAIN);
    lv_obj_set_style_border_side(ALL_Bg,LV_BORDER_SIDE_FULL,LV_PART_MAIN);
    lv_obj_set_style_border_color(ALL_Bg,lv_color_white(),LV_PART_MAIN);
    lv_obj_set_style_border_width(ALL_Bg,0,LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(ALL_Bg,LV_SCROLLBAR_MODE_OFF);

    Main = lv_obj_create(ALL_Bg);
    lv_obj_set_size(Main,240,240);
    lv_obj_set_style_bg_color(Main,lv_color_black(),LV_PART_MAIN);
    lv_obj_set_style_radius(Main,120,LV_PART_MAIN);
    lv_obj_set_style_border_side(Main,LV_BORDER_SIDE_FULL,LV_PART_MAIN);
    lv_obj_set_style_border_color(Main,lv_palette_main(LV_PALETTE_GREY),LV_PART_MAIN);
    lv_obj_set_style_border_width(Main,5,LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(Main,LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(Main);

	// Eye0_Create();
	// Eye1_Create();
	// Eye2_Create();
	// Eye3_Create();

    for ( i = 0; i < 4; i++)
    {
		//眼球
		Eye_Group[i] = lv_obj_create(Main);
		lv_obj_set_size(Eye_Group[i],EYE_SIZE_W,EYE_SIZE_W);
		lv_obj_set_style_bg_color(Eye_Group[i],lv_color_make(56,110,23),LV_PART_MAIN);
		lv_obj_set_style_radius(Eye_Group[i],EYE_SIZE_W/2,LV_PART_MAIN);
		lv_obj_set_style_outline_width(Eye_Group[i],3,LV_PART_MAIN);
		lv_obj_set_style_outline_color(Eye_Group[i],lv_color_make(84,104,58),LV_PART_MAIN);

		//瞳孔
		Eye_base[i] = lv_obj_create(Eye_Group[i]);
		lv_obj_set_size(Eye_base[i],40,40);
		lv_obj_set_style_radius(Eye_base[i],20,LV_PART_MAIN);
		lv_obj_align_to(Eye_base[i],Eye_Group[i],LV_ALIGN_CENTER,0,0);
		lv_obj_set_style_bg_color(Eye_base[i],lv_color_black(),LV_PART_MAIN);
		lv_obj_set_scrollbar_mode(Eye_base[i],LV_SCROLLBAR_MODE_OFF);

		//眼部高光
		Eye_in = lv_obj_create(Eye_Group[i]);
		lv_obj_set_size(Eye_in,12,12);
		lv_obj_set_style_radius(Eye_in,6,LV_PART_MAIN);
		lv_obj_align_to(Eye_in,Eye_Group[i],LV_ALIGN_TOP_RIGHT,5,-7);
		lv_obj_set_scrollbar_mode(Eye_in,LV_SCROLLBAR_MODE_OFF);

		lv_obj_set_scrollbar_mode(Eye_Group[i],LV_SCROLLBAR_MODE_OFF);//关闭滚动条
        lv_obj_align_to(Eye_Group[i],Main,LV_ALIGN_CENTER,Eye_Position[i][0],Eye_Position[i][1]);

        lv_anim_init(&Anima_Eye_Group[i]);
        lv_anim_set_var(&Anima_Eye_Group[i],Eye_Group[i]);
        lv_anim_set_values(&Anima_Eye_Group[i],0,90);
        lv_anim_set_time(&Anima_Eye_Group[i], 300);
		// lv_anim_set_delay(&Anima_Eye_Group[i], 500);
        lv_anim_set_exec_cb(&Anima_Eye_Group[i], Animation_CB);
        lv_anim_set_path_cb(&Anima_Eye_Group[i],lv_anim_path_ease_in_out);

		// lv_anim_set_path_cb(&Anima_Eye1,lv_anim_path_ease_in_out);
		// lv_anim_set_path_cb(&Anima_Eye1,lv_anim_path_linear);
		// lv_anim_set_path_cb(&Anima,lv_anim_path_ease_in);
		// lv_anim_set_path_cb(&Anima,lv_anim_path_ease_out);//==

		// lv_anim_set_playback_time(&Anima_Eye_Size[i],200);
		// lv_anim_set_playback_delay(&Anima_Eye_Size[i],200);
        lv_anim_set_repeat_delay(&Anima_Eye_Group[i],2000);
        lv_anim_set_repeat_count(&Anima_Eye_Group[i], LV_ANIM_REPEAT_INFINITE);

    }

	ChangeSizeAnima();

    for ( i = 0; i < 4; i++)
    {
        lv_anim_start(&Anima_Eye_Group[i]);
    }
}



#endif



















/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void create_demo_application(void);

/**********************
 *   APPLICATION MAIN
 **********************/
void app_main() {

    /* If you want to use a task to create the graphic, you NEED to create a Pinned task
     * Otherwise there can be problem such as memory corruption and so on.
     * NOTE: When not using Wi-Fi nor Bluetooth you can pin the guiTask to core 0 */
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
}

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    // lv_color_t* test = heap_caps_malloc(1024*1024*2, MALLOC_CAP_SPIRAM);
    // assert(test != NULL);


    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1 != NULL);

    /* Use double buffered when not working with monochrome displays */
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf2 != NULL);
#else
    static lv_color_t *buf2 = NULL;
#endif

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

#if defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_IL3820         \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_JD79653A    \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_UC8151D     \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_SSD1306

    /* Actual size in pixels, not bytes. */
    size_in_px *= 8;
#endif

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    //free(test);
#if defined CONFIG_DISPLAY_ORIENTATION_PORTRAIT || defined CONFIG_DISPLAY_ORIENTATION_PORTRAIT_INVERTED
    disp_drv.rotated = 1;
#endif

    /* When using a monochrome display we need to register the callbacks:
     * - rounder_cb
     * - set_px_cb */
#ifdef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
#endif

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Register an input device when enabled on the menuconfig */
#if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */
     create_demo_application();
       // Eye_Create();
    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    free(buf1);
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    free(buf2);
#endif
    vTaskDelete(NULL);
}

static void create_demo_application(void)
{
    /* When using a monochrome display we only show "Hello World" centered on the
     * screen */
#if defined CONFIG_LV_TFT_DISPLAY_MONOCHROME || \
    defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_ST7735S

    /* use a pretty small demo for monochrome displays */
    /* Get the current screen  */
    lv_obj_t * scr = lv_disp_get_scr_act(NULL);

    /*Create a Label on the currently active screen*/
    lv_obj_t * label1 =  lv_label_create(scr, NULL);

    /*Modify the Label's text*/
    lv_label_set_text(label1, "Hello\nworld");

    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);
#else
    /* Otherwise we show the selected demo */

    #if defined CONFIG_LV_USE_DEMO_WIDGETS
        lv_demo_widgets();
    #elif defined CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER
        lv_demo_keypad_encoder();
    #elif defined CONFIG_LV_USE_DEMO_BENCHMARK
        lv_demo_benchmark();
    #elif defined CONFIG_LV_USE_DEMO_STRESS
        lv_demo_stress();
    #else
        #error "No demo application selected."
    #endif
#endif
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

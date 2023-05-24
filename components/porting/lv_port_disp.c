/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 0

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>
#include "lvgl_helpers.h"
#include "esp_heap_caps.h"
#include "esp_log.h"




/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

// static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */


    /* Example for 1) */
    // static lv_disp_draw_buf_t draw_buf_dsc_1;
    // // static lv_color_t buf_1[DISP_BUF_SIZE];                          /*A buffer for 10 rows*/

    // lv_color_t* buf_1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    // assert(buf_1 != NULL);

    // lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, DISP_BUF_SIZE);   /*Initialize the display buffer*/







    /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;

    // static lv_color_t buf_2_1[DISP_BUF_SIZE];                        /*A buffer for 10 rows*/
    // static lv_color_t buf_2_2[DISP_BUF_SIZE];                        /*An other buffer for 10 rows*/
    lv_color_t* buf_1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf_1 != NULL);
    lv_color_t* buf_2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf_2 != NULL);
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_1, buf_2,DISP_BUF_SIZE);   /*Initialize the display buffer*/


 

    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
    // static lv_disp_draw_buf_t draw_buf_dsc_3;
    // // static lv_color_t buf_3_1[DISP_BUF_SIZE];            /*A screen sized buffer*/
    // // static lv_color_t buf_3_2[DISP_BUF_SIZE];            /*Another screen sized buffer*/

    // lv_color_t* buf_1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    // assert(buf_1 != NULL);
    // lv_color_t* buf_2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    // assert(buf_2 != NULL);
    // lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_1, buf_2,DISP_BUF_SIZE);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_driver_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_2;

    /*Required for Example 3)*/
    // disp_drv.full_refresh = 1;

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
    lvgl_driver_init();
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
// static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
// {
//     if(disp_flush_enabled) {
//         /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

//         int32_t x;
//         int32_t y;
//         for(y = area->y1; y <= area->y2; y++) {
//             for(x = area->x1; x <= area->x2; x++) {
//                 /*Put a pixel to the display. For example:*/
//                 /*put_px(x, y, *color_p)*/
//                 color_p++;
//             }
//         }
//     }

//     /*IMPORTANT!!!
//      *Inform the graphics library that you are ready with the flushing*/
//     lv_disp_flush_ready(disp_drv);
// }

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

#include "lv_port_disp.h"
#include <stdbool.h>
#include "lvgl_helpers.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <stdio.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
// #include "esp_err.h"
// #include "esp_log.h"
// #include "lvgl.h"





#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (10 * 1000 * 1000)


#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_DATA0          6
#define EXAMPLE_PIN_NUM_DATA1          7
#define EXAMPLE_PIN_NUM_DATA2          8
#define EXAMPLE_PIN_NUM_DATA3          9
#define EXAMPLE_PIN_NUM_DATA4          10
#define EXAMPLE_PIN_NUM_DATA5          11
#define EXAMPLE_PIN_NUM_DATA6          12
#define EXAMPLE_PIN_NUM_DATA7          13
#define EXAMPLE_PIN_NUM_DATA8          14
#define EXAMPLE_PIN_NUM_DATA9          15
#define EXAMPLE_PIN_NUM_DATA10         16
#define EXAMPLE_PIN_NUM_DATA11         17
#define EXAMPLE_PIN_NUM_DATA12         18
#define EXAMPLE_PIN_NUM_DATA13         19
#define EXAMPLE_PIN_NUM_DATA14         20
#define EXAMPLE_PIN_NUM_DATA15         21
#define EXAMPLE_PIN_NUM_WR              5
#define EXAMPLE_PIN_NUM_CS             3
#define EXAMPLE_PIN_NUM_DC             4
#define EXAMPLE_PIN_NUM_RST            2
#define EXAMPLE_PIN_NUM_BK_LIGHT       1


#define EXAMPLE_LCD_H_RES              320
#define EXAMPLE_LCD_V_RES              240

#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8


#define EXAMPLE_PSRAM_DATA_ALIGNMENT   64
static const char *TAG = "lv_port_disp";

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}


void lv_port_disp_init(void)
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions

    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);

    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = EXAMPLE_PIN_NUM_DC,
        .wr_gpio_num = EXAMPLE_PIN_NUM_WR,
        .data_gpio_nums = {
            EXAMPLE_PIN_NUM_DATA0,
            EXAMPLE_PIN_NUM_DATA1,
            EXAMPLE_PIN_NUM_DATA2,
            EXAMPLE_PIN_NUM_DATA3,
            EXAMPLE_PIN_NUM_DATA4,
            EXAMPLE_PIN_NUM_DATA5,
            EXAMPLE_PIN_NUM_DATA6,
            EXAMPLE_PIN_NUM_DATA7,
            EXAMPLE_PIN_NUM_DATA8,
            EXAMPLE_PIN_NUM_DATA9,
            EXAMPLE_PIN_NUM_DATA10,
            EXAMPLE_PIN_NUM_DATA11,
            EXAMPLE_PIN_NUM_DATA12,
            EXAMPLE_PIN_NUM_DATA13,
            EXAMPLE_PIN_NUM_DATA14,
            EXAMPLE_PIN_NUM_DATA15,

        },
        .bus_width = 16,
        .max_transfer_bytes = EXAMPLE_LCD_H_RES * 100 * sizeof(uint16_t),
        .psram_trans_align = EXAMPLE_PSRAM_DATA_ALIGNMENT,
        .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = EXAMPLE_PIN_NUM_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .swap_color_bytes = 1, // Swap can be done in LvGL (default) or DMA
        },
        .on_color_trans_done = example_notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    ESP_LOGI(TAG, "Install LCD driver of ili9341 (st7789 compatible)");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_invert_color(panel_handle, false);

    esp_lcd_panel_io_tx_param(io_handle, 0xF2, (uint8_t[]) { 0 }, 1); // 3Gamma function disable
    esp_lcd_panel_io_tx_param(io_handle, 0x26, (uint8_t[]) { 1 }, 1); // Gamma curve 1 selected
    esp_lcd_panel_io_tx_param(io_handle, 0xE0, (uint8_t[]) {          // Set positive gamma
        0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 }, 15);
    esp_lcd_panel_io_tx_param(io_handle, 0xE1, (uint8_t[]) {          // Set negative gamma
        0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F }, 15);

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    // lv_init();

    lv_color_t *buf1 = NULL;
    lv_color_t *buf2 = NULL;

    buf1 = heap_caps_aligned_alloc(EXAMPLE_PSRAM_DATA_ALIGNMENT, EXAMPLE_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
//     buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf1);

    buf2 = heap_caps_aligned_alloc(EXAMPLE_PSRAM_DATA_ALIGNMENT, EXAMPLE_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
//     buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf2);

    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 100);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
    lvgl_driver_init();
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}















































#endif

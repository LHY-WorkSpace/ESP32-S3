
/**
 * @file lv_port_indev_templ.h
 *
 */

/*Copy this file as "lv_port_indev.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_INDEV_TEMPL_H
#define LV_PORT_INDEV_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define  TOUCH_PAD  1 
#define  MOUSE      2 
#define  KEY_PAD    3
#define  ENCODER    4
#define  BUTTON     5

#define  DEV_TYPE  ENCODER


#if( DEV_TYPE == TOUCH_PAD) 
extern lv_indev_t * indev_touchpad;

#elif ( DEV_TYPE == MOUSE )
extern lv_indev_t * indev_mouse;

#elif ( DEV_TYPE == KEY_PAD )
extern lv_indev_t * indev_keypad;

#elif( DEV_TYPE == ENCODER )
extern lv_indev_t * indev_encoder;

#elif( DEV_TYPE == BUTTON )
#define BUTTON_NUM  (4)
extern lv_indev_t * indev_button;
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_indev_init(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_INDEV_TEMPL_H*/

#endif /*Disable/Enable content*/

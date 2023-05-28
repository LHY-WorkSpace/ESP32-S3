#ifndef LVGL_UI_H
#define LVGL_UI_H


void MainUICreate(void);
void LVGL_Task(void);
void LVGL_Init(void);
void set_MeterValue(int32_t v);
void Wave_Task(void);
void SetEyeBgColorRGB(uint8_t r, uint8_t g, uint8_t b);
void RotateEye(uint8_t Dir ,uint8_t times);
void ChangeEyeFocalize(uint8_t times);
#endif


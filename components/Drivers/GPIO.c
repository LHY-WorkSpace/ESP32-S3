#include "GPIO.h"
#include "driver/gpio.h"


const   gpio_config_t GPIO = 
{
    .pin_bit_mask = ((1ULL << (uint8_t)(1))),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = 1,
    .pull_down_en = 0,
    .intr_type = GPIO_INTR_DISABLE,
};
    


void GPIO_Init(void)
{
   gpio_config(&GPIO);
}





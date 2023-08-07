#include "GPIO.h"
#include "driver/gpio.h"


#if 0

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

#else

void GPIO_Init(void)
{
    // OutPut Mode
    esp_rom_gpio_pad_select_gpio(12);
    gpio_set_direction(12, GPIO_MODE_OUTPUT);
    gpio_set_level(12, 0);// OutPut Low Level

    // InPut Mode
    esp_rom_gpio_pad_select_gpio(11);
    gpio_set_direction(11, GPIO_MODE_INPUT);
    gpio_get_level(11);

}

#endif





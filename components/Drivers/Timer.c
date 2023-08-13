#include "Timer.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "Key.h"
#include "AS5600.h"
#include "FOC.h"
// "driver/gptimer.h"是更新版的"driver/tmer.h"，都能使用，语法不同

//0.1 ms
#define LVGL_TICK_PERIOD_MS	(1)

static bool Timer_CB(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);

//无需考虑定时器分配 gptimer_handle_t 会自动分配
void Timer_Init()
{
	gptimer_handle_t gptimer = NULL;
	gptimer_config_t timer_config = 
	{
		.clk_src = GPTIMER_CLK_SRC_DEFAULT,
		.direction = GPTIMER_COUNT_UP,
		.resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
	};

	gptimer_alarm_config_t alarm_config =
	{
		.reload_count = 0, // counter will reload with 0 on alarm event
		.alarm_count = LVGL_TICK_PERIOD_MS*1000, // period = 1s @resolution 1MHz
		.flags.auto_reload_on_alarm = true, // enable auto-reload
	};

	gptimer_event_callbacks_t cbs = 
	{
		.on_alarm = Timer_CB, // register user callback
	};

	ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));//分配定时器
	ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
	ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
	ESP_ERROR_CHECK(gptimer_enable(gptimer));
	ESP_ERROR_CHECK(gptimer_start(gptimer));

}

static bool IRAM_ATTR Timer_CB(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
	static uint16_t cnt;

	lv_tick_inc(LVGL_TICK_PERIOD_MS);

	if( ( cnt % 20 ) == 0 )
	{
		button_ticks(); //20 ms
	}

	cnt++;
	if(cnt >= 10000)
	{
		cnt = 0;
	}

	return 0;
}













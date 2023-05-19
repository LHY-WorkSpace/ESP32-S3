#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "MathFun.h"


//     MCPWM0A = 0,        /*!<PWM0A output pin*/
//     MCPWM0B,            /*!<PWM0B output pin*/
//     MCPWM1A,            /*!<PWM1A output pin*/
//     MCPWM1B,            /*!<PWM1B output pin*/
//     MCPWM2A,            /*!<PWM2A output pin*/
//     MCPWM2B,            /*!<PWM2B output pin*/

#define UA_A_PIN    (35)
#define UA_B_PIN    (36)

// #define UB_A_PIN    (35)
// #define UB_B_PIN    (35)

// #define UC_A_PIN    (35)
// #define UC_B_PIN    (35)

static void gen_action_config(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb, mcpwm_cmpr_handle_t cmpa, mcpwm_cmpr_handle_t cmpb)
{
      ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gena,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmpa, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(genb,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpb, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmpb, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
}
void FOC_GPIO_Init(void)
{

    mcpwm_timer_handle_t mcpwm_timer_handle  = NULL ;
    mcpwm_timer_config_t timer_config = 
    {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1 * 1000 * 1000,
        .period_ticks = 500, // 50us <-> 20KHz
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    mcpwm_timer_enable(mcpwm_timer_handle);
    mcpwm_new_timer(&timer_config,&mcpwm_timer_handle);



    mcpwm_oper_handle_t mcpwm_oper_handle  = NULL;
    mcpwm_operator_config_t mcpwm_operator_config = 
    {
        .group_id = 0,
    };
    mcpwm_new_operator(&mcpwm_operator_config,&mcpwm_oper_handle);


    mcpwm_cmpr_handle_t UA_A_comparator_handle;
    mcpwm_cmpr_handle_t UA_B_comparator_handle;
    mcpwm_comparator_config_t UA_A_comparator_config =
    {
        .flags.update_cmp_on_tep = true,
        .flags.update_cmp_on_tez = true,
    };
    mcpwm_comparator_config_t UA_B_comparator_config =
    {
        .flags.update_cmp_on_tep = true,
        .flags.update_cmp_on_tez = true,
    };
    mcpwm_new_comparator(mcpwm_oper_handle,&UA_A_comparator_config,&UA_A_comparator_handle);
    mcpwm_new_comparator(mcpwm_oper_handle,&UA_B_comparator_config,&UA_B_comparator_handle);


    mcpwm_gen_handle_t  mcpwm_UA_A_gen_handle;
    mcpwm_gen_handle_t  mcpwm_UA_B_gen_handle;
    mcpwm_generator_config_t  mcpwm_UA_A_generator_config=
    {
        .gen_gpio_num = UA_A_PIN,
        .flags.io_loop_back = true,
    };
    mcpwm_generator_config_t  mcpwm_UA_B_generator_config=
    {
        .gen_gpio_num = UA_B_PIN,
        .flags.io_loop_back = true,
    };
    mcpwm_new_generator(mcpwm_oper_handle,&mcpwm_UA_A_generator_config,&mcpwm_UA_A_gen_handle);
    mcpwm_new_generator(mcpwm_oper_handle,&mcpwm_UA_B_generator_config,&mcpwm_UA_B_gen_handle);


    mcpwm_dead_time_config_t UA_A_dead_time_config = 
    {
        .posedge_delay_ticks = 100,
        .negedge_delay_ticks = 0,
        .flags.invert_output = true
    };
    mcpwm_dead_time_config_t UA_B_dead_time_config = 
    {
        .posedge_delay_ticks = 0,
        .negedge_delay_ticks = 100,
        .flags.invert_output = false,
    };
    mcpwm_generator_set_dead_time(mcpwm_UA_A_gen_handle,mcpwm_UA_A_gen_handle,&UA_A_dead_time_config);
    mcpwm_generator_set_dead_time(mcpwm_UA_A_gen_handle,mcpwm_UA_B_gen_handle,&UA_B_dead_time_config);


    gen_action_config(mcpwm_UA_A_gen_handle,mcpwm_UA_B_gen_handle,UA_A_comparator_handle,UA_B_comparator_handle);
    mcpwm_timer_start_stop(mcpwm_timer_handle,MCPWM_TIMER_START_NO_STOP);

}







typedef void (*set_dead_time_cb_t)(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb);
typedef void (*set_gen_actions_cb_t)(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb, mcpwm_cmpr_handle_t cmpa, mcpwm_cmpr_handle_t cmpb);

static void mcpwm_deadtime_test_template(uint32_t timer_resolution, uint32_t period, uint32_t cmpa, uint32_t cmpb, int gpioa, int gpiob,
        set_gen_actions_cb_t set_generator_actions, set_dead_time_cb_t set_dead_time)
{
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000,
        .period_ticks = 50,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    mcpwm_timer_handle_t timer = NULL;
    (mcpwm_new_timer(&timer_config, &timer));
    (mcpwm_timer_enable(timer));
    
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };
    mcpwm_oper_handle_t oper = NULL;
    (mcpwm_new_operator(&operator_config, &oper));
    (mcpwm_operator_connect_timer(oper, timer));



    mcpwm_cmpr_handle_t comparator_a = NULL;
    mcpwm_cmpr_handle_t comparator_b = NULL;
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    (mcpwm_new_comparator(oper, &comparator_config, &comparator_a));
    (mcpwm_new_comparator(oper, &comparator_config, &comparator_b));
    (mcpwm_comparator_set_compare_value(comparator_a, cmpa));
    (mcpwm_comparator_set_compare_value(comparator_b, cmpb));

    mcpwm_gen_handle_t generator_a = NULL;
    mcpwm_gen_handle_t generator_b = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = UA_A_PIN,
    };
    (mcpwm_new_generator(oper, &generator_config, &generator_a));
    generator_config.gen_gpio_num = UA_B_PIN;
    (mcpwm_new_generator(oper, &generator_config, &generator_b));

    set_generator_actions(generator_a, generator_b, comparator_a, comparator_b);
    set_dead_time(generator_a, generator_b);

    (mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
    vTaskDelay(pdMS_TO_TICKS(100));
    (mcpwm_timer_start_stop(timer, MCPWM_TIMER_STOP_EMPTY));
    vTaskDelay(pdMS_TO_TICKS(10));

    (mcpwm_timer_disable(timer));
    (mcpwm_del_generator(generator_a));
    (mcpwm_del_generator(generator_b));
    (mcpwm_del_comparator(comparator_a));
    (mcpwm_del_comparator(comparator_b));
    (mcpwm_del_operator(oper));
    (mcpwm_del_timer(timer));
}



static void ahc_set_generator_actions(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb, mcpwm_cmpr_handle_t cmpa, mcpwm_cmpr_handle_t cmpb)
{
    (mcpwm_generator_set_actions_on_timer_event(gena,
                MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    (mcpwm_generator_set_actions_on_compare_event(gena,
                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_LOW),
                MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
}
static void ahc_set_dead_time(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb)
{
    mcpwm_dead_time_config_t dead_time_config = {
        .posedge_delay_ticks = 50,
        .negedge_delay_ticks = 0
    };
    (mcpwm_generator_set_dead_time(gena, gena, &dead_time_config));
    dead_time_config.posedge_delay_ticks = 0;
    dead_time_config.negedge_delay_ticks = 100;
    dead_time_config.flags.invert_output = true;
    (mcpwm_generator_set_dead_time(gena, genb, &dead_time_config));
}

static void alc_set_generator_actions(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb, mcpwm_cmpr_handle_t cmpa, mcpwm_cmpr_handle_t cmpb)
{
    (mcpwm_generator_set_actions_on_timer_event(gena,
                MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    (mcpwm_generator_set_actions_on_compare_event(gena,
                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_LOW),
                MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
}

static void alc_set_dead_time(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb)
{
    mcpwm_dead_time_config_t dead_time_config = {
        .posedge_delay_ticks = 50,
        .negedge_delay_ticks = 0,
        .flags.invert_output = true
    };
    (mcpwm_generator_set_dead_time(gena, gena, &dead_time_config));
    dead_time_config.posedge_delay_ticks = 0;
    dead_time_config.negedge_delay_ticks = 100;
    dead_time_config.flags.invert_output = false;
    (mcpwm_generator_set_dead_time(gena, genb, &dead_time_config));
}
void ERRER()
{
    printf("Active High Complementary\r\n");
    mcpwm_deadtime_test_template(10000, 600, 200, 400, 0, 2, ahc_set_generator_actions, ahc_set_dead_time);

    // printf("Active Low Complementary\r\n");
    // mcpwm_deadtime_test_template(10000, 600, 200, 400, 0, 2, alc_set_generator_actions, alc_set_dead_time);
}

#if 0

/* brushed dc motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * This example will show you how to use MCPWM module to control brushed dc motor.
 * This code is tested with L298 motor driver.
 * User may need to make changes according to the motor driver they use.
*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#define GPIO_PWM0A_OUT 15   //Set GPIO 15 as PWM0A
#define GPIO_PWM0B_OUT 16   //Set GPIO 16 as PWM0B

static void mcpwm_example_gpio_initialize()
{
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}

/**
 * @brief Configure MCPWM module for brushed dc motor
 */
static void mcpwm_example_brushed_motor_control(void *arg)
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    while (1) 
    {
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 50.0);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 30.0);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void FOC_main()
{
    printf("Testing brushed motor...\n");
    xTaskCreate(mcpwm_example_brushed_motor_control, "mcpwm_examlpe_brushed_motor_control", 4096, NULL, 5, NULL);
}

#endif








#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/Timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_timer.h"
#include "MathFun.h"
#include "FOC.h"


#define iq	(3.0f)
#define id	(0.0f)
#define VCC_MOTOR	(12.0f)

// 7 pair(*2) magnet
// 6 pair(*2) coil
#define POLE_PAIR	(6)


// 33 34 35 36 37 在八线SPI时被SRAM和flash占用
#define UA_A_PIN    (4)
#define UA_B_PIN    (5)
#define UB_A_PIN    (6)
#define UB_B_PIN    (7)
#define UC_A_PIN    (15)
#define UC_B_PIN    (16)
#define TICK_HZ     (1*1000*1000)//1MHz
#define PWM_FREQ    (20000)//HZ


uint8_t PinGroup[U_PhaseMax][PWM_Max]=
{
    {UA_A_PIN,UA_B_PIN},
    {UB_A_PIN,UB_B_PIN},
    {UC_A_PIN,UC_B_PIN},
};

mcpwm_cmpr_handle_t Comparator[U_PhaseMax];

// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/mcpwm/mcpwm_bldc_hall_control/main/mcpwm_bldc_hall_control_example_main.c

void FOC_GPIO_Init(void)
{
    uint8_t i;

    mcpwm_timer_handle_t Timer;
    mcpwm_timer_config_t Timer_Config = 
    {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = TICK_HZ,
        .period_ticks =  TICK_HZ/PWM_FREQ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    //分配一个定时器;
    mcpwm_new_timer(&Timer_Config,&Timer);
    mcpwm_timer_enable(Timer);

    mcpwm_oper_handle_t Operator[U_PhaseMax];
    mcpwm_operator_config_t Operator_Config = 
    {
        .group_id = 0,
        // .flags.update_gen_action_on_tez = true,
        // .flags.update_gen_action_on_tep = true,
        // .flags.update_gen_action_on_sync = true,
        // .flags.update_dead_time_on_tez = true,
        // .flags.update_dead_time_on_tep = true,
        // .flags.update_dead_time_on_sync = true,
    };

    //多个操作器可以连接到同一个定时器
    ///分配3个操作器,连接到同一个定时器
    for ( i = 0; i < U_PhaseMax; i++)
    {
        mcpwm_new_operator(&Operator_Config,&Operator[i]);
        mcpwm_operator_connect_timer(Operator[i],Timer);
    }
    

    mcpwm_comparator_config_t Comparator_Config =
    {
        .flags.update_cmp_on_tep = true,
        // .flags.update_cmp_on_tez = true,
        // .flags.update_cmp_on_sync = true,
    };
    //每个操作器分配一个比较器
    for ( i = 0; i < U_PhaseMax; i++)
    {
        mcpwm_new_comparator(Operator[i],&Comparator_Config,&Comparator[i]);
        mcpwm_comparator_set_compare_value(Comparator[i], 30);
    }


    mcpwm_gen_handle_t  PWM_Out_Channel[U_PhaseMax][PWM_Max];
    mcpwm_generator_config_t  PWM_A_Pin_Config=
    {
        .gen_gpio_num = 0,
        .flags.io_loop_back = false,
        // .flags.invert_pwm = false,
    };
    mcpwm_generator_config_t  PWM_B_Pin_Config=
    {
        .gen_gpio_num = 0,
        .flags.io_loop_back = false,
        // .flags.invert_pwm = false,
    };


    for ( i = 0; i < U_PhaseMax; i++)
    {
        PWM_A_Pin_Config.gen_gpio_num = PinGroup[i][PWM_A];
        PWM_B_Pin_Config.gen_gpio_num = PinGroup[i][PWM_B];
        mcpwm_new_generator(Operator[i],&PWM_A_Pin_Config,&PWM_Out_Channel[i][PWM_A]);
        mcpwm_new_generator(Operator[i],&PWM_B_Pin_Config,&PWM_Out_Channel[i][PWM_B]);
    }


    for (i = 0; i < U_PhaseMax; i++)
    {
        mcpwm_generator_set_actions_on_timer_event( PWM_Out_Channel[i][PWM_A], 
                                                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                                                    MCPWM_GEN_TIMER_EVENT_ACTION_END());
        mcpwm_generator_set_actions_on_compare_event(PWM_Out_Channel[i][PWM_A],
                                                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, Comparator[i], MCPWM_GEN_ACTION_LOW),
                                                    MCPWM_GEN_COMPARE_EVENT_ACTION_END());

        mcpwm_generator_set_actions_on_timer_event( PWM_Out_Channel[i][PWM_B],
                                                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                                                    MCPWM_GEN_TIMER_EVENT_ACTION_END());
        mcpwm_generator_set_actions_on_compare_event(PWM_Out_Channel[i][PWM_B],
                                                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, Comparator[i], MCPWM_GEN_ACTION_LOW),
                                                    MCPWM_GEN_COMPARE_EVENT_ACTION_END());
    }

    mcpwm_dead_time_config_t PWM_A_dead_time_config = 
    {
        .posedge_delay_ticks = 0,//上升沿延迟时间
        .negedge_delay_ticks = 0,//下降沿延迟时间
        .flags.invert_output = false,
    };
    mcpwm_dead_time_config_t PWM_B_dead_time_config = 
    {
        .posedge_delay_ticks = 10,
        .negedge_delay_ticks = 10,
        .flags.invert_output = true,
    };

    for (i = 0; i < U_PhaseMax; i++)
    {
        mcpwm_generator_set_dead_time(PWM_Out_Channel[i][PWM_A],PWM_Out_Channel[i][PWM_A],&PWM_A_dead_time_config);
        mcpwm_generator_set_dead_time(PWM_Out_Channel[i][PWM_B],PWM_Out_Channel[i][PWM_B],&PWM_B_dead_time_config);
    }
    
    mcpwm_timer_start_stop(Timer,MCPWM_TIMER_START_NO_STOP);

}
//求电角度
float ElectricalAngle(float shaft_angle, int pole_pairs) 
{
  return (shaft_angle * pole_pairs);
}

float LimitAngle(float shaft_angle) 
{
  return (float)((int)shaft_angle % 360);
}


// 帕克逆变换+克拉克逆变换
float IA(float Angle)
{
	float Temp;
	Temp = id * FastCos(DEGTORAD(Angle)) + iq * FastSin(DEGTORAD(Angle));
	return Temp;
}

float IB(float Angle)
{
	float Temp;
	Temp = iq *( 0.866f * FastCos(DEGTORAD(Angle)) + 0.5f * FastSin(DEGTORAD(Angle)) ) + id * (0.866f * FastSin(DEGTORAD(Angle)) - 0.5f * FastCos(DEGTORAD(Angle)));
	return Temp;
}

float IC(float Angle)
{
	float Temp;
	Temp = iq *( 0.5f * FastSin(DEGTORAD(Angle)) - 0.866f * FastCos(DEGTORAD(Angle))) + id * ( 0.5f * FastCos(DEGTORAD(Angle)) + 0.866f * FastSin(DEGTORAD(Angle)) );
	return Temp;
}



void SetPWMDuty(uint8_t Phase,uint8_t Value)
{
    if(Phase >= U_PhaseMax)
    {
        return;
    }
    mcpwm_comparator_set_compare_value(Comparator[Phase], TICK_HZ/PWM_FREQ*Value/100);
}


void PWM_Task()
{
	uint8_t i;
    static uint16_t duty; 
    TickType_t Time;	

    Time = xTaskGetTickCount();
    while (1)
    {

        for ( i = 0; i < U_PhaseMax; i++)
        {
            mcpwm_comparator_set_compare_value(Comparator[i], duty);
        }
        if(duty>=1000)
        {
            duty=0;
        }
        duty+=100;
		vTaskDelayUntil(&Time,10/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);
}


void Foc_CTL()
{
	static float Angle  = 1.0f;
    TickType_t Time;	
	float I[3];
    float angtmp;

    Time = xTaskGetTickCount();
    while (1)
    {
		Angle += 10.0;

        angtmp = ElectricalAngle(Angle,POLE_PAIR);
        angtmp = LimitAngle(angtmp);
		I[0] = IA(angtmp)+VCC_MOTOR/2;
		I[1] = IB(angtmp)+VCC_MOTOR/2;
		I[2] = IC(angtmp)+VCC_MOTOR/2;
        
		SetPWMDuty(UA_Phase,(uint8_t)(I[0]*100/VCC_MOTOR));
		SetPWMDuty(UB_Phase,(uint8_t)(I[1]*100/VCC_MOTOR));
		SetPWMDuty(UC_Phase,(uint8_t)(I[2]*100/VCC_MOTOR));
		printf("Angle:%.2f Ia:%.2f Ib:%.2f Ic:%.2f\r\n",Angle,I[0]/VCC_MOTOR,I[1]/VCC_MOTOR,I[2]/VCC_MOTOR);

		if(Angle >= 360.0f)
		{
			Angle = 0.0f;
		}
		vTaskDelayUntil(&Time,1/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);
}




void FOC_Task()
{
	static float Angle  = 1.0f;	
	float I[3];
    float angtmp;

		Angle += 10.0;

        angtmp = ElectricalAngle(Angle,POLE_PAIR);
        angtmp = LimitAngle(angtmp);
		I[0] = IA(angtmp)+VCC_MOTOR/2;
		I[1] = IB(angtmp)+VCC_MOTOR/2;
		I[2] = IC(angtmp)+VCC_MOTOR/2;
        
		SetPWMDuty(UA_Phase,(uint8_t)(I[0]*100/VCC_MOTOR));
		SetPWMDuty(UB_Phase,(uint8_t)(I[1]*100/VCC_MOTOR));
		SetPWMDuty(UC_Phase,(uint8_t)(I[2]*100/VCC_MOTOR));
		//printf("Angle:%.2f Ia:%.2f Ib:%.2f Ic:%.2f\r\n",Angle,I[0]/VCC_MOTOR,I[1]/VCC_MOTOR,I[2]/VCC_MOTOR);

		if(Angle >= 360.0f)
		{
			Angle = 0.0f;
		}
}
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
#include "AS5600.h"
#include "PID.h"
// 7 pair(*2) magnet
// 6 pair(*2) coil

#define POLE_PAIR	(7)
#define Uq          (6.0f)
#define Ud          (0.0f)
#define VCC_MOTOR	(12.0f)


// 33 34 35 36 37 在八线SPI时被SRAM和flash占用
#define UA_A_PIN    (4)
#define UA_B_PIN    (PWM_PIN_NULL)
#define UB_A_PIN    (6)
#define UB_B_PIN    (PWM_PIN_NULL)
#define UC_A_PIN    (15)
#define UC_B_PIN    (PWM_PIN_NULL)
#define TICK_HZ     (1*1000*1000)//1MHz
#define PWM_FREQ    (20*1000)//HZ

float Ua,Ub,Uc;
uint8_t PinGroup[U_PhaseMax][PWM_Max]=
{
    {UA_A_PIN,UA_B_PIN},
    {UB_A_PIN,UB_B_PIN},
    {UC_A_PIN,UC_B_PIN},
};

mcpwm_cmpr_handle_t Comparator[U_PhaseMax];
static PID_t PositionPID;
static PID_t SpeedPID;
static PID_t ForcePID;
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
        mcpwm_new_generator(Operator[i],&PWM_A_Pin_Config,&PWM_Out_Channel[i][PWM_A]);

        if(PinGroup[i][PWM_B] != PWM_PIN_NULL)
        {
            PWM_B_Pin_Config.gen_gpio_num = PinGroup[i][PWM_B];
            mcpwm_new_generator(Operator[i],&PWM_B_Pin_Config,&PWM_Out_Channel[i][PWM_B]);
        }

    }


    for (i = 0; i < U_PhaseMax; i++)
    {
            mcpwm_generator_set_actions_on_timer_event( PWM_Out_Channel[i][PWM_A], 
                                                        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                                                        MCPWM_GEN_TIMER_EVENT_ACTION_END());
            mcpwm_generator_set_actions_on_compare_event(PWM_Out_Channel[i][PWM_A],
                                                        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, Comparator[i], MCPWM_GEN_ACTION_LOW),
                                                        MCPWM_GEN_COMPARE_EVENT_ACTION_END());
        if(PinGroup[i][PWM_B] != PWM_PIN_NULL)
        {        
            mcpwm_generator_set_actions_on_timer_event( PWM_Out_Channel[i][PWM_B],
                                                        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                                                        MCPWM_GEN_TIMER_EVENT_ACTION_END());
            mcpwm_generator_set_actions_on_compare_event(PWM_Out_Channel[i][PWM_B],
                                                        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, Comparator[i], MCPWM_GEN_ACTION_LOW),
                                                        MCPWM_GEN_COMPARE_EVENT_ACTION_END());
        }
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

        if(PinGroup[i][PWM_B] != PWM_PIN_NULL)
        {  
        mcpwm_generator_set_dead_time(PWM_Out_Channel[i][PWM_B],PWM_Out_Channel[i][PWM_B],&PWM_B_dead_time_config);
        }
    }
    
    mcpwm_timer_start_stop(Timer,MCPWM_TIMER_START_NO_STOP);

}

//求电角度 = 物理角度*极对数
float ElectricalAngle(float physics_angle, int pole_pairs) 
{
  return (physics_angle * (float)pole_pairs);
}

float LimitAngle(float Input) 
{
    float Tmp;
    Tmp = fmod(Input,360.0);
    if( Tmp < 0.0)
    {
       Tmp += 360.0;
    }
    return Tmp;
}
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

//逆变换
void N_Transform(float uq, float ud, float Angle)
{
    float Ualpha,Ubeta; 

    Angle = LimitAngle(Angle);
    //帕克逆变换
    Ualpha = ud * FastCos(DEGTORAD(Angle)) - uq * FastSin(DEGTORAD(Angle)); 
    Ubeta =  ud * FastSin(DEGTORAD(Angle)) + uq * FastCos(DEGTORAD(Angle)); 

    // 克拉克逆变换
    Ua = Ualpha + VCC_MOTOR/2;
    Ub = (sqrt(3)*Ubeta-Ualpha)/2 + VCC_MOTOR/2;
    Uc = (-Ualpha-sqrt(3)*Ubeta)/2 + VCC_MOTOR/2;

    // SetPWMDuty(UA_Phase,(uint8_t)(Ua*100/VCC_MOTOR));
    // SetPWMDuty(UB_Phase,(uint8_t)(Ub*100/VCC_MOTOR));
    // SetPWMDuty(UC_Phase,(uint8_t)(Uc*100/VCC_MOTOR));
    //printf("Angle:%.2f Ia:%d Ib:%d Ic:%d\r\n",Angle,(uint8_t)(Ua*100/VCC_MOTOR),(uint8_t)(Ub*100/VCC_MOTOR),(uint8_t)(Uc*100/VCC_MOTOR));

}

void P_Transform(float Ia, float Ib, float Ic)
{
    // float iq,id; 
    // float ialpha,ibeta;
    // float Angle = 0.0;

    // ialpha = Ia;
    // ibeta = (1/sqrt(3))*Ia + (2/sqrt(3))*Ib;

    // //// 帕克变换
    // iq = ibeta  * FastCos(DEGTORAD(Angle)) - ialpha * FastSin(DEGTORAD(Angle));
    // id = ialpha * FastCos(DEGTORAD(Angle)) + ibeta  * FastSin(DEGTORAD(Angle));

}

void SVPWM_CTL(float uq, float ud, float Angle)
{

    int sector = floor(Angle / 60.0) + 1;
    // 计算占空比
    float T1 = sqrt(3)*FastSin(DEGTORAD(sector*60.0 - Angle)) * uq/VCC_MOTOR;
    float T2 = sqrt(3)*FastSin(DEGTORAD(Angle - (sector-1.0)*60.0)) * uq/VCC_MOTOR;
    // 两个版本 
    // 以电压电源为中心/2
      float T0 = 1 - T1 - T2;
    // 低电源电压，拉到0
    //float T0 = 0;

    // 计算占空比（时间）
    float Ta,Tb,Tc; 
    switch(sector)
    {
        case 1:
            Ta = T1 + T2 + T0/2;
            Tb = T2 + T0/2;
            Tc = T0/2;
            break;
        case 2:
            Ta = T1 +  T0/2;
            Tb = T1 + T2 + T0/2;
            Tc = T0/2;
            break;
        case 3:
            Ta = T0/2;
            Tb = T1 + T2 + T0/2;
            Tc = T2 + T0/2;
            break;
        case 4:
            Ta = T0/2;
            Tb = T1+ T0/2;
            Tc = T1 + T2 + T0/2;
            break;
        case 5:
            Ta = T2 + T0/2;
            Tb = T0/2;
            Tc = T1 + T2 + T0/2;
            break;
        case 6:
            Ta = T1 + T2 + T0/2;
            Tb = T0/2;
            Tc = T1 + T0/2;
            break;
        default:
            // 可能的错误状态
            Ta = 0;
            Tb = 0;
            Tc = 0;
      }

      // 计算相电压和中心
      Ua = Ta*VCC_MOTOR;
      Ub = Tb*VCC_MOTOR;
      Uc = Tc*VCC_MOTOR;
}









void SetPWMDuty(uint8_t Phase,uint8_t Value)
{
    if(Phase >= U_PhaseMax)
    {
        return;
    }

    if( Value > 100)
    {
        return;
    }

    mcpwm_comparator_set_compare_value(Comparator[Phase], TICK_HZ*Value/PWM_FREQ/100);
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

float Addval = 0.0;
void Foc_CTL()
{
	float Angle  = 1.0f;
    TickType_t Time;	
    float angtmp;
    float UqTmp;   

    PID_Init(&PositionPID);
    PID_Change_Kp(&PositionPID,0.05);
    // PID_Change_Ki(&PositionPID,0.01);
    PID_Change_Kd(&PositionPID,0.01);


    Time = xTaskGetTickCount();
    while (1)
    {
        PID_SetTarget(&PositionPID,Addval);
        Angle = AS5600_Angle(ANGLE_TURN_MODE);
        // printf("FOC %.2f\n",Angle);
        angtmp = ElectricalAngle(Angle,POLE_PAIR);
        angtmp = LimitAngle(angtmp);

        UqTmp = PID_Process(&PositionPID,Angle);

        printf("FOC:%.2f,%.2f,%.2f\n",UqTmp,Angle,Addval);
        N_Transform(_constrain(UqTmp,-6.0,6.0),0,angtmp);
        // SVPWM_CTL(_constrain(UqTmp,-6.0,6.0),0,angtmp);
        FOC_TickTask();
		vTaskDelayUntil(&Time,1/portTICK_PERIOD_MS);
    }
	vTaskDelete(NULL);
}







void FOC_TickTask()
{
    SetPWMDuty(UA_Phase,(uint8_t)(Ua*100/VCC_MOTOR));
    SetPWMDuty(UB_Phase,(uint8_t)(Ub*100/VCC_MOTOR));
    SetPWMDuty(UC_Phase,(uint8_t)(Uc*100/VCC_MOTOR));

    // printf("FOC:%.2f,%.2f,%.2f\n",(Ua*100/VCC_MOTOR),(Ub*100/VCC_MOTOR),(Uc*100/VCC_MOTOR));

}



// void BLDCMotor::setPhaseVoltage(float Uq, float angle_el) {
//   switch (foc_modulation)
//   {
//     case FOCModulationType::SinePWM :
//       // 正弦PWM调制
//       // 逆派克+克拉克变换

//       // 在0到360°之间的角度归一化
//       // 只有在使用 _sin和 _cos 近似函数时才需要
//       angle_el = normalizeAngle(angle_el + zero_electric_angle);
//       // 逆派克变换
//       Ualpha =  -_sin(angle_el) * Uq;  // -sin(angle) * Uq;
//       Ubeta =  _cos(angle_el) * Uq;    //  cos(angle) * Uq;

//       // 克拉克变换
//       Ua = Ualpha + voltage_power_supply/2;
//       Ub = -0.5 * Ualpha  + _SQRT3_2 * Ubeta + voltage_power_supply/2;
//       Uc = -0.5 * Ualpha - _SQRT3_2 * Ubeta + voltage_power_supply/2;
//       break;

//     case FOCModulationType::SpaceVectorPWM :
//       // 解释空间矢量调制(SVPWM)算法视频
//       // https://www.youtube.com/watch?v=QMSWUMEAejg

//       // 如果负电压的变化与相位相反
//       // 角度+180度
//       if(Uq < 0) angle_el += _PI;
//       Uq = abs(Uq);

//       // 在0到360°之间的角度归一化
//       // 只有在使用 _sin和 _cos 近似函数时才需要
//       angle_el = normalizeAngle(angle_el + zero_electric_angle + _PI_2);

//       // 找到我们目前所处的象限
//       int sector = floor(angle_el / _PI_3) + 1;
//       // 计算占空比
//       float T1 = _SQRT3*_sin(sector*_PI_3 - angle_el) * Uq/voltage_power_supply;
//       float T2 = _SQRT3*_sin(angle_el - (sector-1.0)*_PI_3) * Uq/voltage_power_supply;
//       // 两个版本
//       // 以电压电源为中心/2
//       float T0 = 1 - T1 - T2;
//       // 低电源电压，拉到0
//       //float T0 = 0;

//       // 计算占空比（时间）
//       float Ta,Tb,Tc; 
//       switch(sector){
//         case 1:
//           Ta = T1 + T2 + T0/2;
//           Tb = T2 + T0/2;
//           Tc = T0/2;
//           break;
//         case 2:
//           Ta = T1 +  T0/2;
//           Tb = T1 + T2 + T0/2;
//           Tc = T0/2;
//           break;
//         case 3:
//           Ta = T0/2;
//           Tb = T1 + T2 + T0/2;
//           Tc = T2 + T0/2;
//           break;
//         case 4:
//           Ta = T0/2;
//           Tb = T1+ T0/2;
//           Tc = T1 + T2 + T0/2;
//           break;
//         case 5:
//           Ta = T2 + T0/2;
//           Tb = T0/2;
//           Tc = T1 + T2 + T0/2;
//           break;
//         case 6:
//           Ta = T1 + T2 + T0/2;
//           Tb = T0/2;
//           Tc = T1 + T0/2;
//           break;
//         default:
//          // 可能的错误状态
//           Ta = 0;
//           Tb = 0;
//           Tc = 0;
//       }

//       // 计算相电压和中心
//       Ua = Ta*voltage_power_supply;
//       Ub = Tb*voltage_power_supply;
//       Uc = Tc*voltage_power_supply;
//       break;
//   }
  
//   // 设置硬件中的电压
//   setPwm(Ua, Ub, Uc);
// }


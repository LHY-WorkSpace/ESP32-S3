#ifndef FOC_H
#define FOC_H

typedef enum
{
    UA_Phase,
    UB_Phase,
    UC_Phase,
    U_PhaseMax,
}U_Phase_e;

typedef enum
{
    PWM_A,
    PWM_B,
    PWM_Max,
}PwmChannel_e;


void FOC_GPIO_Init(void);
void FOC_main(void);
void PWM_Task(void);
void SetPWMDuty(uint8_t Phase,uint8_t Value);
void Foc_CTL(void);
void FOC_Task(void);
#endif


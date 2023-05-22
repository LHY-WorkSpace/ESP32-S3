#ifndef FOC_H
#define FOC_H

typedef enum
{
    UA_Timer,
    UB_Timer,
    UC_Timer,
    U_TimerMax,
}U_Timer_e;



void FOC_GPIO_Init(void);
void FOC_main(void);
void ERRER(void);


/*









*/






#endif
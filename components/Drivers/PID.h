#ifndef PID_H
#define PID_H

typedef struct 
{
	float TargetValue;//定义设定值
	float err;//定义偏差值
	float err_next;//定义上一个偏差值
	float err_last;//定义最上前的偏差值
	float Kp, Ki, Kd;//定义比例、积分、微分系数
	float IntegralValue;//积分值
	float OutValue;//输出值
}PID_t;



void PID_Init(PID_t *PID_Data);
void PID_Change_Kp(PID_t *PID_Data, float k);
void PID_Change_Ki(PID_t *PID_Data, float k);
void PID_Change_Kd(PID_t *PID_Data, float k);
void PID_SetTarget(PID_t *PID_Data, float k);
float PID_Process(PID_t *PID_Data,float Target);

#endif







#include <stdio.h>
#include <inttypes.h>
#include "MathFun.h"
#include "PID.h"



// static PID_t pid;


//***************************************************//
//  功能描述: PID参数初始化
//  
//  参数: 无
//  
//  返回值: 无
//  
//  说明: 无
//  
//***************************************************//
void PID_Init(PID_t *PID_Data)
{
	PID_Data->TargetValue = 0.0;
	PID_Data->IntegralValue = 0.0;
	PID_Data->OutValue = 0.0;
	PID_Data->err = 0.0;
	PID_Data->err_last = 0.0;
	PID_Data->err_next = 0.0;
	//可设置下面的值，从而达到最佳效果
	PID_Data->Kp = 0.0;
	PID_Data->Ki = 0.0; 
	PID_Data->Kd = 0.0;
}

//***************************************************//
//  功能描述: 修改PID系数
//  
//  参数: P I D 系数
//  
//  返回值: 无
//  
//  说明: 无
//  
//***************************************************//
void PID_Change_Kp(PID_t *PID_Data, float k)
{
	PID_Data->Kp = k;
}

void PID_Change_Ki(PID_t *PID_Data, float k)
{
	PID_Data->Ki = k; 
}

void PID_Change_Kd(PID_t *PID_Data, float k)
{
	PID_Data->Kd = k;
}
void PID_SetTarget(PID_t *PID_Data, float k)
{
	PID_Data->TargetValue = k;
}
//***************************************************//
//  功能描述: PID处理函数
//  
//  参数:设定值
//  
//  返回值: 当前值
//  
//  说明: 无
//  
//***************************************************//
float PID_Process(PID_t *PID_Data,float Actual)
{
	PID_Data->err = PID_Data->TargetValue - Actual;

	PID_Data->IntegralValue += PID_Data->err;

    PID_Data->OutValue = PID_Data->Kp*(PID_Data->err) 
					   + PID_Data->Ki*(PID_Data->IntegralValue) 
					   + PID_Data->Kd*(PID_Data->err - PID_Data->err_last);

	PID_Data->err_last = PID_Data->err;

	return PID_Data->OutValue;
}

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
	PID_Data->SetValue = 0.0;
	PID_Data->ActualValue = 0.0;
	PID_Data->err = 0.0;
	PID_Data->err_last = 0.0;
	PID_Data->err_next = 0.0;
	//可设置下面的值，从而达到最佳效果
	PID_Data->Kp = 0.1;
	PID_Data->Ki = 0.1; 
	PID_Data->Kd = 0.3;
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
float PID_Process(PID_t *PID_Data,float Target)
{
	float incrementValue;

	PID_Data->SetValue = Target;
	PID_Data->err = PID_Data->SetValue - PID_Data->ActualValue;
    incrementValue = PID_Data->Kp*(PID_Data->err - PID_Data->err_next) + PID_Data->Ki*PID_Data->err + PID_Data->Kd*(PID_Data->err - 2 * PID_Data->err_next + PID_Data->err_last);//计算出增量
	PID_Data->ActualValue += incrementValue;
	PID_Data->err_last = PID_Data->err_next;
	PID_Data->err_next = PID_Data->err;
	return PID_Data->ActualValue;
}

#ifndef  _AS5600_H_
#define  _AS5600_H_

#include "DataType.h"

#define AS5600_ADDRESS      (0x36 << 1) 


//========= Configuration Registers =========
#define ZMCO_REG        (0x00)//

#define ZPOS_H_REG      (0x01)//
#define ZPOS_L_REG      (0x02)//

#define MPOS_H_REG      (0x03)//
#define MPOS_L_REG      (0x04)//

#define NABG_H_REG      (0x05)//
#define MANG_L_REG      (0x06)//

#define CONF_H_REG      (0x07)//
#define CONF_L_REG      (0x08)//



//========= Data Registers =========
#define RAW_ANGLE_H_REG      (0x0C)//
#define RAW_ANGLE_L_REG      (0x0D)//

#define ANGLE_H_REG      (0x0E)//
#define ANGLE_L_REG      (0x0F)//



//========= Status Registers =========
#define STATUS_REG      (0x0B)//

#define AGC_REG         (0x1A)//

#define MAGNITUDE_H_REG      (0x1B)//
#define MAGNITUDE_L_REG      (0x1C)//

//========= Burn Commands =========
#define BURN            (0xFF)//

void AS5600_Init(void);
void AS5600_WriteData(uint8_t addr,uint8_t length,uint8_t *data);
void AS5600_ReadData(uint8_t addr,uint8_t length,uint8_t *data);
float AS5600_Angle(void);
void AS5600_UpdateAngle(void);
void AS5600_Test(void);
#endif




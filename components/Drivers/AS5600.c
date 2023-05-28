#include "AS5600.h"
#include "driver/i2c.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO		17    /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO		18    /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM			I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ		600000     /*!< I2C master clock frequency */
#define WRITE_BIT				I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT				I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN			0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS			0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL					0x0         /*!< I2C ack value */
#define NACK_VAL				0x1         /*!< I2C nack value */

//************************// 
//  功能描述: AS5600_ IO 初始化函数
//  
//  参数: 无
//  
//  返回值: TRUE:成功  
//          FALSE:失败
//          0xFF:地址超范围
//			
//  说明: 
//
//************************//  
void AS5600_Init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    static  i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	conf.clk_flags = 0;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

//************************// 
//  功能描述: AS5600_ 数据写入函数
//  
//  参数: 物理地址，长度，数据指针
//  
//  返回值: TRUE:成功  
//          FALSE:失败
//          0xFF:地址超范围
//			
//  说明: 
//
//************************//  
void  AS5600_WriteData(uint8_t addr,uint8_t length,uint8_t *data)
{
	uint8_t i;
	i2c_cmd_handle_t cmd;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd,( AS5600_ADDRESS | WRITE_BIT ), ACK_CHECK_EN);
	i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);
	i2c_master_write(cmd,data, length, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 10 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

}



//************************// 
//  功能描述: AS5600_ 数据读取函数
//  
//  参数: 物理地址，长度，数据指针
//  
//  返回值: TRUE:成功  
//          FALSE:失败
//          0xFF:地址超范围
//			
//  说明: 
//
//************************//  
void AS5600_ReadData(uint8_t addr,uint8_t length,uint8_t *data)
{
	uint8_t i;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
	i2c_master_write_byte(cmd,( AS5600_ADDRESS | WRITE_BIT ) , ACK_CHECK_EN);
	i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd,( AS5600_ADDRESS | READ_BIT ), ACK_CHECK_EN);

	for(i=0;i<length;i++)
	{
		if( i == length-1)
		{
			i2c_master_read_byte(cmd,&data[i],NACK_VAL);
		}
		else
		{
			i2c_master_read_byte(cmd,&data[i],ACK_VAL);
		}
	}


    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}




void AS5600_Test()
{
    TickType_t Time;	
    Time=xTaskGetTickCount();
	B16_B08 Angle;
	uint16_t AngleTmp=0;
	AS5600_Init();
    while (1)
    {
		memset(Angle.B08,0,sizeof(B16_B08));
		AS5600_ReadData(RAW_ANGLE_L_REG,1,&Angle.B08[0]);
		AS5600_ReadData(RAW_ANGLE_H_REG,1,&Angle.B08[1]);
		AngleTmp = Angle.B16*360/4096;
		printf("Angle:%d\r\n",AngleTmp);
		vTaskDelayUntil(&Time,10/portTICK_PERIOD_MS);
    }

	vTaskDelete(NULL);
}











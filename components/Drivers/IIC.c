#include "IIC.h"
#include "driver/i2c.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"




#define I2C_MASTER_SCL_IO		17    /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO		18    /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM			I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ		200000     /*!< I2C master clock frequency */
#define WRITE_BIT				I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT				I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN			0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS			0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL					0x0         /*!< I2C ack value */
#define NACK_VAL				0x1         /*!< I2C nack value */


void i2c_master_init()
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


void  EEPROMWriteData(uint16_t addr,uint16_t length,uint8_t *data)
{
	static uint8_t times;
	uint8_t k,i;
	uint16_t PageNum,WR_Len,Offset,LenCount;
	B16_B08 MemAddr;
    i2c_cmd_handle_t cmd;
	esp_err_t ret;
	if( addr >= EEPROM_PAGE_SIZE*EEPROM_PAGES)
	{
		return;
	}
	
	MemAddr.B16 = addr;

	PageNum = addr/EEPROM_PAGE_SIZE;
	Offset = addr - PageNum*EEPROM_PAGE_SIZE;	
	WR_Len = EEPROM_PAGE_SIZE - Offset;	
	LenCount = 0;

	if( WR_Len >= length )	
	{
		WR_Len = length;
	}

	do
	{
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd,( EEPROM_ADDRESS | WRITE_BIT |MemAddr.B08[1]<<1 ), ACK_CHECK_EN);
		i2c_master_write_byte(cmd, MemAddr.B08[0], ACK_CHECK_EN);
		i2c_master_write(cmd,(data+LenCount), WR_Len, ACK_CHECK_EN);
		LenCount +=  WR_Len;
   		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		vTaskDelay(6 / portTICK_PERIOD_MS);
		MemAddr.B16 += WR_Len;

		if( (length - LenCount) >= EEPROM_PAGE_SIZE)
		{
			WR_Len = EEPROM_PAGE_SIZE;
		}
		else
		{
			WR_Len = length - LenCount;
		}
		times++;

	}while(LenCount != length);

}

void EEPROMReadData(uint16_t addr,uint16_t length,uint8_t *data)
{
	uint16_t i;
	B16_B08 MemAddr;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	MemAddr.B16=addr;

    i2c_master_start(cmd);
	i2c_master_write_byte(cmd,( EEPROM_ADDRESS | WRITE_BIT | (MemAddr.B08[1]<<1) ), ACK_CHECK_EN);
	i2c_master_write_byte(cmd,MemAddr.B08[0], ACK_CHECK_EN);
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd,( EEPROM_ADDRESS | READ_BIT | MemAddr.B08[1]<<1 ), ACK_CHECK_EN);
	i2c_master_read(cmd, data, length, ACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
	vTaskDelay(2 / portTICK_PERIOD_MS);

}



void IIC_Test()
{
	uint8_t Data[200];

	i2c_master_init();

	for (uint8_t i = 0; i < sizeof(Data); i++)
	{
		Data[i] = 0x11+i;
	}
	EEPROMWriteData(0,sizeof(Data),Data);

	memset(Data,0,sizeof(Data));

	EEPROMReadData(0,sizeof(Data),Data);
	for (uint8_t i = 0; i < sizeof(Data); i++)
	{
		printf("Val:%d\r\n",Data[i]);
		vTaskDelay(2/ portTICK_PERIOD_MS);
	}

}
























































































































#if 0

/*
IIC_SCL     GPIO_Pin_6                                       
IIC_SDA     GPIO_Pin_7
*/


void IIC_Init()
{



}


void IIC_Delay(uint32_t nus)
{
	uint32_t i,k;

	for(k=0; k<nus; k++)
	{
		for(i=0; i<10; i++)
		{
			__NOP();
		}
	}
}



void Start_IIC(void)
{
	IIC_SDA_HIGH;
	IIC_Delay(2);
	IIC_SCL_HIGH;
	IIC_Delay(2);
	IIC_SDA_LOW;
	IIC_Delay(2);
	IIC_SCL_LOW;
	IIC_Delay(2);

}


void Stop_IIC(void)
{
	IIC_SCL_LOW;
	IIC_Delay(2);
	IIC_SDA_LOW;
	IIC_Delay(2);
	IIC_SCL_HIGH;
	IIC_Delay(2);
	IIC_SDA_HIGH;
	IIC_Delay(2);

}



void IIC_Send_Ack(void)
{
	IIC_SCL_LOW;
	IIC_SDA_LOW;
	IIC_Delay(2);
	IIC_SCL_HIGH;
	IIC_Delay(2);
	IIC_SCL_LOW;
	IIC_Delay(2);
	IIC_SDA_HIGH;
	IIC_Delay(2);
}



void IIC_Send_NAck(void)
{
	IIC_SCL_LOW;
	IIC_SDA_HIGH;
	IIC_Delay(2);
	IIC_SCL_HIGH;
	IIC_Delay(2);
	IIC_SCL_LOW;
	IIC_Delay(2);
	IIC_SDA_HIGH;
	IIC_Delay(2);
}


uint8_t IIC_Wait_Ack_OK(void)
{
	uint8_t i=0;

	IIC_SCL_HIGH; 
	IIC_Delay(2);                              
	while( IIC_SDA_STATE ==1)
	{
			i++;
			if(i>10)
			{
				Stop_IIC();
				return FALSE;
			}
			IIC_Delay(2);	
	}			
	IIC_SCL_LOW;	
	IIC_Delay(2);
	IIC_SDA_HIGH;
	return TRUE;
}

void IIC_SenddByte(uint8_t data)
{

	uint8_t i=0;
	
	for(i=0;i<8;i++)
	{
		if(data&0x80)	
		{
			IIC_SDA_HIGH;
		}
		else
		{
			IIC_SDA_LOW;
		}
		data<<=1;
		IIC_Delay(2);
		IIC_SCL_HIGH;
		IIC_Delay(2);
		IIC_SCL_LOW;
	}
	IIC_Delay(2);  
	IIC_SDA_HIGH;               
}


uint8_t IIC_GetByte(void)
{
	uint8_t data=0;
	uint8_t i=0;

	for(i=0;i<8;i++)
	{		
			data<<=1;
			IIC_SCL_LOW;
			IIC_Delay(2); 		
			IIC_SCL_HIGH;	
			IIC_Delay(2);
			if( IIC_SDA_STATE ==1)	
			{
      			data|=0x01;
			}

	}
 	IIC_SCL_LOW;	
	IIC_Delay(2);
	IIC_SDA_HIGH;
  return data;
}

#endif


































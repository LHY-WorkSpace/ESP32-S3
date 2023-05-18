#ifndef  I2C_H
#define  I2C_H
#include <stdio.h>
#include <inttypes.h>

typedef union 
{
    uint8_t  B08[2];
    uint16_t B16;
}B16_B08;

#define EEPROM_PAGE_SIZE                 (16)   //�ֽ�
#define EEPROM_PAGES                     (64)   //ҳ�� �ڲ�Ram��ҳ(16 Byte)����
#define EEPROM_ADDRESS                   (0xA0)  //AT24C02 �豸��ַ


void IIC_Test(void);
void EEPROMReadData(uint16_t addr,uint16_t length,uint8_t *data);
void  EEPROMWriteData(uint16_t addr,uint16_t length,uint8_t *data);

#endif

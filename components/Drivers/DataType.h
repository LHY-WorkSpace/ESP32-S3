#ifndef  DATATYPE_H
#define  DATATYPE_H

#include <stdio.h>
#include <inttypes.h>

#define  HIGH   1
#define  LOW    0

#define	Bit_0	0x0001
#define	Bit_1	0x0002
#define	Bit_2	0x0004
#define	Bit_3	0x0008
#define	Bit_4	0x0010
#define	Bit_5	0x0020
#define	Bit_6	0x0040
#define	Bit_7	0x0080
#define	Bit_8	0x0100
#define	Bit_9	0x0200
#define	Bit_10	0x0400
#define	Bit_11	0x0800
#define	Bit_12	0x1000
#define	Bit_13	0x2000
#define	Bit_14	0x4000
#define	Bit_15	0x8000



typedef union 
{
    uint8_t  B08[2];
    uint16_t B16;
}B16_B08;

typedef union 
{
    uint8_t  B08[4];
    uint32_t B32;
}B32_B08;

typedef union 
{
    uint16_t B16[2];
    uint32_t B32;
}B32_B16;


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef char s8;
typedef short s16;
typedef int s32;



#endif







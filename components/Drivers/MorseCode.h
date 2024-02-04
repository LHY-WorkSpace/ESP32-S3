#ifndef  MORSE_H
#define  MORSE_H


#define DOT 	6
#define LINE    (3*(DOT))

typedef struct
{
	char Letter;
    u8   MorseCodeLen;
	u16  MorseCode[8];
}CodeTable_t;




void  MorseCode_Init(void);
void MorseCodeSend(char *Data);
void MorseCodeTimerTick(void);
#endif




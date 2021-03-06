#ifndef UTIL_H_
#define UTIL_H_

#include "p33Fxxxx.h"

#define int8 char
#define uint8 unsigned char

#define int16 signed short int
#define uint16 unsigned short int

#define int32 long signed int
#define uint32 long unsigned int

#define Fosc 7370000
//#define Fosc 6950000
#define Fy (Fosc / 2)
#define SAMPLE_RATE 8000

//#define D4 PORTBbits.RB0
//#define D5 PORTBbits.RB1
//#define D6 PORTBbits.RB2
//#define D7 PORTBbits.RB3
//#define RS PORTBbits.RB4
//#define RW PORTBbits.RB5
//#define E PORTDbits.RD9

void io_init();
void adc_init();
void sample_timer_init();

void delay32_2(unsigned long int delay);
uint16 bitrev(uint16 in, uint8 bits);

#endif

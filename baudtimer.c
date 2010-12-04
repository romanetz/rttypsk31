#include "baudtimer.h"

//Timer control routines
void baud_timer_init() {
	PR1 = 0xFFFF;
	TMR1 = 0x0000;
	T1CONbits.TCKPS = 0b01;
	T1CONbits.TON = 1;
}

void baud_timer_restart() {
	TMR1 = 0x0000;
}

unsigned int baud_time_get() {
	unsigned int tmp = TMR1;
	TMR1 = 0x0000;
	return tmp;
}

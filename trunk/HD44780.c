#include "HD44780.h"

unsigned int display_offset = 0;
unsigned int display_pause = 1000;

#define RS PORTBbits.RB6
#define RW PORTBbits.RB9
#define E PORTBbits.RB7

#define D4 PORTBbits.RB5
#define D5 PORTBbits.RB8
#define D6 PORTBbits.RB4
#define D7 PORTAbits.RA4

//Initialize the display
void setDispPortSend() {
	TRISBbits.TRISB6 = 0;
	TRISBbits.TRISB9 = 0;
	TRISBbits.TRISB7 = 0;

	TRISBbits.TRISB5 = 0;
	TRISBbits.TRISB8 = 0;
	TRISBbits.TRISB4 = 0;
	TRISAbits.TRISA4 = 0;
}

void setDispPortReceive() {
	TRISBbits.TRISB6 = 0;
	TRISBbits.TRISB9 = 0;
	TRISBbits.TRISB7 = 0;

	TRISBbits.TRISB5 = 1;
	TRISBbits.TRISB8 = 1;
	TRISBbits.TRISB4 = 1;
	TRISAbits.TRISA4 = 1;
}

void setDispPort(unsigned int rwrs, unsigned int x) {
	D4 = ((x) >> 0) & 0x1;
	D5 = ((x) >> 1) & 0x1;
	D6 = ((x) >> 2) & 0x1;
	D7 = ((x) >> 3) & 0x1;
	RW = ((rwrs) >> 1) & 0x1;
	RS = ((rwrs) >> 0) & 0x1;
}

void initDisplay() {
	setDispPortSend();
	//Wait at least 15ms
	__delay32(17500000);

	setDispPort(0, 0b0011);
	E = 1; delay(1000); E = 0;

	//Wait at least 4.1ms
	__delay32(1700000);

	setDispPort(0, 0b0011);
	E = 1; delay(1000); E = 0;

	//Wait at least 100microseconds
	__delay32(300000);

	setDispPort(0, 0b0011);
	E = 1; delay(1000); E = 0;

	//Wait about 5ms
	__delay32(1500000);
	
	setDispPort(0, 0b0010);
	E = 1; delay(1000); E = 0;
	__delay32(1500000);

	comDisplay(0, HD_CMD_function);
	comDisplay(0, 0b00001000);
	comDisplay(0, HD_CMD_clear);
	comDisplay(0, HD_CMD_mode);
	comDisplay(0, HD_CMD_display);
}

//Send data to display
void comDisplay(unsigned short int rwrs, unsigned short int val) {
	//Check to see if display is busy
	setDispPortReceive();
	RS = 0;
	RW = 1;
	while(1) {
		E = 1;
		delay(display_pause);
		if(!D7) {
			break;
		}
		E = 0;
		delay(display_pause);
		E = 1;
		delay(display_pause);
		E = 0;
		delay(display_pause);
	}
	setDispPortSend();

	setDispPort(rwrs, 0xF & (val >> 4));
	E = 1;
	delay(display_pause);
	E = 0;
	delay(display_pause);

	setDispPort(rwrs, 0xF & val);
	E = 1;
	delay(display_pause);
	E = 0;
	delay(10 * display_pause);
}

printDisplay(unsigned int character) {
	if(display_offset <= 7) {
		comDisplay(1, character);
	} else {
		if(display_offset == 8) {
			comDisplay(0, HD_CMD_DDaddress | 0x40);
		}
		if(display_offset > 15) {
			comDisplay(0, HD_CMD_return);
			display_offset = 0;
		}
		comDisplay(1, character);
	}
	display_offset++;
}

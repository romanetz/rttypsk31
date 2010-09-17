#include "HD44780.h"

unsigned int display_offset = 0;
unsigned int display_pause = 1000;

//Initialize the display
void initDisplay() {
	//Wait at least 15ms
	__delay32(17500000);

	PORTB = 0b000011;
	E = 1; delay(1000); E = 0;

	//Wait at least 4.1ms
	__delay32(1700000);

	PORTB = 0b000011;
	E = 1; delay(1000); E = 0;

	//Wait at least 100microseconds
	__delay32(300000);

	PORTB = 0b000011;
	E = 1; delay(1000); E = 0;

	//Wait about 5ms
	__delay32(1500000);
	
	PORTB = 0b000010;
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
	TRISB = 0b1111001111;
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
	TRISB = 0b1111000000;

	PORTB = ((rwrs << 4) | 0xF & (val >> 4));
	E = 1;
	delay(display_pause);
	E = 0;
	delay(display_pause);

	PORTB = ((rwrs << 4) | 0xF & val);
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

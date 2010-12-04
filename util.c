#include "util.h"

void io_init() {
	//Port B
	//First bit (from left) for analog in
	//Next bit nothing
	//Next 2 bits (from left) are input for rotary encoder
	//Last bits are outputs for display
	TRISA = 0xFFFF;
}

void adc_init() {
	AD1PCFGLbits.PCFG0 = 0;
	AD1CON1bits.ADON = 0;
	AD1CON1bits.AD12B = 1; // Enable 12bit conversion
	AD1CON1bits.FORM = 0; // Signed fractional output (_Q15)0x03E0;
	AD1CON1bits.SSRC = 7; // Automatic conversion
	AD1CON1bits.ASAM = 0; // Sampling begines when ADCON1bits.SAMP is set
	AD1CON2 = 0x0000;
	AD1CON3bits.ADRC = 0; // Clock derived from system clock
	AD1CON3bits.SAMC = 14; // Sample time = 20 * Tad. Minimum of 14 * Tad for 12bit conversion
	AD1CON3bits.ADCS = 5; // Tad must be > 117.6nS. 1Mhz clock -> 1uS Tad
					      // At max Tcy is 40Mhz, so just divide by 40 and we'll be fine
					     
	//Total ADC time is 17 Tad, or 23 * 40 * Ty, or 920 clocks
	AD1CHS0 = 0x0000;
	
	AD1CON1bits.ADON = 1; // Enable ADC
}

void sample_timer_init() {
	T3CONbits.TON = 0; // Disable Timer
	T3CONbits.TCS = 0; // Select internal instruction cycle clock
	T3CONbits.TGATE = 0; // Disable Gated Timer mode
	T3CONbits.TCKPS = 0b00; // Select 1:1 Prescaler
	TMR3 = 0x00; // Clear timer register
	
	PR3 = Fy / SAMPLE_RATE; // Load the period value
	
	IPC2bits.T3IP = 0x01; // Set Timer1 Interrupt Priority Level
	IFS0bits.T3IF = 0; // Clear Timer1 Interrupt Flag
	IEC0bits.T3IE = 1; // Enable Timer1 interrupt
	T3CONbits.TON = 1;
}


void delay32_2(unsigned long int delay) {
	unsigned long int x = 0;
	unsigned int z = 0;
	for(x = 0; x < delay; x++) {
		z++;
	}
}

uint16 bitrev(uint16 in, uint8 bits) {
	uint16 out, marker;
	uint8 i;
	
	if(bits == 0)
		return 0;
	
	out = 0;
	marker = 0x1 << (bits - 1);
	
	for(i = 0; i < bits; i++) {
		out >>= 1;
		out = out | ((in & marker) << i);
		marker >>= 1;
	}
	
	return out;
}

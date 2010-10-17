#include "dsp.h"
#include "libq.h"

#include "limits.h"
#include "float.h"
#include "math.h"

#include "HD44780.h"
#include "fractionaltypes.h"

#define DELAY_LENGTH 64

//#define MARK_FREQUENCY 1275.0
//#define SPACE_FREQUENCY 1445.0

//_FOSCSEL(FNOSC_FRC);

#define Fosc 7370000
#define Fy (Fosc / 2)
#define SAMPLE_RATE 8000


//This just wraps around all the other intializiation functions
void init();

void initIO();
void initADC();

//Initialize all the RTTY decode junk
void initRTTY();
uint16 symbolIndex;
uint16 symbolArray[32];

#define none 2
#define mark 1
#define space 0
uint16 currentSymbol;
uint16 process;

//Baud timer control functions
void initBaudTimer();
void restartBaudTimer();
unsigned int getBaudTime();

//Sample timer control functions
void initSampleTimer();

//RTTY decode utility functions
uint16 validateRTTY(uint16 character);
void printRTTY(unsigned int character);

void delay32_2(unsigned long int delay);

//signed int spacevar[FFT_BLOCK_LENGTH] __attribute__ ((space (xmemory), aligned (FFT_BLOCK_LENGTH * 2)));
unsigned short int sample_idx;
F15 sample[DELAY_LENGTH];// __attribute__ ((space (xmemory)));
unsigned int ed_idx = 0;
F16 ed[256];

#define RTTY_NULL 0b10000000
#define RTTY_LF 0b10000001
#define RTTY_SP 0b10000010
#define RTTY_CR 0b10000011
#define RTTY_FIGS 0b10000100
#define RTTY_LTRS 0b10000101
#define RTTY_ENQ 0b10000110
#define RTTY_BEL 0b10000111

unsigned int rtty_mode = RTTY_LTRS;

//RTTY notes:
//RTTYrite has a " in stead of a +, we use that.
//Also, it sends BEL in place of '.

const unsigned int RTTY_LETTERS_TO_HANTRONIX[32] = {
RTTY_NULL,//0x00
HD_cap_E,//0x01
RTTY_LF,//0x02
HD_cap_A,//0x03
RTTY_SP,//0x04
HD_cap_S,//0x05
HD_cap_I,//0x06
HD_cap_U,//0x07
RTTY_CR,//0x08
HD_cap_D,//0x09
HD_cap_R,//0x0A
HD_cap_J,//0x0B
HD_cap_N,//0x0C
HD_cap_F,//0x0D
HD_cap_C,//0x0E
HD_cap_K,//0x0F
HD_cap_T,//0x10
HD_cap_Z,//0x11
HD_cap_L,//0x12
HD_cap_W,//0x13
HD_cap_H,//0x14
HD_cap_Y,//0x15
HD_cap_P,//0x16
HD_cap_Q,//0x17
HD_cap_O,//0x18
HD_cap_B,//0x19
HD_cap_G,//0x1A
RTTY_FIGS,//0x1B
HD_cap_M,//0x1C
HD_cap_X,//0x1D
HD_cap_V,//0x1E
RTTY_LTRS//0x1F
};

const unsigned int RTTY_FIGURES_TO_HANTRONIX[32] = {
RTTY_NULL,//0x00
HD_three,//0x01
RTTY_LF,//0x02
HD_subtract,//0x03
RTTY_SP,//0x04
HD_apostrophe,//0x05
HD_eight,//0x06
HD_seven,//0x07
RTTY_CR,//0x08
RTTY_ENQ,//0x09
HD_four,//0x0A
RTTY_BEL,//0x0B
HD_comma,//0x0C
HD_exclamation,//0x0D
HD_colon,//0x0E
HD_leftparenthesis,//0x0F
HD_five,//0x10
HD_doublequote,//0x11 //Should be HD_add, but RTTYrite is non-standard
HD_rightparenthesis,//0x12
HD_two,//0x13
HD_pound,//0x14
HD_six,//0x15
HD_zero,//0x16
HD_one,//0x17
HD_nine,//0x18
HD_question,//0x19
HD_and,//0x1A
RTTY_FIGS,//0x1B
HD_period,//0x1C
HD_divide,//0x1D
HD_semicolon,//0x1E
RTTY_LTRS//0x1F
};

int main(void)
{
	unsigned int x = 0;
	
	symbolIndex = 0;

	RCONbits.SWDTEN = 0;
	init();
	IPC1 = 0xFFFF;

	IEC0bits.T3IE = 1;
	IFS0bits.T3IF = 0;

	//Decode loop

	//Useful Constants
	const int16 symbolTime = (uint16)((((float)Fosc / 2.0) / 8.0) / 45.45); // The timer is driven by Fosc / 2 through a 256 prescaler and we are looking for 45.45 baud symbols
	const int16 switchTime = symbolTime / 5;
	
	//Decode state variables
	int32 markTime = 0, spaceTime = 0,
		decodeSymbol = none, dam = 0;
		
	uint16 processMark = 0, processSpace = 0,
		character = 0, symbolCount = 0;
		
	process = 0;
	
	restartBaudTimer();
	while(1) {
		if(process == 0)
			continue;
			
		uint16 Telaps = getBaudTime();
		
		//Watch for a symbol switch
		//  If an unexpected signal detection change occurs, track it to see if real
		if(currentSymbol == decodeSymbol) {
			//If it turns out to be fake, do nothing (because time added to right bin anyway)
			dam = 0;
		} else {
			//If it turns out to be real, evaluate other timer and switch decode symbol 
			dam += Telaps;
			
			if(dam > switchTime) {
				//If need to switch symbols, evaluate the old decodeSymbol before updating to new
				switch(decodeSymbol) {
					case mark:
						if(currentSymbol == space) spaceTime = dam - Telaps; //The '- Telaps' term avoids double adding
						markTime -= dam;
						processMark = 1;
						break;
					case space:
						if(currentSymbol == mark) markTime = dam - Telaps;
						spaceTime -= dam;
						processSpace = 1;
						break;
				}

				//Update to new symbol
				decodeSymbol = currentSymbol;

				//Reset the dam
				dam = 0;
			}	
		}
		
		
		switch(decodeSymbol) {
			case mark:
				markTime += Telaps;
				break;
			case space:
				spaceTime += Telaps;
				break;
		}
		
		if(processMark) {
			while(markTime > symbolTime - switchTime) {
 				character = (character << 1) | 0x0001;
 				
 				symbolArray[symbolIndex] = 1;
 				if(symbolIndex == 32)
 					symbolIndex = 0;
 				else
 					symbolIndex++;
 					
				markTime -= symbolTime;
				symbolCount++;
			}
			
			markTime = 0;
			processMark = 0;
		}
		
		if(processSpace) {
			while(spaceTime > symbolTime - switchTime) {
 				character = (character << 1) & 0xFFFE;
 				
 				symbolArray[symbolIndex] = 0;
 				if(symbolIndex == 32)
 					symbolIndex = 0;
 				else
 					symbolIndex++;
 					
				spaceTime -= symbolTime;
				symbolCount++;
			}
			
			spaceTime = 0;
			processSpace = 0;
		}
		
		while(symbolCount >= 8) {
			uint16 overshoot = symbolCount - 8;
			uint16 adjustedCharacter = character >> overshoot;
			
			if(validateRTTY(adjustedCharacter)) {
				printRTTY(adjustedCharacter);
				symbolCount -= 8;
			} else
				symbolCount--;
		}	
		
		process = 0;
	}
}

//Initialize everything
void init() {
	initIO();
	initADC();
	initBaudTimer();
	//initDisplay();
	
	sample_idx = 0;
	
	initRTTY();
	
	initSampleTimer(); // We start this timer last so that processing will only begin after all initialization is finished
}

void initIO() {
	//Port B
	//First bit (from left) for analog in
	//Next bit nothing
	//Next 2 bits (from left) are input for rotary encoder
	//Last bits are outputs for display
	TRISA = 0xFFFF;
}

void initADC() {
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

F16 td,
	D0, D1, kftau,
	negOne,
	atan_lookup_table[16][16];

void initRTTY() {
	const float pi = 3.14159265f;

	float Fs, Ts,
		psi,
		tau,
		Fmark, Fspace,
		F0, W0, T0,
		K1, G1;

	psi = PI / 3.0f;

	Fs = (float)SAMPLE_RATE;
	Ts = (1.0f / Fs);
	Fmark = 1275.0f;
	Fspace = 1445.0f;
	F0 = (Fmark + Fspace) / 2.0f;
	W0 = 2.0f * pi * F0;
	T0 = 1.0f / F0;

	tau = psi / W0;

	K1 = 1.0f;
	G1 = K1 / W0;

	D0 = floatToF16(Fs * T0);
	D1 = floatToF15(Fs * G1);
	kftau = floatToF16(Fs * tau);
	td = kftau;

	int16 ix, iy;
	for(ix = -8; ix < 8; ix++) {
		for(iy = -8; iy < 8; iy++) {
			float x = (float)ix, y = (float)iy,	atan = 0.0f;

			if(y == 0) {
				if(x >= 0) {
					atan = 0.0f;
				} else {
					atan = -1.0f * pi;
				}	
			} else if(x == 0) {
				if(y > 0) {
					atan = pi / 2.0f;
				} else {
					atan = pi / -2.0f;
				}	
			} else {	
				atan = atan2f(y, x);
			}	

			atan_lookup_table[iy + 8][ix + 8] = floatToF16(atan);
		}
	}
	
	currentSymbol = 0;
}

uint8 count_leading_unused_bits(uint16 x) {
	uint8 i = 0, j = 0;
	
	uint8 sign = (x >> 15) & 0x1;
	
	if(sign == 0x1)
		x != x;
	
	x <<= 1;
	
	while((x & 0x8000) == 0 && i < 16) {
		i++;
		x <<= 1;
	}
	
	return i;
}

uint16 remove_leading_bits(uint8 bitcount, uint16 x) {
	return (x & 0x8000) | ((x << bitcount) & 0x7FFF);
}

F16 atan_lookup(F15 y, F15 x) {
	F15 _x = x;
	F15 _y = y;
	
	uint8 xs = count_leading_unused_bits(x);
	uint8 ys = count_leading_unused_bits(y);
	
	uint8 s = (xs < ys) ? xs : ys;

	x = remove_leading_bits(s, x);	
	y = remove_leading_bits(s, y);	

	x += 0x8000;
	y += 0x8000;
	
	x = (x >> 12) & 0xF;
	y = (y >> 12) & 0xF;

	F16 retval = atan_lookup_table[y][x];
	return retval;
}

void __attribute__((__interrupt__, __shadow__, no_auto_psv)) _T3Interrupt(void) {
	td = F16dec(td);
		
	F15 input = (ADC1BUF0 - 0x0800) << 4;
	AD1CON1bits.SAMP = 1;

	sample_idx = (sample_idx + 1) & (DELAY_LENGTH - 1);
	sample[sample_idx] = input;

	if(td < 0) {
		F16 kfx = F16sub(td, kftau);
		F16 kfy = td;
		
		uint16 kx_idx = sample_idx + F16Toint(kfx);
		F15 kx_alpha = F16ToF15(kfx);
	
		uint16 ky_idx = sample_idx;
		F15 ky_alpha = F16ToF15(kfy);

		F15 x = F15add(
					F15mul(
						F15neg(kx_alpha), sample[(kx_idx - 1) & (DELAY_LENGTH - 1)]
						),
					F15mul(
						F15inc(kx_alpha), sample[kx_idx]
						)
					);
	
		F15 y = F15add(
					F15mul(
						F15neg(ky_alpha), sample[(ky_idx - 1) & (DELAY_LENGTH - 1)]
						),
					F15mul(
						F15inc(ky_alpha), sample[ky_idx & (DELAY_LENGTH - 1)]
						)
					);
	
		F16 e = atan_lookup(x, y);
		ed[ed_idx] = e ;

		if(ed_idx == 255) {
			ed_idx = 0;
		} else {
			ed_idx++;
		}
		
		if(e > 20000)
			currentSymbol = space;
		else if(e < 20000)
			currentSymbol = mark;
		else
			currentSymbol = none;
		
		F16 cd = F16unsafeMul(4, D1, e);
	
		td = F16add(td, F16sub(D0, cd));
		process = 1;
	}

	IFS0bits.T3IF = 0; // clear interrupt flag
}

//Timer control routines
void initBaudTimer() {
	PR1 = 0xFFFF;
	TMR1 = 0x0000;
	T1CONbits.TCKPS = 0b01;
	T1CONbits.TON = 1;
}

void restartBaudTimer() {
	TMR1 = 0x0000;
}

unsigned int getBaudTime() {
	unsigned int tmp = TMR1;
	TMR1 = 0x0000;
	return tmp;
}

void initSampleTimer() {
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

uint16 validateRTTY(uint16 character) {
	uint16 stopBits = character & 0x0003, startBit = (character & 0x0080) ^ 0x0080;
	
	if((startBit | stopBits) == 0x0083)
		return 1;
	else
		return 0;
}

void printRTTY(unsigned int character) {
	uint16 strippedCharacter = ((character >> 2) & 0x1F), i = 0;

	//buffer[buffer_x] = sent2;
	//buffer_x++;
	//if(buffer_x > 31) {
	//	character = RTTY_FIGURES_TO_HANTRONIX[sent];
	//}
	//comDisplay(0, HD_CMD_clear);
	switch(rtty_mode) {
		case RTTY_LTRS:
			character = RTTY_LETTERS_TO_HANTRONIX[strippedCharacter];
			break;
		case RTTY_FIGS:
			character = RTTY_FIGURES_TO_HANTRONIX[strippedCharacter];
			break;
	}

	/*switch(character) {
		case (RTTY_NULL):
			printDisplay(HD_space);
			break;
		case (RTTY_LF):
			break;
		case (RTTY_SP):
			printDisplay(HD_space);
			break;
		case (RTTY_CR):
			if(display_offset < 8) {
				display_offset = 8;
			} else {
				display_offset = 16;
			}
			break;
		case (RTTY_FIGS):
			rtty_mode = RTTY_FIGS;
			break;
		case (RTTY_LTRS):
			rtty_mode = RTTY_LTRS;
			break;
		case (RTTY_ENQ):
			printDisplay(HD_space);
			printDisplay(HD_low_e);
			printDisplay(HD_space);
			break;
		case (RTTY_BEL):
			printDisplay(HD_space);
			printDisplay(HD_low_b);
			printDisplay(HD_space);
			break;
		default:
			printDisplay(character);
			break;
	}*/
	//chars_sent++;
}


void delay32_2(unsigned long int delay) {
	unsigned long int x = 0;
	unsigned int z = 0;
	for(x = 0; x < delay; x++) {
		z++;
	}
}

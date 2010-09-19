#include "dsp.h"
#include "libq.h"

#include "limits.h"
#include "float.h"
#include "math.h"

#include "HD44780.h"

#define DELAY_LENGTH 16

//#define MARK_FREQUENCY 1275.0
//#define SPACE_FREQUENCY 1445.0

#define MCLK 29480000
#define SAMPLE_RATE 10236

//This just wraps around all the other intializiation functions
void init();

void initIO();
void initADC();

//Initialize all the RTTY decode junk
void initRTTY();

//Baud timer control functions
void initBaudTimer();
void restartBaudTimer();
unsigned int getBaudTime();

//Sample timer control functions
void initSampleTimer();

//RTTY decode utility functions
unsigned int validateRTTY(unsigned int character);
void printRTTY(unsigned int character);

void delay32_2(unsigned long int delay);

//signed int spacevar[FFT_BLOCK_LENGTH] __attribute__ ((space (xmemory), aligned (FFT_BLOCK_LENGTH * 2)));
unsigned short int sample_idx;
_Q15 sample[DELAY_LENGTH] __attribute__ ((space (xmemory)));

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

	RCONbits.SWDTEN = 0;
	init();
	IPC1 = 0xFFFF;

	//IEC0bits.SI2CIE = 1;
	//IEC0bits.ADIE = 1;
	IEC0bits.T3IE = 1;
	IFS0bits.T3IF = 0;

	//Decode loop
	while(1) {
	}
}

//Initialize everything
void init() {
	sample_idx = 0;

	initRTTY();

	initIO();
	initADC();
	initBaudTimer();
	initSampleTimer();
	initDisplay();
}

void initIO() {
	//Port B
	//First bit (from left) for analog in
	//Next bit nothing
	//Next 2 bits (from left) are input for rotary encoder
	//Last bits are outputs for display
	TRISB = 0b1111000000;
	TRISD = 0x0000;
}

void initADC() {
	initADC();
	ADPCFGbits.PCFG9 = 0;
	ADCON1 = 0x03E4;
	ADCON2 = 0x0008;
	ADCON3 = 0x000A;
	ADCHS = 0x0009;
	ADCON1bits.ADON = 1;
	ADCON1bits.ASAM = 1;
}

_Q16 td,
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

	D0 = _Q16ftoi(Fs * T0);
	D1 = _Q16ftoi(Fs * G1);
	kftau = _Q16ftoi(Fs * tau);
	td = kftau;
	negOne = _Q16ftoi(-1.0f);

	uint16 ix, iy;
	for(ix = 0; ix < 16; ix++) {
		for(iy = 0; iy < 16; iy++) {
			//float x = (ix & 0x8) ?  -1.0f * float(ix & 0x7) : float(ix & 0x7),
			//	y = (iy & 0x8) ?  -1.0f * float(iy & 0x7) : float(iy & 0x7),
			//	atan = 0.0f;

			//atan = atan2f(y, x);

			//atan_lookup_table[iy][ix] = _Q16ftoi(atan);
		}
	}
}

_Q15 _Q15addOne(_Q15 input) {
	return input ^ 0x8000;
}

_Q16 atan_lookup(_Q15 y, _Q15 x) {
	uint16 _x = x;
	uint16 _y = y;

	x >>= 12;
	y >>= 12;

	x &= 0xF;
	y &= 0xF;

	return atan_lookup_table[y][x];
}

int16 intPart(_Q16 x) {
	return (int16)(x >> 16 & 0xFFFF);
}

_Q15 fractPart(_Q16 x) {
	_Q15 tx = (x >> 1) & 0xFFFF;
	
	if(x & 0x80000000) // If x is negative
		tx |= 0x8000;
		
	return tx;
}

void __attribute__((__interrupt__, __shadow__, no_auto_psv)) _T3Interrupt(void) {
	td = _Q16add(td, negOne);

	_Q15 input = ADCBUF2;

	sample_idx = (sample_idx + 1) & (DELAY_LENGTH - 1);
	sample[sample_idx] = input;

	if(td > 0) return;

	_Q16 kf = td;
	_Q16 kfy = kf + kftau;

	uint16 kx_idx = sample_idx;
	_Q15 kx_alpha = kf;

	uint16 ky_idx = sample_idx + intPart(kfy);
	_Q15 ky_alpha = fractPart(kfy);

	_Q15 x = _Q15mul(_Q15neg(kx_alpha), sample[(kx_idx - 1) & (DELAY_LENGTH - 1)]) +
				_Q15mul(_Q15addOne(kx_alpha), sample[kx_idx]);

	_Q15 y = _Q15mul(_Q15neg(ky_alpha), sample[(ky_idx - 1) & (DELAY_LENGTH - 1)]) +
				_Q15mul(_Q15addOne(ky_alpha), sample[ky_idx & (DELAY_LENGTH - 1)]);

	_Q16 e = atan_lookup(x, y);
	_Q16 cd = _Q16mul(D1, e);

	td = _Q16add(td, _Q16sub(D0 - cd));

	IFS0bits.T3IF = 0; // clear interrupt flag
}

void __attribute__((__interrupt__, __shadow__, no_auto_psv)) _SI2CInterrupt(void) {
	if(I2CSTATbits.D_A == 1 && I2CSTATbits.RBF == 1) {
		//Do something with I2CRCV
	}

	IFS0bits.SI2CIF = 0;
}

//Timer control routines
void initBaudTimer() {
	T1CON = 0x8030;
	PR1 = 0xFFFF;
	TMR1 = 0x0000;
}

void restartBaudTimer() {
	TMR1 = 0x0000;
}

unsigned int getBaudTime() {
	return TMR1;
}

#define SAMPLE_DELAY MCK/SAMPLE_RATE
//We do the math manually, and get 720
//Timer 3 generates a special ADC signal, so we use it
void initSampleTimer() {
	T3CON = 0x8000;
	PR3 = 720;
	TMR3 = 0x0000;
}

unsigned int validateRTTY(unsigned int character) {
	unsigned int character_tmp = character & 0xC000;
	character &= 0xFF00;
	character ^= 0x0100;
	character &= 0x0100;
	character |= character_tmp;
	if(character == 0xC100) {
		return 1;
	} else {
		return 0;
	}
}

void printRTTY(unsigned int character) {
	unsigned int sent = character;
	unsigned int sent2 = character;
	sent >>= 9;
	sent &= 0x1F;

	//buffer[buffer_x] = sent2;
	//buffer_x++;
	//if(buffer_x > 31) {
	//	character = RTTY_FIGURES_TO_HANTRONIX[sent];
	//}
	//comDisplay(0, HD_CMD_clear);
	switch(rtty_mode) {
		case RTTY_LTRS:
			character = RTTY_LETTERS_TO_HANTRONIX[sent];
			break;
		case RTTY_FIGS:
			character = RTTY_FIGURES_TO_HANTRONIX[sent];
			break;
	}

	switch(character) {
		case (RTTY_NULL):
			printDisplay(HD_space);
			break;
		case (RTTY_LF):
			break;
		case (RTTY_SP):
			printDisplay(HD_space);
			break;
		case (RTTY_CR):
			/*if(display_offset < 8) {
				display_offset = 8;
			} else {
				display_offset = 16;
			}*/
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
	}
	//chars_sent++;
}


void delay32_2(unsigned long int delay) {
	unsigned long int x = 0;
	unsigned int z = 0;
	for(x = 0; x < delay; x++) {
		z++;
	}
}
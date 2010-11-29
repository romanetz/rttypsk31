#include "dsp.h"
#include "libq.h"

#include "limits.h"
#include "float.h"
#include "math.h"

#include "HD44780.h"
#include "util.h"
#include "fractionaltypes.h"
#include "frontend.h"

#define DELAY_LENGTH 64

//#define MARK_FREQUENCY 1275.0
//#define SPACE_FREQUENCY 1445.0

//_FOSCSEL(FNOSC_FRC);

#define SAMPLE_RATE 8000


//This just wraps around all the other intializiation functions
void init();

void initIO();
void initADC();

//Initialize all the demodulation junk
void initTDTL();

//Generic globals
uint16 process;
F16 td, oldTd;

//RTTY global communicator variables
#define rttyNone 2
#define rttyMark 1
#define rttySpace 0

//PSK31 global communicator variables
#define IE 1
#define psk0 0
#define psk1 1
#define pskAny 2
#define pskNone 3
F16 tmp[1024];

F16 atan_lookup(F15 yi, F15 xi);

//Baud timer control functions
void initBaudTimer();
void restartBaudTimer();
unsigned int getBaudTime();

//Sample timer control functions
void initSampleTimer();

//RTTY decode utility functions
uint16 validateRTTY(uint16 character);
void printRTTY(uint16 character);

//PSK decode utility functions
void printPSK(uint16 character);

void delay32_2(unsigned long int delay);

//signed int spacevar[FFT_BLOCK_LENGTH] __attribute__ ((space (xmemory), aligned (FFT_BLOCK_LENGTH * 2)));
unsigned short int sample_idx;
F15 sample[DELAY_LENGTH];// __attribute__ ((space (xmemory)));

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

uint16 PSK_TO_PSK_IDX[128] = {
0b1,
0b11,
0b101,
0b111,
0b1011,
0b1101,
0b1111,
0b10101,
0b10111,
0b11011,
0b11101,
0b11111,
0b101011,
0b101101,
0b101111,
0b110101,
0b110111,
0b111011,
0b111101,
0b111111,
0b1010101,
0b1010111,
0b1011011,
0b1011101,
0b1011111,
0b1101011,
0b1101101,
0b1101111,
0b1110101,
0b1110111,
0b1111011,
0b1111101,
0b1111111,
0b10101011,
0b10101101,
0b10101111,
0b10110101,
0b10110111,
0b10111011,
0b10111101,
0b10111111,
0b11010101,
0b11010111,
0b11011011,
0b11011101,
0b11011111,
0b11101011,
0b11101101,
0b11101111,
0b11110101,
0b11110111,
0b11111011,
0b11111101,
0b11111111,
0b101010101,
0b101010111,
0b101011011,
0b101011101,
0b101011111,
0b101101011,
0b101101101,
0b101101111,
0b101110101,
0b101110111,
0b101111011,
0b101111101,
0b101111111,
0b110101011,
0b110101101,
0b110101111,
0b110110101,
0b110110111,
0b110111011,
0b110111101,
0b110111111,
0b111010101,
0b111010111,
0b111011011,
0b111011101,
0b111011111,
0b111101011,
0b111101101,
0b111101111,
0b111110101,
0b111110111,
0b111111011,
0b111111101,
0b111111111,
0b1010101011,
0b1010101101,
0b1010101111,
0b1010110101,
0b1010110111,
0b1010111011,
0b1010111101,
0b1010111111,
0b1011010101,
0b1011010111,
0b1011011011,
0b1011011101,
0b1011011111,
0b1011101011,
0b1011101101,
0b1011101111,
0b1011110101,
0b1011110111,
0b1011111011,
0b1011111101,
0b1011111111,
0b1101010101,
0b1101010111,
0b1101011011,
0b1101011101,
0b1101011111,
0b1101101011,
0b1101101101,
0b1101101111,
0b1101110101,
0b1101110111,
0b1101111011,
0b1101111101,
0b1101111111,
0b1110101011,
0b1110101101,
0b1110101111,
0b1110110101,
0b1110110111,
0b1110111011
};

uint16 PSK_IDX_TO_HANTRONIX[128] = {
HD_space,
HD_low_e,
HD_low_t,
HD_low_o,
HD_low_a,
HD_low_i,
HD_low_n,
HD_low_r,
HD_low_s,
HD_low_l,
'\n',
'\n',
HD_low_h,
HD_low_d,
HD_low_c,
HD_subtract,
HD_low_u,
HD_low_m,
HD_low_f,
HD_low_p,
HD_equals,
HD_period,
HD_low_g,
HD_low_y,
HD_low_b,
HD_low_w,
HD_cap_T,
HD_cap_S,
HD_comma,
HD_cap_E,
HD_low_v,
HD_cap_A,
HD_cap_I,
HD_cap_O,
HD_cap_C,
HD_cap_R,
HD_cap_D,
HD_zero,
HD_cap_M,
HD_one,
HD_low_k,
HD_cap_P,
HD_cap_L,
HD_cap_F,
HD_cap_N,
HD_low_x,
HD_cap_B,
HD_two,
HD_space, //Tab
HD_colon,
HD_rightparenthesis,
HD_leftparenthesis,
HD_cap_G,
HD_three,
HD_cap_H,
HD_cap_U,
HD_five,
HD_cap_W,
HD_doublequote,
HD_six,
HD_underscore,
HD_multiply,
HD_cap_X,
HD_four,
HD_cap_Y,
HD_cap_K,
HD_apostrophe,
HD_eight,
HD_seven,
HD_divide, ///
HD_cap_V,
HD_nine,
HD_or,
HD_semicolon,
HD_low_q,
HD_low_z,
HD_greaterthan,
HD_dollar,
HD_cap_Q,
HD_add,
HD_low_j,
HD_lessthan,
HD_divide, //\
HD_pound,
HD_leftbracket,
HD_rightbracket,
HD_cap_J,
HD_exclamation,
HD_space,
HD_cap_Z,
HD_question,
HD_rightcurlybracket,
HD_leftcurlybracket,
HD_and,
HD_at,
HD_power,
HD_percent,
HD_accent, //~
HD_space,
HD_space,
HD_accent,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space,
HD_space
};

int main(void)
{
	unsigned int x = 0;
	
	RCONbits.SWDTEN = 0;
	
	//Demodulation constants
	F16 D1, kftau,
		e = 0, ie = 0;
	
	initIO();
	initADC();
	initBaudTimer();
	frontend_init();
	initDisplay();
	
	sample_idx = 0;
	
	initTDTL(&D1, &kftau);
	
	initSampleTimer(); // We start this timer last so that processing will only begin after all initialization is finished

	IPC1 = 0xFFFF;

	IEC0bits.T3IE = 1;
	IFS0bits.T3IF = 0;

	td = oldTd = 0;
	process = 0;
	
	//Decode loop
	uint16 doRTTY = 1, doPSK = 0;
	
	//Useful RTTY Constants
	const int16 rttySymbolTime = (uint16)((((float)Fosc / 2.0) / 8.0) / 44.45); // The timer is driven by Fosc / 2 through a 256 prescaler and we are looking for 45.45 baud symbols
	const int16 rttySwitchTime = rttySymbolTime / 4;
	
	//RTTY Decode state variables
	int32 rttyMarkTime = 0, rttySpaceTime = 0,
		rttyDecodeSymbol = rttyMark, rttyDam = 0;
		
	uint16 rttyCurrentSymbol = rttyNone, rttyProcessMark = 0, rttyProcessSpace = 0,
		rttyCharacter = 0, rttySymbolCount = 0;
		
	//Useful PSK Constants
	const int16 pskSymbolTime = (uint16)((((float)Fosc / 2.0) / 8.0) / 31.25); // The timer is driven by Fosc / 2 through a 256 prescaler and we are looking for 45.45 baud symbols
	const int16 pskSwitchTime = pskSymbolTime / 4;
	
	const F16 pi = floatToF16(3.14159265f), piErr = pi - floatToF16(3.14159265f / 4.0f);
	//PSK Decode state variables
	int32 pskDam = 0;
	
	uint16 pskWatch = psk1, pskSpace = 0, pskCharacter = 0, pskSymbolCount = 0;
	
	//Initialize phase detector DSP
	
	//High pass filter for PSK
	uint16 ed_idx = 0, idx = 0;
	float Fs, Ts, alpha_float, beta_float;
	Fs = (float)SAMPLE_RATE;
	Ts = (1.0f / Fs);
	
	alpha_float = 500.0f;
	beta_float = expf(-1.0f * alpha_float * Ts);
	
	F16 hp_one_minus_alpha, hp_beta;
	F16 e_hp, e_d1;
	
	hp_one_minus_alpha = floatToF16(1.0f - alpha_float * Ts);
	hp_beta = floatToF16(beta_float);

	//Integral for PSK
	const F16 F16pi = floatToF16(pi);
	F16 e_d[IE];
	
	//Low pass filter for RTTY	
	alpha_float = 1500.0f;
	beta_float = expf(-1.0f * alpha_float * Ts);
	
	F16 lp_alpha, lp_beta;
	F16 e_lp;
	
	lp_alpha = floatToF16(alpha_float * Ts);
	lp_beta = floatToF16(beta_float);
	
	restartBaudTimer();
	
	while(1) {
		if(process == 0)
			continue;
		
		F16 kfx = F16sub(oldTd, kftau);
		F16 kfy = oldTd;
		
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
	
		e = atan_lookup(x, y);
		
		/*tmp[idx] = e;
		
		if(idx >= 1023)
			idx = 0;
		else
			idx++;*/
			
		//Highpass filter for PSK
		e_hp = F16add(F16unsafeMul(4, hp_beta, F16sub(e_hp, e_d1)), F16unsafeMul(4, hp_one_minus_alpha, e));
		
		//Lowpass filter for RTTY
		e_lp = F16add(F16unsafeMul(4, lp_beta, e_lp), F16unsafeMul(4, lp_alpha, e));
		
		e_d1 = e;
		
		tmp[idx] = e_lp;
		
		if(idx >= 1023)
			idx = 0;
		else
			idx++;
		
		F16 cd = F16unsafeMul(4, D1, e);
	
		//Disable global interrupts on this update
		IEC0bits.T3IE = 0;
		td = F16add(td, F16neg(cd));
		IEC0bits.T3IE = 1;
			
		uint16 Telaps = getBaudTime();
		
		if(doRTTY) {
			if(e_lp <= 0)
				rttyCurrentSymbol = rttyMark;
			else if(e_lp > 0)
				rttyCurrentSymbol = rttySpace;
			else
				rttyCurrentSymbol = rttyNone;
			
			//Watch for a symbol switch
			//  If an unexpected signal detection change occurs, track it to see if real
			if(rttyCurrentSymbol == rttyDecodeSymbol) {
				//If it turns out to be fake, do nothing (because time added to right bin anyway)
				rttyDam = 0;
			} else {
				//If it turns out to be real, evaluate other timer and switch decode symbol 
				rttyDam += Telaps;
				
				if(rttyDam > rttySwitchTime) {
					//If need to switch symbols, evaluate the old decodeSymbol before updating to new
					switch(rttyDecodeSymbol) {
						case rttyMark:
							if(rttyCurrentSymbol == rttySpace) rttySpaceTime = rttyDam - Telaps; //The '- Telaps' term avoids double adding
							rttyMarkTime -= rttyDam - Telaps;
							rttyProcessMark = 1;
							break;
						case rttySpace:
							if(rttyCurrentSymbol == rttyMark) rttyMarkTime = rttyDam - Telaps;
							rttySpaceTime -= rttyDam - Telaps;
							rttyProcessSpace = 1;
							break;
					}
	
					//Update to new symbol
					rttyDecodeSymbol = rttyCurrentSymbol;
	
					//Reset the dam
					rttyDam = 0;
				}	
			}
			
			
			switch(rttyDecodeSymbol) {
				case rttyMark:
					rttyMarkTime += Telaps;
					break;
				case rttySpace:
					rttySpaceTime += Telaps;
					break;
			}
			
			if(rttyProcessMark) {
				while(rttyMarkTime > (rttySymbolTime >> 1)) {
	 				rttyCharacter = (rttyCharacter << 1) | 0x0001;
	 					
					rttyMarkTime -= rttySymbolTime;
					rttySymbolCount++;
				}
				
				rttyMarkTime = 0;
				rttyProcessMark = 0;
			}
			
			if(rttyProcessSpace) {
				while(rttySpaceTime > (rttySymbolTime >> 1)) {
	 				rttyCharacter = (rttyCharacter << 1) & 0xFFFE;
	 					
					rttySpaceTime -= rttySymbolTime;
					rttySymbolCount++;
				}
				
				rttySpaceTime = 0;
				rttyProcessSpace = 0;
			}
			
			while(rttySymbolCount >= 8) {
				uint16 overshoot = rttySymbolCount - 8;
				uint16 adjustedCharacter = rttyCharacter >> overshoot;
				
				if(validateRTTY(adjustedCharacter)) {
					adjustedCharacter = bitrev((adjustedCharacter >> 2) & 0x1F, 5);
					
					rttySymbolCount -= 8;
					
					printRTTY(adjustedCharacter);
				} else
					rttySymbolCount--;
			}
			
			process = 0;
		} else if(doPSK) {
			F16 tie = (ie < 0) ? (ie ^ 0xFFFFFFFF) + 1 : ie;
		
			pskDam += Telaps;

			if(pskDam >= pskSwitchTime && tie >= piErr) {
				if(pskWatch == pskAny | pskWatch == psk0) {
					pskCharacter = (pskCharacter << 1) & 0xFFFE;
					
					//pskDam -= pskSymbolTime;
					pskDam = 0;
						
					pskWatch = psk1;
					pskSymbolCount++;
				} else {
					pskCharacter >>= 1;
					
					if(pskSymbolCount > 0) {
						printPSK(pskCharacter);
						if(pskCharacter != 0b1)
							pskSpace = 0;
					} else if(!pskSpace) {
						printPSK(0b1);
						pskSpace = 1;
					}	
					
					pskDam = 0;
					pskCharacter = 0;
					pskWatch = psk1;
					pskSymbolCount = 0;
				}	
			}

			if(pskDam >= (pskSymbolTime + pskSwitchTime)) {
				pskCharacter = (pskCharacter << 1) | 0x1;

				//if(pskSymbolCount > 0)
				//	pskDam -= pskSymbolTime;
				//else
					pskDam = 0;

				pskWatch = pskAny;
				pskSymbolCount++;
			}
			
			process = 0;
		}
	}
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

F16 D0,
	F16pi1By2, F16pi,
	atanLookupTable[64];

void initTDTL(F16 *D1, F16 *kftau) {
	
	const float pi = 3.14159265f;

	float Fs, Ts,
		psi,
		tau,
		Fmark, Fspace,
		F0, W0, T0,
		K1, G1;

	psi = pi / 3.0f;
	
	F16pi1By2 = floatToF16(pi / 2.0f);
	F16pi = floatToF16(pi);

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
	*D1 = floatToF15(Fs * G1);
	*kftau = floatToF16(Fs * tau);
	td = *kftau;

	int i;
	for(i = 0; i < 64; i++)
		atanLookupTable[i] = floatToF16(atanf(((float)i) / 4.0f));
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

uint8 count_trailing_unused_bits(uint16 x) {
	uint8 i = 0, j = 0;
	
	uint8 sign = (x >> 15) & 0x1;
	
	if(sign == 0x1)
		x != x;
	
	while((x & 0x0001) == 0 && i < 16) {
		i++;
		x >>= 1;
	}
	
	return i;
}

uint16 remove_leading_bits(uint8 bitcount, uint16 x) {
	return (x & 0x8000) | ((x << bitcount) & 0x7FFF);
}

F16 atan_lookup(F15 yi, F15 xi) {
	F15 _x, _y;
	
	uint8 xsign, ysign;
	
	//Handle the zeroes here
	if(yi == 0)
		if(xi >= 0)
			return F16pi;
		else
			return F16neg(F16pi);
	
	if(xi == 0)
		if(yi > 0)
			return F16pi1By2;
		else
			return F16neg(F16pi1By2);
	
	_x = (xi < 0) ? F15neg(xi) : xi;
	_y = (yi < 0) ? F15neg(yi) : yi;
	
	F16 val = 0, valp = 0;
	
	uint16 i = _y / _x;
	uint16 r = _y % _x;
	
	uint8 us = count_leading_unused_bits(r);
	uint8 ls = count_leading_unused_bits(_x);
	
	_x >>= (15 - ls - 7);
	
	uint16 c = 0xFFFF;
	
	if(_x != 0)
		c = (r << us) / _x;
		
	c = (c << (15 - (us + (15 - ls - 7)))) - 1;
	
	F15 alpha = c, alphap;
	
	i = (i << 2) | ((alpha & 0x6000) >> 13);
	alpha = (alpha << 2) & 0x7FFF;
	
	alphap = F15neg(F15inc(alpha));
	
	if(i >= 64) {
		val = F16pi1By2;
	} else {
		val = atanLookupTable[i];
		
		if(i + 1 < 64)
			valp = atanLookupTable[i + 1];
		else
			valp = F16pi1By2;
			
		val = F16add(F16unsafeMul(2, val, F15ToF16(alphap)), F16unsafeMul(2, valp, F15ToF16(alpha)));
	}
	
	if(xi > 0 && yi > 0)
		return val;
	else if(xi > 0 && yi < 0)
		return F16neg(val);
	else if(xi < 0 && yi > 0)
		return F16add(val, F16pi1By2);
	else if(xi < 0 && yi < 0)
		return F16neg(F16add(val, F16pi1By2));
}

uint16 idx;

void __attribute__((__interrupt__, __shadow__, no_auto_psv)) _T3Interrupt(void) {
	td = F16dec(td);
		
	F15 input = (ADC1BUF0 - 0x0800) << 4;

	/*tmp[idx] = input;
	
	if(idx >= 1023)
		idx = 0;
	else
		idx++;*/
			
	input = frontend_filter(input);
	
	/*if(idx >= 1023)
		idx = 0;
	else
		idx++;
		
	tmp[idx] = input;*/
			
	AD1CON1bits.SAMP = 1;

	sample_idx = (sample_idx + 1) & (DELAY_LENGTH - 1);
	sample[sample_idx] = input;

	if(td < 0) {
		oldTd = td;
		td = F16add(td, D0);
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

void printRTTY(uint16 character) {
	switch(rtty_mode) {
		case RTTY_LTRS:
			character = RTTY_LETTERS_TO_HANTRONIX[character];
			break;
		case RTTY_FIGS:
			character = RTTY_FIGURES_TO_HANTRONIX[character];
			break;
	}

	switch(character) {
		case (RTTY_NULL):
			character <<= 1;
			printDisplay(HD_space);
			break;
		case (RTTY_LF):
			break;
		case (RTTY_SP):
			printDisplay(HD_space);
			break;
		case (RTTY_CR):
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

void printPSK(uint16 character) { 
	int16 idxOffset = 128 / 2;
	uint16 idx = idxOffset;
	
	//character = bitrev(character, 16 - count_leading_unused_bits(character));
	
	while(idxOffset > 0) {
		
		if(idx > 128)
			return;
			
		uint16 val = PSK_TO_PSK_IDX[idx];
		
		idxOffset >>= 1;
		
		if(val > character)
			idx -= idxOffset;
		else if(val < character)
			idx += idxOffset;
		else
			break;
			
		if(idxOffset == 0) {
			val = PSK_TO_PSK_IDX[idx];

			if(val > character && idx == 1) {
				idx = 0;
				break;
			}

			return;
		}	
	}
	
	printDisplay(PSK_IDX_TO_HANTRONIX[idx]);
}


void delay32_2(unsigned long int delay) {
	unsigned long int x = 0;
	unsigned int z = 0;
	for(x = 0; x < delay; x++) {
		z++;
	}
}

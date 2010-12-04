#include "rtty.h"
#include "baudtimer.h"
#include "HD44780.h"
#include "math.h"

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

uint16 validateRTTY(uint16 character);
void printRTTY(uint16 character);

//RTTY global communicator variables
#define rttyNone 2
#define rttyMark 1
#define rttySpace 0

unsigned int rtty_mode = RTTY_LTRS;

//Useful RTTY Constants
int16 rttySymbolTime;
int16 rttySwitchTime;
	
//RTTY Decode state variables
int32 rttyMarkTime, rttySpaceTime,
	rttyDecodeSymbol, rttyDam;
		
uint16 rttyCurrentSymbol, rttyProcessMark, rttyProcessSpace,
	rttyCharacter, rttySymbolCount;

F16 lp_alpha, lp_beta;
F16 e_lp;

F16 tmp[512];
uint16 idxtmp;

void rtty_init() {
	rttySymbolTime = (uint16)((((float)Fosc / 2.0) / 8.0) / 45.45); // The timer is driven by Fosc / 2 through a 256 prescaler and we are looking for 45.45 baud symbols
	rttySwitchTime = rttySymbolTime / 4;
	
	rttyMarkTime = 0;
	rttySpaceTime = 0;
	rttyDecodeSymbol = rttyMark;
	rttyDam = 0;
	
	rttyCurrentSymbol = rttyNone;
	rttyProcessMark = 0;
	rttyProcessSpace = 0;
	rttyCharacter = 0;
	rttySymbolCount = 0;
	
	//Initialize phase detector DSP
	//Low pass filter for RTTY
	float alpha_float = 1500.0f;
	float beta_float = expf(-1.0f * alpha_float * 1.0f / ((float)SAMPLE_RATE));
	
	lp_alpha = floatToF16(alpha_float * 1.0f / ((float)SAMPLE_RATE));
	lp_beta = floatToF16(beta_float);
	idxtmp = 0;
}

void rtty_process(F16 e) {
	uint16 Telaps = baud_time_get();
	
	e_lp = F16add(F16unsafeMul(4, lp_beta, e_lp), F16unsafeMul(4, lp_alpha, e));
	
		tmp[idxtmp] = e;
		if(idxtmp > 511)
			idxtmp = 0;
		else
			idxtmp++;
		
	if(e <= 0)
		rttyCurrentSymbol = rttyMark;
	else if(e > 0)
		rttyCurrentSymbol = rttySpace;
			
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
			
			//printRTTY(adjustedCharacter);
		} else
			rttySymbolCount--;
	}			
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
}

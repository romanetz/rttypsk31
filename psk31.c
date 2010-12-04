#include "psk31.h"
#include "baudtimer.h"
#include "HD44780.h"

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

void psk31_print(uint16 character);

//PSK31 global communicator variables
#define psk0 0
#define psk1 1
#define pskAny 2
#define pskNone 3

//Integral for PSK
#define IE 4
F16 e_d[IE];
uint16 e_d_idx;

F16 ie;

//Useful PSK Constants
int16 pskSymbolTime;
int16 pskSwitchTime;
	
F16 pi, piErr;

//PSK Decode state variables
int32 pskDam;
uint16 pskWatch, pskSpace, pskCharacter, pskSymbolCount;
	
void psk31_init() {
	pskSymbolTime = (uint16)((((float)Fosc / 2.0) / 8.0) / 31.25); // The timer is driven by Fosc / 2 through a 256 prescaler and we are looking for 45.45 baud symbols
	pskSwitchTime = pskSymbolTime / 4;

	pi = floatToF16(3.14159265f);
	piErr = pi - floatToF16(3.14159265f / 4.0f);
	
	pskDam = 0;
	pskWatch = psk1;
	pskSpace = 0;
	pskCharacter = 0;
	pskSymbolCount = 0;
	
	ie = 0;
	e_d_idx = 0;
	
	int i;
	
	for(i = 0; i < IE; i++)
		e_d[i] = 0;
}
	
void psk31_process(F16 e) {
	uint16 Telaps = baud_time_get();
	
	ie += e - e_d[e_d_idx];
	e_d[e_d_idx] = e;
	e_d_idx = e_d_idx & (IE - 1);
	
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
				//psk31_print(pskCharacter);
				if(pskCharacter != 0b1)
					pskSpace = 0;
			} else if(!pskSpace) {
				psk31_print(0b1);
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
}

void psk31_print(uint16 character) { 
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

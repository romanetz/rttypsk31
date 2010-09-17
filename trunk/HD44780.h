#include "util.h"

#define HD_CMD_clear		0b00000001
#define HD_CMD_return		0b00000010
#define HD_CMD_mode			0b00000110
#define HD_CMD_display		0b00001111
#define HD_CMD_shift		0b00010100
#define HD_CMD_function		0b00101000
#define HD_CMD_CGaddress	0b01000000
#define HD_CMD_DDaddress	0b10000000
#define HD_CMD_busy			0b00000000
#define HD_CMD_datawrite	0b00000000
#define HD_CMD_dataread		0b00000000

#define HD_CMD_mode_OPT_shiftright 0b00000010
#define HD_CMD_display_OPT_on 0b0000100
#define HD_CMD_display_OPT_cursor 0b0000010
#define HD_CMD_display_OPT_blink 0b0000001
#define HD_CMD_shift_OPT_displayshift 0b0001000
#define HD_CMD_shift_OPT_shiftright 0b0000100
#define HD_CMD_function_OPT_8bit 0b0000100
#define HD_CMD_function_OPT_twoline 0b0000010

#define HD_space 0b00100000
#define HD_exclamation 0b00100001
#define HD_doublequote 0b00100010
#define HD_pound 0b00100011
#define HD_dollar 0b00100100
#define HD_percent 0b00100101
#define HD_and 0b00100110
#define HD_apostrophe 0b00100111
#define HD_leftparenthesis 0b00101000
#define HD_rightparenthesis 0b00101001
#define HD_multiply 0b00101010
#define HD_add 0b00101011
#define HD_comma 0b00101100
#define HD_subtract 0b00101101
#define HD_period 0b00101110
#define HD_divide 0b00101111

#define HD_zero 0b00110000
#define HD_one 0b00110001
#define HD_two 0b00110010
#define HD_three 0b00110011
#define HD_four 0b00110100
#define HD_five 0b00110101
#define HD_six 0b00110110
#define HD_seven 0b00110111
#define HD_eight 0b00111000
#define HD_nine 0b00111001
#define HD_colon 0b00111010
#define HD_semicolon 0b00111011
#define HD_lessthan 0b00111100
#define HD_equals 0b00111101
#define HD_greaterthan 0b00111110
#define HD_question 0b00111111

#define HD_at 0b01000000
#define HD_cap_A 0b01000001
#define HD_cap_B 0b01000010
#define HD_cap_C 0b01000011
#define HD_cap_D 0b01000100
#define HD_cap_E 0b01000101
#define HD_cap_F 0b01000110
#define HD_cap_G 0b01000111
#define HD_cap_H 0b01001000
#define HD_cap_I 0b01001001
#define HD_cap_J 0b01001010
#define HD_cap_K 0b01001011
#define HD_cap_L 0b01001100
#define HD_cap_M 0b01001101
#define HD_cap_N 0b01001110
#define HD_cap_O 0b01001111

#define HD_cap_P 0b01010000
#define HD_cap_Q 0b01010001
#define HD_cap_R 0b01010010
#define HD_cap_S 0b01010011
#define HD_cap_T 0b01010100
#define HD_cap_U 0b01010101
#define HD_cap_V 0b01010110
#define HD_cap_W 0b01010111
#define HD_cap_X 0b01011000
#define HD_cap_Y 0b01011001
#define HD_cap_Z 0b01011010
#define HD_leftbracket 0b01011011
#define HD_yen 0b01011100
#define HD_rightbracket 0b01011101
#define HD_power 0b01011110
#define HD_underscore 0b01011111

#define HD_accent 0b01100000
#define HD_low_a 0b01100001
#define HD_low_b 0b01100010
#define HD_low_c 0b01100011
#define HD_low_d 0b01100100
#define HD_low_e 0b01100101
#define HD_low_f 0b01100110
#define	HD_low_g	0b01100111
#define HD_low_h 0b01101000
#define HD_low_i 0b01101001
#define HD_low_j 0b01101010
#define HD_low_k 0b01101011
#define HD_low_l 0b01101100
#define HD_low_m 0b01101101
#define HD_low_n 0b01101110
#define HD_low_o 0b01101111

#define HD_low_p 0b01110000
#define HD_low_q 0b01110001
#define HD_low_r 0b01110010
#define HD_low_s 0b01110011
#define HD_low_t 0b01110100
#define HD_low_u 0b01110101
#define HD_low_v 0b01110110
#define HD_low_w 0b01110111
#define HD_low_x 0b01111000
#define HD_low_y 0b01111001
#define HD_low_z 0b01111010
#define HD_leftcurlybracket 0b01111011
#define HD_or 0b01111100
#define HD_rightcurlybracket 0b01111101
#define HD_rightarrow 0b01111110
#define HD_leftarrow 0b01111111

void initDisplay();
void comDisplay(unsigned short int rwrs, unsigned short int val);

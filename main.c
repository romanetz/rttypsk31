#include "dsp.h"
#include "libq.h"

#include "limits.h"
#include "float.h"

#include "HD44780.h"
#include "util.h"
#include "fractionaltypes.h"
#include "baudtimer.h"
#include "frontend.h"
#include "arctanapprox.h"


#define DELAY_LENGTH 64

//Initialize all the demodulation junk
void tdtl_init();

//Generic globals
uint16 process;
F16 td, oldTd;

unsigned short int sample_idx;
F15 sample[DELAY_LENGTH];

F16 D0, D1, kftau;

int main(void) {
	unsigned int x = 0;
	
	RCONbits.SWDTEN = 0;
	
	//Demodulation constants
	F16	e = 0, ie = 0;
	uint16 doRTTY = 1, doPSK = 0;
	
	//Global intialization
	td = oldTd = 0;
	process = 0;
	sample_idx = 0;
	
	//Order of some of these inits matters
	io_init();
	adc_init();
	baud_timer_init();
	atan_init();
	psk31_init();
	rtty_init();
	frontend_init();
	//display_init();
	tdtl_init();
	sample_timer_init(); // We start this timer last so that processing will only begin after all initialization is finished

	//Set up interrupts
	IPC1 = 0xFFFF;
	IEC0bits.T3IE = 1;
	IFS0bits.T3IF = 0;
	

	//Get rid of 'junk' symbol time
	baud_timer_restart();
	
	while(1) {
		if(process == 0)
			continue;
		
		F16 kfx = F16sub(oldTd, kftau);
		F16 kfy = oldTd;
		
		uint16 kx_idx = sample_idx + F16Toint(kfx);
		F15 kx_alpha = F16ToF15(kfx);
	
		uint16 ky_idx = sample_idx;
		F15 ky_alpha = F16ToF15(kfy);

		F15 xt = F15add(
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
	
		e = atan_lookup(y, xt);
		
		F16 cd = F16unsafeMul(4, D1, e);
	
		//Disable global interrupts on this update
		IEC0bits.T3IE = 0;
		td = F16add(td, F16neg(cd));
		IEC0bits.T3IE = 1;
		
		if(doRTTY) {
			rtty_process(e);
			process = 0;
		} else if(doPSK) {
			process = 1;
			psk31_process(e);		
			process = 0;
		}
	}
}


void tdtl_init() {
	const float pi = 3.14159265f;

	float Fs, Ts,
		psi,
		tau,
		Fmark, Fspace,
		F0, W0, T0,
		K1, G1;

	psi = pi / 3.0f;

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
}

void __attribute__((__interrupt__, __shadow__, no_auto_psv)) _T3Interrupt(void) {
	td = F16dec(td);
		
	F15 input = (ADC1BUF0 - 0x0800) << 4;
	input = frontend_filter(input);
	
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

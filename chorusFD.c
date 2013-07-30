/**
 ******************************************************************************
 * File Name          : chorusFD.c
 * Author			  : Xavier Halgand (thanks to Gabriel Rivas)
 * Date               :
 * Description        : stereo chorus/flanger
 ******************************************************************************
 */
#include "chorusFD.h"
#include <math.h>
#include "sinetable.h"
#include  <stdint.h>
#include "CONSTANTS.h"

/* Private define ------------------------------------------------------------*/
#define MARGIN			6 // minimal delay (in samples)


/*****************************************************************************
 *                       chorus/flanger diagram (one channel) :
 *
 *                    ---------[mix >----------------------------
 *                    |                                         |
 *                    |                                         |
 *                    |x1                                       v
 *     xin ------>[+]----->[z^-M]--[interp.]----[fw >--------->[+]-----> yout
 *                 ^         delay line      |
 *                 |                         |
 *                 --< fb]<-------------------
 *
 *******************************************************************************/


static Lfo_t			lfoL, lfoR ; // 2 independant LFOs
static chorus_t 		delR, delL ; // 2 fractional delay lines

/*-------------------------------------------------------------------------------------------*/
float Lfo_SampleCompute(Lfo_t * op) // ! returns a positive value between 0 and op.amp !
{
	float z;

	op->phase += _2PI * Ts * op->freq; // increment phase
	while (op->phase < 0) // keep phase in [0, 2pi]
		op->phase += _2PI;
	while (op->phase >= _2PI)
		op->phase -= _2PI;

	z = sinetable[lrintf(ALPHA * (op->phase))];
	op->out = op->amp * (z + 1);

	return op->out;
}

/*---------------------------------------------------------------------------------------------*/
void chorusFD_init(void)
{
	chorusDelay_init(&delL, LEFT_DELAY, FEEDBACK, FORWARD, MIX);
	chorusDelay_init(&delR, RIGHT_DELAY, FEEDBACK, FORWARD, MIX);
	lfoL.amp = LEFT_SWEEP;
	lfoR.amp = RIGHT_SWEEP;
	lfoL.freq = LEFT_RATE ;
	lfoR.freq = RIGHT_RATE ;
	lfoL.phase = _PI/2; // initial phases for quadrature
	lfoR.phase = 0;
}

/*---------------------------------------------------------------------------------------------*/
void chorusDelay_init(chorus_t *del, float delay, float dfb, float dfw, float dmix)
{
	Delay_set_fb(del, dfb);
	Delay_set_fw(del, dfw);
	Delay_set_mix(del, dmix);
	Delay_set_delay(del, delay);
	del->in_idx = DEPTH - 1;// Place the input pointer at the end of the buffer
	del->mode = 1;
}

/*-------------------------------------------------------------------------------------------*/
void inc_chorusRate(void)
{
	lfoL.freq *= 1.05f;
	lfoR.freq *= 1.05f;
}
/*-------------------------------------------------------------------------------------------*/
void dec_chorusRate(void)
{
	lfoL.freq *= .95f;
	lfoR.freq *= .95f;
}
/*-------------------------------------------------------------------------------------------*/
void inc_chorusDelay(void)
{
	float d;

	d = delL.baseDelay * 1.1f;
	if (d < DEPTH)
	{
		delL.baseDelay = d;
	}

	d = delR.baseDelay * 1.1f;
	if (d < DEPTH)
	{
		delR.baseDelay = d;
	}
}
/*-------------------------------------------------------------------------------------------*/
void dec_chorusDelay(void)
{
	delL.baseDelay *= .9f;
	delR.baseDelay *= .9f;
}
/*-------------------------------------------------------------------------------------------*/
void inc_chorusFeedback(void)
{
	/* increase feedback delay */

	delL.fb *= 1.02f ;//
	delR.fb *= 1.02f ;//
}
/*-------------------------------------------------------------------------------------------*/
void dec_chorusFeedback(void)
{
	/* decrease feedback delay */

	delL.fb *= 0.95f ;//
	delR.fb *= 0.95f ;//
}
/*-------------------------------------------------------------------------------------------*/
void inc_chorusSweep(void)
{
	lfoL.amp *= 1.05f ;//
	lfoR.amp *= 1.05f ;//
}
/*-------------------------------------------------------------------------------------------*/
void dec_chorusSweep(void)
{
	lfoL.amp *= .95f ;//
	lfoR.amp *= .95f ;//
}
/*-------------------------------------------------------------------------------------------*/
void toggleChorusMode(void)
{
	delL.mode *= -1 ;//
	delR.mode *= -1 ;//
}
/*-------------------------------------------------------------------------------------------*/
void changeChorusFDBsign(void)
{
	delL.fb *= -1.f ;//
	delR.fb *= -1.f ;//
}
/*-------------------------------------------------------------------------------------------*/
void Delay_set_delay(chorus_t *del, float delay)
{
	del->baseDelay = delay;
}
/*-------------------------------------------------------------------------------------------*/
void Delay_set_fb(chorus_t *del, float val)
{
	del->fb = val;
}
/*-------------------------------------------------------------------------------------------*/
void Delay_set_fw(chorus_t *del, float val)
{
	del->fw = val;
}
/*-------------------------------------------------------------------------------------------*/
void Delay_set_mix(chorus_t *del, float val)
{
	del->mix = val;
}
/*-------------------------------------------------------------------------------------------*/
float Delay_get_fb(chorus_t *del )
{
	return del->fb;
}
/*-------------------------------------------------------------------------------------------*/
float Delay_get_fw(chorus_t *del )
{
	return del->fw;
}
/*-------------------------------------------------------------------------------------------*/
float Delay_get_mix(chorus_t *del )
{
	return del->mix;
}

/*-------------------------------------------------------------------------------------------*/
void delay_write (chorus_t *del, float xin)
{
	del->dline[del->in_idx] = xin;
	if (del->in_idx >= DEPTH-1)
		del->in_idx = 0;
	else (del->in_idx)++;
}
/*-------------------------------------------------------------------------------------------*/
float delay_read (chorus_t *del, float delay) // "delay" is a floating point number of samples
{
	float d;		// true delay
	float f;		// fractional part of delay
	int32_t i;		// integer part of delay
	float y_n;		// y(n)
	float y_n_1;	// y(n-1)
	float y_n_2;	// y(n-2)
	float y_n_3;	// y(n-3)
	int32_t idx;

	d = delay;
	if (d < MARGIN) d = MARGIN;
	if (d > DEPTH-MARGIN) d = DEPTH-MARGIN;

	i = (int32_t)floorf(d);
	f = d - i;

	idx = del->in_idx - i;
	if (idx < 0) idx += DEPTH;
	y_n = del->dline[idx]; 		// y(n)

	idx--;
	if (idx < 0) idx += DEPTH;
	y_n_1 = del->dline[idx];	// y(n-1)

	idx--;
	if (idx < 0) idx += DEPTH;
	y_n_2 = del->dline[idx];	// y(n-2)

	idx--;
	if (idx < 0) idx += DEPTH;
	y_n_3 = del->dline[idx];	// y(n-3)

	//return (y_n_1 - y_n) * f + y_n ; // linear interpolation

	//return (.5f)*(f-1)*(f-2)*y_n - f*(f-2)*y_n_1 + (.5f)*f*(f-1)*y_n_2 ; // 2nd order Lagrange interpolation

	//return .5f*(f-1)*((f-2)*y_n + f*y_n_2) - f*(f-2)*y_n_1 ;	// 2nd order Lagrange interpolation (faster)

	/* 3rd order Lagrange interpolation :  */
	return (f-2)*(f-3)*(-0.16666666666f *(f-1)*y_n + 0.5f * f * y_n_1) + f*(f-1)*(-0.5f * (f-3)*y_n_2 + 0.166666666666f * (f-2)*y_n_3);

}

/*---------------------------------------------------------------------------------------------*/
/*
This is the main mono chorus task,
 */

float mono_chorus_compute(chorus_t *del, Lfo_t *lfo, float xin)
{
	float yout;
	float x1;
	float x2;

	x2 = delay_read (del, del->baseDelay + Lfo_SampleCompute(lfo) + MARGIN);

	if (del->mode == 1)
		x1 = xin + del->fb * x2; // variable delay feedback signal or ...
	else
		x1 = xin + del->fb * delay_read (del, del->baseDelay + MARGIN); // fixed delay feedback signal

	yout = del->mix * x1 + del->fw * x2;
	delay_write(del, x1);

	return yout;
}

/*--------------------This is the main stereo chorus function : ----------------------------*/

void stereoChorus_compute (float * left_out, float * right_out, float in)
{
	*left_out = mono_chorus_compute(&delL, &lfoL, in);
	*right_out = mono_chorus_compute(&delR, &lfoR, in);
}


/*------------------------------------------END--------------------------------------------*/


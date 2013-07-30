/**
 ******************************************************************************
 * File Name          : chorusFD.h
 * Author             : Xavier Halgand (thanks to Gabriel Rivas)
 * Date               :
 * Description        :
 ******************************************************************************
 */
#ifndef __CHORUSFD_H__
#define __CHORUSFD_H__

#include  <stdint.h>

/*--------------------------------------------------------------------------------------*/

#define DEPTH	 		1400 // Size of delay buffer, in samples : 29.17 ms
#define LEFT_DELAY 		240 /*  initial left delay */
#define RIGHT_DELAY 	240 /*  initial right delay */
#define LEFT_SWEEP		50
#define RIGHT_SWEEP		50
#define LEFT_RATE		0.11f // in Hz
#define RIGHT_RATE		0.12f // in Hz
#define FEEDBACK		-0.2f // look at the diagram
#define FORWARD			0.5f
#define MIX				0.5f


/*---------------------------------------------------------------------------------------*/
typedef struct
{
	float 	amp;
	float 	freq;
	float 	phase;
	float 	out;

} Lfo_t;

typedef struct
{
	float		mix;       		/* delay blend parameter */
	float		fb;       		/* feedback volume */
	float		fw;       		/* delay tap mix volume */
	int32_t		in_idx;    		/* delay write index */
	float		dline[DEPTH] ;	/* delay buffer */
	float		baseDelay;		/* tap position */
	int8_t		mode; 			/* constant or variable delayed feedback ? */

} chorus_t ;

/*---------------------------------------------------------------------------------------------*/

void chorusFD_init(void);
void chorusDelay_init(chorus_t *del, float delay_samples,float dfb,float dfw, float dmix);
void stereoChorus_compute (float * left_out, float * right_out, float in);
void inc_chorusFeedback(void);
void dec_chorusFeedback(void);
void inc_chorusRate(void);
void dec_chorusRate(void);
void inc_chorusDelay(void);
void dec_chorusDelay(void);
void inc_chorusSweep(void);
void dec_chorusSweep(void);
void toggleChorusMode(void);
void changeChorusFDBsign(void);
void Delay_set_fb(chorus_t *del, float val);
void Delay_set_fw(chorus_t *del, float val);
void Delay_set_mix(chorus_t *del, float val);
void Delay_set_delay(chorus_t *del, float n_delay);
float Delay_get_fb(chorus_t *del);
float Delay_get_fw(chorus_t *del);
float Delay_get_mix(chorus_t *del);
float mono_chorus_compute(chorus_t *del, Lfo_t *lfo, float xin);

/*****************************************************************************************************/
#endif

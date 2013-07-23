/**
 ******************************************************************************
 * File Name		: audio.c
 * Date				:
 * Author			: Xavier Halgand
 * Description		:
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/

#include "audio.h"


uint16_t 			audiobuff[BUFF_LEN]; // THE audio buffer
static uint8_t		volume = VOL;
static bool			sound = true;


void audio_init(void)
{
	EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, VOL, SAMPLERATE);
	EVAL_AUDIO_Play((uint16_t*) audiobuff, BUFF_LEN); // start sound
}

//---------------------------------------------------------------------------
/**
 * @brief  Basic management of the timeout situation.
 * @param  None
 * @retval None
 */
uint32_t
Codec_TIMEOUT_UserCallback(void)
{
	//STM_EVAL_LEDOn(LED5); /*  alert : red LED !  */
	return (0);
}
//---------------------------------------------------------------------------
/**
 * @brief  Manages the DMA Half Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void
EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{
	/* Generally this interrupt routine is used to load the buffer when
   a streaming scheme is used: When first Half buffer is already transferred load
   the new data to the first half of buffer while DMA is transferring data from
   the second half. And when Transfer complete occurs, load the second half of
   the buffer while the DMA is transferring from the first half ... */

	STM_EVAL_LEDOff(LED_Orange);
	make_sound(0, BUFF_LEN_DIV4);
	STM_EVAL_LEDOn(LED_Orange);


}
//---------------------------------------------------------------------------
/**
 * @brief  Manages the DMA Complete Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void
EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{

	STM_EVAL_LEDOff(LED_Orange);
	make_sound(BUFF_LEN_DIV2, BUFF_LEN_DIV4);
	STM_EVAL_LEDOn(LED_Orange);

}

//---------------------------------------------------------------------------
/** NOT USED !
 * @brief  Get next data sample callback
 * @param  None
 * @retval Next data sample to be sent
 */
uint16_t
EVAL_AUDIO_GetSampleCallBack(void)
{
	return 0;
}


//--------------------------------- toggle ON/OFF volume ------------------------------------------
void toggleSound(void)
{
	if ( ! sound )
	{
		pitchGenResetPhase();
		EVAL_AUDIO_VolumeCtl(volume);
		sound = true;
	}
	else
	{
		EVAL_AUDIO_VolumeCtl(0);
		sound = false;
	}
}
//------------------------------- increase output DAC volume --------------------------------------------
void incVol(void)
{
	if (volume < MAXVOL)
	{
		volume++;
		EVAL_AUDIO_VolumeCtl(volume);
	}
}
//---------------------------------decrease output DAC volume ------------------------------------------
void decVol(void)
{
	if (volume > 0)
	{
		volume--;
		EVAL_AUDIO_VolumeCtl(volume);
	}
}

/*
*********************************************************************************************************
*
*	Module Name : Timer Module
*	File Name   : bsp_timer.h
*	Version     : V1.3
*	Description : Header File
*
*	Copyright (C), 2015-2016, Anfu Lai Electronics www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

/*
	Define several global variables for software timers here.
	Note: __IO (volatile) must be added because these variables are accessed in both interrupts and the main program, 
	which may cause compiler optimization errors.
*/
#define TMR_COUNT	4		/* Number of software timers (Timer ID range: 0 - 3) */

/* Timer mode enumeration. Members must be volatile to avoid issues with compiler optimization. */
typedef enum
{
	TMR_ONCE_MODE = 0,		/* One-time operation mode */
	TMR_AUTO_MODE = 1		/* Automatic timing operation mode */
}TMR_MODE_E;

/* Timer structure. Members must be volatile to avoid issues with compiler optimization. */
typedef struct
{
	volatile uint8_t Mode;		/* Counter mode, one-time */
	volatile uint8_t Flag;		/* Timer expiration flag */
	volatile uint32_t Count;	/* Counter */
	volatile uint32_t PreLoad;	/* Counter preload value */
}SOFT_TMR;

/* Functions provided for use by other C files */
void bsp_InitTimer(void);
void bsp_DelayMS(uint32_t n);
void bsp_DelayUS(uint32_t n);
void bsp_StartTimer(uint8_t _id, uint32_t _period);
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period);
void bsp_StopTimer(uint8_t _id);
uint8_t bsp_CheckTimer(uint8_t _id);
int32_t bsp_GetRunTime(void);
int32_t bsp_CheckRunTime(int32_t _LastTime);

void bsp_InitHardTimer(void);
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack);

#endif

/***************************** Anfu Lai Electronics www.armfly.com (END OF FILE) *********************************/

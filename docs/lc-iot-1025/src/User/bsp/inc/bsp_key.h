/*********************************************************************************************************
*
*	Module Name : Key Driver Module
*	File Name   : bsp_key.h
*	Version     : V1.0
*	Description : Header File
*
*	Copyright (C), 2013-2014, Anfu Lai Electronics www.armfly.com
*
*********************************************************************************************************/

#ifndef __BSP_KEY_H
#define __BSP_KEY_H

/* Rename key macros according to the functionality of the application */
#define KEY_DOWN_K1		KEY_1_DOWN
#define KEY_UP_K1		KEY_1_UP
#define KEY_LONG_K1		KEY_1_LONG

#define KEY_DOWN_K2		KEY_2_DOWN
#define KEY_UP_K2		KEY_2_UP
#define KEY_LONG_K2		KEY_2_LONG

#define KEY_DOWN_K3		KEY_3_DOWN
#define KEY_UP_K3		KEY_3_UP
#define KEY_LONG_K3		KEY_3_LONG

#define JOY_DOWN_U		KEY_4_DOWN		/* Up */
#define JOY_UP_U		KEY_4_UP
#define JOY_LONG_U		KEY_4_LONG

#define JOY_DOWN_D		KEY_5_DOWN		/* Down */
#define JOY_UP_D		KEY_5_UP
#define JOY_LONG_D		KEY_5_LONG

#define JOY_DOWN_L		KEY_6_DOWN		/* Left */
#define JOY_UP_L		KEY_6_UP
#define JOY_LONG_L		KEY_6_LONG

#define JOY_DOWN_R		KEY_7_DOWN		/* Right */
#define JOY_UP_R		KEY_7_UP
#define JOY_LONG_R		KEY_7_LONG

#define JOY_DOWN_OK		KEY_8_DOWN		/* ok */
#define JOY_UP_OK		KEY_8_UP
#define JOY_LONG_OK		KEY_8_LONG

#define SYS_DOWN_K1K2	KEY_9_DOWN		/* K1 K2 combination key */
#define SYS_UP_K1K2	    KEY_9_UP
#define SYS_LONG_K1K2	KEY_9_LONG

#define SYS_DOWN_K2K3	KEY_10_DOWN		/* K2 K3 combination key */
#define SYS_UP_K2K3  	KEY_10_UP
#define SYS_LONG_K2K3	KEY_10_LONG

/* Key ID, mainly used as the input parameter for the bsp_KeyState() function */
typedef enum
{
	KID_K1 = 0,
	KID_K2,
	KID_K3,
	KID_JOY_U,
	KID_JOY_D,
	KID_JOY_L,
	KID_JOY_R,
	KID_JOY_OK
}KEY_ID_E;

/*
	Key debounce time is 50ms, in units of 10ms.
	Only when the state is continuously detected as unchanged for 50ms is it considered valid, including both press and release events.
	Even if the key circuit does not implement hardware debounce, this debounce mechanism can reliably detect key events.
*/
#define KEY_FILTER_TIME   5
#define KEY_LONG_TIME     100			/* Unit: 10ms, sustained for 1 second, considered a long press event */

/*
	Each key corresponds to a global structure variable.
*/
typedef struct
{
	/* Below is a function pointer pointing to the function that determines if the key is pressed */
	uint8_t (*IsKeyDownFunc)(void); /* Function to determine if the key is pressed, 1 indicates pressed */

	uint8_t  Count;			/* Debounce counter */
	uint16_t LongCount;		/* Long press counter */
	uint16_t LongTime;		/* Duration the key is pressed, 0 means long press is not detected */
	uint8_t  State;			/* Current state of the key (pressed or released) */
	uint8_t  RepeatSpeed;	/* Continuous key press period */
	uint8_t  RepeatCount;	/* Continuous key press counter */
}KEY_T;

/*
	Define key value codes, each key's press, release, and long press events must be defined in the following order.

	It is recommended to use enum instead of #define for the following reasons:
	(1) Easier to add new key values and adjust the order, making the code more readable.
	(2) The compiler can help avoid duplicate key values.
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 indicates no key event */

	KEY_1_DOWN,				/* Key 1 pressed */
	KEY_1_UP,				/* Key 1 released */
	KEY_1_LONG,				/* Key 1 long press */

	KEY_2_DOWN,				/* Key 2 pressed */
	KEY_2_UP,				/* Key 2 released */
	KEY_2_LONG,				/* Key 2 long press */

	KEY_3_DOWN,				/* Key 3 pressed */
	KEY_3_UP,				/* Key 3 released */
	KEY_3_LONG,				/* Key 3 long press */

	KEY_4_DOWN,				/* Key 4 pressed */
	KEY_4_UP,				/* Key 4 released */
	KEY_4_LONG,				/* Key 4 long press */

	KEY_5_DOWN,				/* Key 5 pressed */
	KEY_5_UP,				/* Key 5 released */
	KEY_5_LONG,				/* Key 5 long press */

	KEY_6_DOWN,				/* Key 6 pressed */
	KEY_6_UP,				/* Key 6 released */
	KEY_6_LONG,				/* Key 6 long press */

	KEY_7_DOWN,				/* Key 7 pressed */
	KEY_7_UP,				/* Key 7 released */
	KEY_7_LONG,				/* Key 7 long press */

	KEY_8_DOWN,				/* Key 8 pressed */
	KEY_8_UP,				/* Key 8 released */
	KEY_8_LONG,				/* Key 8 long press */

	/* Combination keys */
	KEY_9_DOWN,				/* Key 9 pressed */
	KEY_9_UP,				/* Key 9 released */
	KEY_9_LONG,				/* Key 9 long press */

	KEY_10_DOWN,			/* Key 10 pressed */
	KEY_10_UP,				/* Key 10 released */
	KEY_10_LONG,			/* Key 10 long press */
}KEY_ENUM;

/* Variables used for key FIFO */
#define KEY_FIFO_SIZE	10
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* Key value buffer */
	uint8_t Read;					/* Buffer read pointer 1 */
	uint8_t Write;					/* Buffer write pointer */
	uint8_t Read2;					/* Buffer read pointer 2 */
}KEY_FIFO_T;

/* Function declarations for external use */
void bsp_InitKey(void);
void bsp_KeyScan10ms(void);
void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
uint8_t bsp_GetKey2(void);
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID);
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed);
void bsp_ClearKey(void);

#endif

/***************************** Anfu Lai Electronics www.armfly.com (END OF FILE) *********************************/

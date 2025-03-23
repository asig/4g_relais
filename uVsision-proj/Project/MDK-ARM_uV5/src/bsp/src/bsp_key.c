/*
*********************************************************************************************************
*
*	Module Name : Independent Key Driver Module (External Input IO)
*	File Name   : bsp_key.c
*	Version     : V1.3
*	Description : Scans independent keys with software debounce mechanism and key FIFO. 
*	              Can detect the following events:
*	              (1) Key press
*	              (2) Key release
*	              (3) Long press
*	              (4) Auto-repeat during long press
*
*	Change Log  :
*		Version   Date        Author   Description
*		V1.0      2013-02-01  armfly   Initial release
*		V1.1      2013-06-29  armfly   Added a second read pointer for bsp_Idle() function to read system 
*		                               control combination keys (e.g., screenshot).
*		                               Added K1 K2 combination key and K2 K3 combination key for system control.
*		V1.2      2016-01-25  armfly   Adjusted for P02 industrial control board. Simplified GPIO definition.
*		V1.3      2018-11-26  armfly   Initialized s_tBtn structure to 0.
*
*	Copyright (C), 2016-2020, Anfu Lai Electronics www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"

/*
	Anfulai STM32-V6 Key Port Line Assignment:
		K1 Key      : PB12   (Low level indicates pressed)
*/

#define HARD_KEY_NUM        1                           /* Number of physical keys */
#define KEY_COUNT           (HARD_KEY_NUM + 2)         /* 8 independent keys + 2 combination keys */

/* Enable GPIO clock */
#define ALL_KEY_GPIO_CLK_ENABLE() { \
		__HAL_RCC_GPIOB_CLK_ENABLE(); \
	};

/* Define GPIO sequentially */
typedef struct
{
	GPIO_TypeDef* gpio;
	uint16_t pin;
	uint8_t ActiveLevel;    /* Active level */
} X_GPIO_T;

/* GPIO and PIN definitions */
static const X_GPIO_T s_gpio_list[HARD_KEY_NUM] = {
	{GPIOB, GPIO_PIN_12, 0},        /* K1 */
};

/* Define a macro function to simplify subsequent code 
   Determine if the GPIO pin is effectively pressed
*/
static KEY_T s_tBtn[KEY_COUNT] = {0};
static KEY_FIFO_T s_tKey;        /* Key FIFO variable, structure */

static void bsp_InitKeyVar(void);
static void bsp_InitKeyHard(void);
static void bsp_DetectKey(uint8_t i);

#define KEY_PIN_ACTIVE(id)

/*
*********************************************************************************************************
*   Function Name: KeyPinActive
*   Description: Determine if the key is pressed
*   Parameters: None
*   Return Value: Returns 1 if pressed (conducted), 0 if not pressed (released)
*********************************************************************************************************
*/
static uint8_t KeyPinActive(uint8_t _id)
{
	uint8_t level;

	if ((s_gpio_list[_id].gpio->IDR & s_gpio_list[_id].pin) == 0)
	{
		level = 0;
	}
	else
	{
		level = 1;
	}

	if (level == s_gpio_list[_id].ActiveLevel)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*   Function Name: IsKeyDownFunc
*   Description: Determine if the key is pressed. Distinguish between single and combination keys. 
*                Single key events do not allow other keys to be pressed.
*   Parameters: None
*   Return Value: Returns 1 if pressed (conducted), 0 if not pressed (released)
*********************************************************************************************************
*/
static uint8_t IsKeyDownFunc(uint8_t _id)
{
	/* Physical single key */
	if (_id < HARD_KEY_NUM)
	{
		uint8_t i;
		uint8_t count = 0;
		uint8_t save = 255;

		/* Determine how many keys are pressed */
		for (i = 0; i < HARD_KEY_NUM; i++)
		{
			if (KeyPinActive(i)) 
			{
				count++;
				save = i;
			}
		}

		if (count == 1 && save == _id)
		{
			return 1;    /* Valid only when one key is pressed */
		}        

		return 0;
	}

	/* Combination key K1K2 */
	if (_id == HARD_KEY_NUM + 0)
	{
		if (KeyPinActive(KID_K1) && KeyPinActive(KID_K2))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	/* Combination key K2K3 */
	if (_id == HARD_KEY_NUM + 1)
	{
		if (KeyPinActive(KID_K2) && KeyPinActive(KID_K3))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	return 0;
}

/*
*********************************************************************************************************
*   Function Name: bsp_InitKey
*   Description: Initialize keys. This function is called by bsp_Init().
*   Parameters: None
*   Return Value: None
*********************************************************************************************************
*/
void bsp_InitKey(void)
{
	bsp_InitKeyVar();        /* Initialize key variables */
	bsp_InitKeyHard();       /* Initialize key hardware */
}

/*
*********************************************************************************************************
*   Function Name: bsp_InitKeyHard
*   Description: Configure GPIO corresponding to keys
*   Parameters: None
*   Return Value: None
*********************************************************************************************************
*/
static void bsp_InitKeyHard(void)
{    
	GPIO_InitTypeDef gpio_init;
	uint8_t i;

	/* Step 1: Enable GPIO clock */
	ALL_KEY_GPIO_CLK_ENABLE();

	/* Step 2: Configure all key GPIOs as floating input mode 
	   (actually, the CPU is in input state after reset) */
	gpio_init.Mode = GPIO_MODE_INPUT;            /* Set input */
	gpio_init.Pull = GPIO_NOPULL;                /* Disable pull-up/down resistors */
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;      /* GPIO speed level */

	for (i = 0; i < HARD_KEY_NUM; i++)
	{
		gpio_init.Pin = s_gpio_list[i].pin;
		HAL_GPIO_Init(s_gpio_list[i].gpio, &gpio_init);    
	}
}

/*
*********************************************************************************************************
*   Function Name: bsp_InitKeyVar
*   Description: Initialize key variables
*   Parameters: None
*   Return Value: None
*********************************************************************************************************
*/
static void bsp_InitKeyVar(void)
{
	uint8_t i;

	/* Clear key FIFO read/write pointers */
	s_tKey.Read = 0;
	s_tKey.Write = 0;
	s_tKey.Read2 = 0;

	/* Assign a set of default values to each key structure member variable */
	for (i = 0; i < KEY_COUNT; i++)
	{
		s_tBtn[i].LongTime = KEY_LONG_TIME;            /* Long press time 0 means no long press event detection */
		s_tBtn[i].Count = KEY_FILTER_TIME / 2;         /* Counter set to half the filter time */
		s_tBtn[i].State = 0;                           /* Default key state, 0 means not pressed */
		s_tBtn[i].RepeatSpeed = 0;                     /* Key repeat speed, 0 means no repeat */
		s_tBtn[i].RepeatCount = 0;                     /* Repeat counter */
	}

	/* If you need to change the parameters of a specific key individually, you can reassign here */
	
//    /* Joystick up, down, left, right, supports auto-repeat after 1 second long press */
//    bsp_SetKeyParam(KID_JOY_U, 100, 6);
//    bsp_SetKeyParam(KID_JOY_D, 100, 6);
//    bsp_SetKeyParam(KID_JOY_L, 100, 6);
//    bsp_SetKeyParam(KID_JOY_R, 100, 6);
}

/*
*********************************************************************************************************
*   Function Name: bsp_PutKey
*   Description: Push a key value into the key FIFO buffer. Can be used to simulate a key press.
*   Parameters: _KeyCode : Key code
*   Return Value: None
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode)
{
	s_tKey.Buf[s_tKey.Write] = _KeyCode;

	if (++s_tKey.Write >= KEY_FIFO_SIZE)
	{
		s_tKey.Write = 0;
	}
}
/*
*********************************************************************************************************
*   Function Name: bsp_GetKey
*   Description: Reads a key value from the key FIFO buffer.
*   Parameters: None
*   Return Value: Key code
*********************************************************************************************************
*/
uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_tKey.Read == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read];

		if (++s_tKey.Read >= KEY_FIFO_SIZE)
		{
			s_tKey.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*   Function Name: bsp_GetKey2
*   Description: Reads a key value from the key FIFO buffer using an independent read pointer.
*   Parameters: None
*   Return Value: Key code
*********************************************************************************************************
*/
uint8_t bsp_GetKey2(void)
{
	uint8_t ret;

	if (s_tKey.Read2 == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read2];

		if (++s_tKey.Read2 >= KEY_FIFO_SIZE)
		{
			s_tKey.Read2 = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*   Function Name: bsp_GetKeyState
*   Description: Reads the state of a key.
*   Parameters: _ucKeyID : Key ID, starting from 0
*   Return Value: 1 indicates pressed, 0 indicates not pressed
*********************************************************************************************************
*/
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID)
{
	return s_tBtn[_ucKeyID].State;
}

/*
*********************************************************************************************************
*   Function Name: bsp_SetKeyParam
*   Description: Sets key parameters.
*   Parameters: _ucKeyID : Key ID, starting from 0
*               _LongTime : Long press event time
*               _RepeatSpeed : Auto-repeat speed
*   Return Value: None
*********************************************************************************************************
*/
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t _RepeatSpeed)
{
	s_tBtn[_ucKeyID].LongTime = _LongTime;            /* Long press time, 0 means no long press event detection */
	s_tBtn[_ucKeyID].RepeatSpeed = _RepeatSpeed;      /* Key auto-repeat speed, 0 means no auto-repeat */
	s_tBtn[_ucKeyID].RepeatCount = 0;                /* Auto-repeat counter */
}

/*
*********************************************************************************************************
*   Function Name: bsp_ClearKey
*   Description: Clears the key FIFO buffer.
*   Parameters: None
*   Return Value: None
*********************************************************************************************************
*/
void bsp_ClearKey(void)
{
	s_tKey.Read = s_tKey.Write;
}

/*
*********************************************************************************************************
*   Function Name: bsp_DetectKey
*   Description: Detects a key. Non-blocking, must be called periodically.
*   Parameters: IO ID, starting from 0
*   Return Value: None
*********************************************************************************************************
*/
static void bsp_DetectKey(uint8_t i)
{
	KEY_T *pBtn;

	pBtn = &s_tBtn[i];
	if (IsKeyDownFunc(i))
	{
		if (pBtn->Count < KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if (pBtn->Count < 2 * KEY_FILTER_TIME)
		{
			pBtn->Count++;
		}
		else
		{
			if (pBtn->State == 0)
			{
				pBtn->State = 1;

				/* Send button press message */
				bsp_PutKey((uint8_t)(3 * i + 1));
			}

			if (pBtn->LongTime > 0)
			{
				if (pBtn->LongCount < pBtn->LongTime)
				{
					/* Send button hold message */
					if (++pBtn->LongCount == pBtn->LongTime)
					{
						/* Put key value into key FIFO */
						bsp_PutKey((uint8_t)(3 * i + 3));
					}
				}
				else
				{
					if (pBtn->RepeatSpeed > 0)
					{
						if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
						{
							pBtn->RepeatCount = 0;
							/* After long press, send 1 key every 10ms */
							bsp_PutKey((uint8_t)(3 * i + 1));
						}
					}
				}
			}
		}
	}
	else
	{
		if (pBtn->Count > KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if (pBtn->Count != 0)
		{
			pBtn->Count--;
		}
		else
		{
			if (pBtn->State == 1)
			{
				pBtn->State = 0;

				/* Send button release message */
				bsp_PutKey((uint8_t)(3 * i + 2));
			}
		}

		pBtn->LongCount = 0;
		pBtn->RepeatCount = 0;
	}
}

/*
*********************************************************************************************************
*   Function Name: bsp_DetectFastIO
*   Description: Detects high-speed input IO. Refreshes every 1ms.
*   Parameters: IO ID, starting from 0
*   Return Value: None
*********************************************************************************************************
*/
static void bsp_DetectFastIO(uint8_t i)
{
	KEY_T *pBtn;

	pBtn = &s_tBtn[i];
	if (IsKeyDownFunc(i))
	{
		if (pBtn->State == 0)
		{
			pBtn->State = 1;

			/* Send button press message */
			bsp_PutKey((uint8_t)(3 * i + 1));
		}

		if (pBtn->LongTime > 0)
		{
			if (pBtn->LongCount < pBtn->LongTime)
			{
				/* Send button hold message */
				if (++pBtn->LongCount == pBtn->LongTime)
				{
					/* Put key value into key FIFO */
					bsp_PutKey((uint8_t)(3 * i + 3));
				}
			}
			else
			{
				if (pBtn->RepeatSpeed > 0)
				{
					if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
					{
						pBtn->RepeatCount = 0;
						/* After long press, send 1 key every 10ms */
						bsp_PutKey((uint8_t)(3 * i + 1));
					}
				}
			}
		}
	}
	else
	{
		if (pBtn->State == 1)
		{
			pBtn->State = 0;

			/* Send button release message */
			bsp_PutKey((uint8_t)(3 * i + 2));
		}

		pBtn->LongCount = 0;
		pBtn->RepeatCount = 0;
	}
}

/*
*********************************************************************************************************
*   Function Name: bsp_KeyScan10ms
*   Description: Scans all keys. Non-blocking, called periodically by systick interrupt every 10ms.
*   Parameters: None
*   Return Value: None
*********************************************************************************************************
*/
void bsp_KeyScan10ms(void)
{
	uint8_t i;

	for (i = 0; i < KEY_COUNT; i++)
	{
		bsp_DetectKey(i);
	}
}

/*
*********************************************************************************************************
*   Function Name: bsp_KeyScan1ms
*   Description: Scans all keys. Non-blocking, called periodically by systick interrupt every 1ms.
*   Parameters: None
*   Return Value: None
*********************************************************************************************************
*/
void bsp_KeyScan1ms(void)
{
	uint8_t i;

	for (i = 0; i < KEY_COUNT; i++)
	{
		bsp_DetectFastIO(i);
	}
}

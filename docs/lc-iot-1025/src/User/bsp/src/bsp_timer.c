/*
*********************************************************************************************************
*
*	Module Name : Timer Module
*	File Name   : bsp_timer.c
*	Version     : V1.5
*	Description : Configures the systick timer as the system tick timer. The default tick period is 1ms.
*
*				  Implements multiple software timers for the main program (precision 1ms), 
*				  which can be increased or decreased by modifying TMR_COUNT.
*				  Implements millisecond-level delay functions (precision 1ms) and microsecond-level delay functions.
*				  Implements a system runtime function (1ms unit).
*
*	Change Log  :
*		Version   Date        Author   Description
*		V1.0      2013-02-01  armfly   Official release.
*		V1.1      2013-06-21  armfly   Added microsecond-level delay function bsp_DelayUS.
*		V1.2      2014-09-07  armfly   Added TIM4 hardware timer interrupt for microsecond-level timing (20us - 16 seconds).
*		V1.3      2015-04-06  armfly   Added bsp_CheckRunTime(int32_t _LastTime) to calculate time differences.
*		V1.4      2015-05-22  armfly   Improved bsp_InitHardTimer(), added conditional compilation for TIM2-5.
*		V1.5      2018-11-26  armfly   Initialized s_tTmr to 0; added g_ucEnableSystickISR variable to avoid exceptions caused by HAL prematurely enabling systick interrupts.
*
*	Copyright (C), 2015-2030, Anfu Lai Electronics www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
/*
	Define the TIM used for hardware timers, can use TIM2 - TIM5
*/
#define USE_TIM2
//#define USE_TIM3
//#define USE_TIM4
//#define USE_TIM5

#ifdef USE_TIM2
	#define TIM_HARD					TIM2
	#define	RCC_TIM_HARD_CLK_ENABLE()	__HAL_RCC_TIM2_CLK_ENABLE()
	#define TIM_HARD_IRQn				TIM2_IRQn
	#define TIM_HARD_IRQHandler			TIM2_IRQHandler
#endif

#ifdef USE_TIM3
	#define TIM_HARD					TIM3
	#define	RCC_TIM_HARD_CLK_ENABLE()	__HAL_RCC_TIM3_CLK_ENABLE()	
	#define TIM_HARD_IRQn				TIM3_IRQn
	#define TIM_HARD_IRQHandler			TIM3_IRQHandler
#endif

#ifdef USE_TIM4
	#define TIM_HARD					TIM4
	#define	RCC_TIM_HARD_CLK_ENABLE()	__HAL_RCC_TIM4_CLK_ENABLE()
	#define TIM_HARD_IRQn				TIM4_IRQn
	#define TIM_HARD_IRQHandler			TIM4_IRQHandler
#endif

#ifdef USE_TIM5
	#define TIM_HARD					TIM5
	#define	RCC_TIM_HARD_CLK_ENABLE()	__HAL_RCC_TIM5_CLK_ENABLE()
	#define TIM_HARD_IRQn				TIM5_IRQn
	#define TIM_HARD_IRQHandler			TIM5_IRQHandler
#endif
/* Save callback function pointers for TIM interrupt */
static void (*s_TIM_CallBack1)(void);
static void (*s_TIM_CallBack2)(void);
static void (*s_TIM_CallBack3)(void);
static void (*s_TIM_CallBack4)(void);

/* These two global variables are used for the bsp_DelayMS() function */
static volatile uint32_t s_uiDelayCount = 0;
static volatile uint8_t s_ucTimeOutFlag = 0;

/* Define software timer structure variables */
static SOFT_TMR s_tTmr[TMR_COUNT] = {0};

/*
	Global runtime in milliseconds
	Maximum duration is 24.85 days. If your product runs continuously beyond this, consider overflow handling.
*/
__IO int32_t g_iRunTime = 0;

static __IO uint8_t g_ucEnableSystickISR = 0; /* Wait for variable initialization */

static void bsp_SoftTimerDec(SOFT_TMR *_tmr);
/*
*********************************************************************************************************
*	Function Name: bsp_InitTimer
*	Description  : Configures the systick interrupt and initializes software timer variables.
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
void bsp_InitTimer(void)
{
	uint8_t i;

	/* Clear all software timers */
	for (i = 0; i < TMR_COUNT; i++)
	{
		s_tTmr[i].Count = 0;
		s_tTmr[i].PreLoad = 0;
		s_tTmr[i].Flag = 0;
		s_tTmr[i].Mode = TMR_ONCE_MODE;	/* Default is one-time mode */
	}

	/*
		Configure systick interrupt period to 1ms and start the systick interrupt.

		SystemCoreClock is the system core clock defined in the firmware, typically 400MHz for STM32H7.

		The parameter of SysTick_Config() specifies how many core clock cycles trigger a Systick interrupt:
			-- SystemCoreClock / 1000  sets the timer frequency to 1000Hz, i.e., a period of 1ms.
			-- SystemCoreClock / 500   sets the timer frequency to 500Hz, i.e., a period of 2ms.
			-- SystemCoreClock / 2000  sets the timer frequency to 2000Hz, i.e., a period of 500us.

		For regular applications, we typically set the timer period to 1ms. For low-speed CPUs or low-power applications, 
		the timer period can be set to 10ms.
	*/
	SysTick_Config(SystemCoreClock / 1000);
	
	g_ucEnableSystickISR = 1;		/* 1 indicates systick interrupt execution */
	
	bsp_InitHardTimer();
}

/*
*********************************************************************************************************
*	Function Name: SysTick_ISR
*	Description  : SysTick interrupt service routine, triggered every 1ms.
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
extern void bsp_RunPer1ms(void);
extern void bsp_RunPer10ms(void);
void SysTick_ISR(void)
{
	static uint8_t s_count = 0;
	uint8_t i;
	
	/* Enter every 1ms (used only for bsp_DelayMS) */
	if (s_uiDelayCount > 0)
	{
		if (--s_uiDelayCount == 0)
		{
			s_ucTimeOutFlag = 1;
		}
	}

	/* Decrement the counters of software timers every 1ms */
	for (i = 0; i < TMR_COUNT; i++)
	{
		bsp_SoftTimerDec(&s_tTmr[i]);
	}

	/* Increment global runtime every 1ms */
	g_iRunTime++;
	if (g_iRunTime == 0x7FFFFFFF)	/* This variable is of type int32_t, max value is 0x7FFFFFFF */
	{
		g_iRunTime = 0;
	}

	bsp_RunPer1ms();		/* Call this function every 1ms, defined in bsp.c */

	if (++s_count >= 10)
	{
		s_count = 0;

		bsp_RunPer10ms();	/* Call this function every 10ms, defined in bsp.c */
	}
}
/*
*********************************************************************************************************
*	Function Name: bsp_SoftTimerDec
*	Description  : Decrements all timer variables by 1 every 1ms. Must be periodically called by SysTick_ISR.
*	Parameters   : _tmr : Pointer to the timer variable
*	Return Value : None
*********************************************************************************************************
*/
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
	if (_tmr->Count > 0)
	{
		/* If the timer variable decrements to 1, set the timer timeout flag */
		if (--_tmr->Count == 0)
		{
			_tmr->Flag = 1;

			/* If in auto mode, automatically reload the counter */
			if (_tmr->Mode == TMR_AUTO_MODE)
			{
				_tmr->Count = _tmr->PreLoad;
			}
		}
	}
}

/*
*********************************************************************************************************
*	Function Name: bsp_DelayMS
*	Description  : Millisecond-level delay with an accuracy of ±1ms
*	Parameters   : n : Delay length in milliseconds. n should be greater than 2.
*	Return Value : None
*********************************************************************************************************
*/
void bsp_DelayMS(uint32_t n)
{
	if (n == 0)
	{
		return;
	}
	else if (n == 1)
	{
		n = 2;
	}

	DISABLE_INT();  			/* Disable interrupts */

	s_uiDelayCount = n;
	s_ucTimeOutFlag = 0;

	ENABLE_INT();  				/* Enable interrupts */

	while (1)
	{
		bsp_Idle();				/* CPU idle operation, see bsp.c and bsp.h files */

		/*
			Wait for the delay time to expire
			Note: The compiler might optimize incorrectly assuming s_ucTimeOutFlag = 0, 
			so the s_ucTimeOutFlag variable must be declared as volatile.
		*/
		if (s_ucTimeOutFlag == 1)
		{
			break;
		}
	}
}
/*
*********************************************************************************************************
*    Function Name: bsp_DelayUS
*    Description  : Microsecond-level delay. This function must be called after the systick timer is started.
*    Parameters   : n : Delay length in microseconds.
*    Return Value : None
*********************************************************************************************************
*/
void bsp_DelayUS(uint32_t n)
{
	uint32_t ticks;
	uint32_t told;
	uint32_t tnow;
	uint32_t tcnt = 0;
	uint32_t reload;
	   
	reload = SysTick->LOAD;                
	ticks = n * (SystemCoreClock / 1000000);	 /* Required tick count */  
	
	tcnt = 0;
	told = SysTick->VAL;             /* Counter value at entry */

	while (1)
	{
		tnow = SysTick->VAL;    
		if (tnow != told)
		{    
			/* SYSTICK is a decrementing counter */    
			if (tnow < told)
			{
				tcnt += told - tnow;    
			}
			/* Reload decrement */
			else
			{
				tcnt += reload - tnow + told;    
			}        
			told = tnow;

			/* Exit if the elapsed time is greater than or equal to the delay time */
			if (tcnt >= ticks)
			{
				break;
			}
		}  
	}
} 

/*
*********************************************************************************************************
*    Function Name: bsp_StartTimer
*    Description  : Starts a timer and sets the timer period.
*    Parameters   : _id     : Timer ID, range [0, TMR_COUNT-1]. Users must manage timer IDs to avoid conflicts.
*                   _period : Timer period in milliseconds.
*    Return Value : None
*********************************************************************************************************
*/
void bsp_StartTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* Print the source file name and function name of the error */
		BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* Parameter error, halt and wait for watchdog reset */
	}

	DISABLE_INT();  			/* Disable interrupts */

	s_tTmr[_id].Count = _period;		/* Initial value of the real-time counter */
	s_tTmr[_id].PreLoad = _period;		/* Auto-reload value of the counter, only effective in auto mode */
	s_tTmr[_id].Flag = 0;				/* Timer timeout flag */
	s_tTmr[_id].Mode = TMR_ONCE_MODE;	/* One-time operation mode */

	ENABLE_INT();  				/* Enable interrupts */
}

/*
*********************************************************************************************************
*    Function Name: bsp_StartAutoTimer
*    Description  : Starts an auto timer and sets the timer period.
*    Parameters   : _id     : Timer ID, range [0, TMR_COUNT-1]. Users must manage timer IDs to avoid conflicts.
*                   _period : Timer period in 10ms units.
*    Return Value : None
*********************************************************************************************************
*/
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* Print the source file name and function name of the error */
		BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* Parameter error, halt and wait for watchdog reset */
	}

	DISABLE_INT();  		/* Disable interrupts */

	s_tTmr[_id].Count = _period;			/* Initial value of the real-time counter */
	s_tTmr[_id].PreLoad = _period;		/* Auto-reload value of the counter, only effective in auto mode */
	s_tTmr[_id].Flag = 0;				/* Timer timeout flag */
	s_tTmr[_id].Mode = TMR_AUTO_MODE;	/* Auto operation mode */

	ENABLE_INT();  			/* Enable interrupts */
}

/*
*********************************************************************************************************
*    Function Name: bsp_StopTimer
*    Description  : Stops a timer.
*    Parameters   : _id     : Timer ID, range [0, TMR_COUNT-1]. Users must manage timer IDs to avoid conflicts.
*    Return Value : None
*********************************************************************************************************
*/
void bsp_StopTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		/* Print the source file name and function name of the error */
		BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* Parameter error, halt and wait for watchdog reset */
	}

	DISABLE_INT();  	/* Disable interrupts */

	s_tTmr[_id].Count = 0;				/* Initial value of the real-time counter */
	s_tTmr[_id].Flag = 0;				/* Timer timeout flag */
	s_tTmr[_id].Mode = TMR_ONCE_MODE;	/* Auto operation mode */

	ENABLE_INT();  		/* Enable interrupts */
}
/*
*********************************************************************************************************
*	Function Name: bsp_CheckTimer
*	Description  : Checks if the timer has timed out.
*	Parameters   : _id     : Timer ID, range [0, TMR_COUNT-1]. Users must manage timer IDs to avoid conflicts.
*				  _period : Timer period in milliseconds.
*	Return Value : Returns 0 if the timer has not timed out, 1 if it has.
*********************************************************************************************************
*/
uint8_t bsp_CheckTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		return 0;
	}

	if (s_tTmr[_id].Flag == 1)
	{
		s_tTmr[_id].Flag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	Function Name: bsp_GetRunTime
*	Description  : Gets the CPU runtime in milliseconds. The maximum duration is 24.85 days. If your product
*				  runs continuously beyond this, consider overflow handling.
*	Parameters   : None
*	Return Value : CPU runtime in milliseconds.
*********************************************************************************************************
*/
int32_t bsp_GetRunTime(void)
{
	int32_t runtime;

	DISABLE_INT();  	/* Disable interrupts */

	runtime = g_iRunTime;	/* This variable is modified in the Systick interrupt, so it needs interrupt protection. */

	ENABLE_INT();  		/* Enable interrupts */

	return runtime;
}

/*
*********************************************************************************************************
*	Function Name: bsp_CheckRunTime
*	Description  : Calculates the difference between the current runtime and a given time. Handles counter overflow.
*	Parameters   : _LastTime : The previous time.
*	Return Value : The difference between the current time and the past time, in milliseconds.
*********************************************************************************************************
*/
int32_t bsp_CheckRunTime(int32_t _LastTime)
{
	int32_t now_time;
	int32_t time_diff;

	DISABLE_INT();  	/* Disable interrupts */

	now_time = g_iRunTime;	/* This variable is modified in the Systick interrupt, so it needs interrupt protection. */

	ENABLE_INT();  		/* Enable interrupts */
	
	if (now_time >= _LastTime)
	{
		time_diff = now_time - _LastTime;
	}
	else
	{
		time_diff = 0x7FFFFFFF - _LastTime + now_time;
	}

	return time_diff;
}

/*
*********************************************************************************************************
*	Function Name: SysTick_Handler
*	Description  : System tick timer interrupt service routine. Referenced in the startup file.
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
void SysTick_Handler(void)
{
	HAL_IncTick();	/* Tick interrupt service routine from the ST HAL library */
	
	if (g_ucEnableSystickISR == 0)
	{
		return;
	}
	
	SysTick_ISR();	/* Tick interrupt service routine from the Anfu Lai bsp library */
}

/*
*********************************************************************************************************
*	The following code uses one TIM with 4 capture interrupts to implement 4 hardware timers.
*********************************************************************************************************
*/

#ifdef TIM_HARD

/*
*********************************************************************************************************
*	Function Name: bsp_InitHardTimer
*	Description  : Configures TIMx for microsecond-level hardware timing. TIMx will run freely and never stop.
*				  TIMx can use TIM2 - TIM5, which have 4 channels and are on APB1 with an input clock of SystemCoreClock / 2.
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
void bsp_InitHardTimer(void)
{
	TIM_HandleTypeDef  TimHandle = {0};
	uint32_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;
	TIM_TypeDef* TIMx = TIM_HARD;
	
	RCC_TIM_HARD_CLK_ENABLE();		/* Enable TIM clock */
	
	/*-----------------------------------------------------------------------
		In the system_stm32f4xx.c file, the SetSysClock(void) function configures the clock as follows:

		HCLK = SYSCLK / 1     (AHB1Periph)
		PCLK2 = HCLK / 2      (APB2Periph)
		PCLK1 = HCLK / 4      (APB1Periph)

		Since the APB1 prescaler != 1, the TIMxCLK on APB1 = PCLK1 x 2 = SystemCoreClock / 2;
		Since the APB2 prescaler != 1, the TIMxCLK on APB2 = PCLK2 x 2 = SystemCoreClock;

		APB1 timers: TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
		APB2 timers: TIM1, TIM8, TIM9, TIM10, TIM11

	----------------------------------------------------------------------- */
	uiTIMxCLK = SystemCoreClock / 2;

	usPrescaler = uiTIMxCLK / 1000000 - 1;	/* Prescaler = 1 */
	if (TIMx == TIM2 )
	{

		usPeriod = 0xFFFF;
	}

	/* 
	   Setting the prescaler to usPrescaler means the timer counter increments every 1 microsecond.
	   The value of usPeriod determines the maximum count:
	   usPeriod = 0xFFFF means a maximum of 0xFFFF microseconds.
	   usPeriod = 0xFFFFFFFF means a maximum of 0xFFFFFFFF microseconds.
	*/
	TimHandle.Instance = TIMx;
	TimHandle.Init.Prescaler         = usPrescaler;
	TimHandle.Init.Period            = usPeriod;
	TimHandle.Init.ClockDivision     = 0;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;
	TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	
	if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	/* Configure timer interrupt for CC capture compare interrupt usage */
	{
		HAL_NVIC_SetPriority(TIM_HARD_IRQn, 0, 2);
		HAL_NVIC_EnableIRQ(TIM_HARD_IRQn);	
	}
	
	/* Start the timer */
	HAL_TIM_Base_Start(&TimHandle);
}

/*
*********************************************************************************************************
*	Function Name: bsp_StartHardTimer
*	Description  : Uses TIM2-5 as a one-shot timer. Executes a callback function when the timer expires.
*				  Can simultaneously start 4 timer channels without interference.
*				  Timing accuracy is ±1us (mainly due to the execution time of this function).
*				  TIM2 and TIM5 are 32-bit timers with a large timing range.
*				  TIM3 and TIM4 are 16-bit timers.
*	Parameters   : _CC : Capture compare channel (1, 2, 3, or 4).
*				  _uiTimeOut : Timeout duration in microseconds. For 16-bit timers, the maximum is 65.5ms;
*							   for 32-bit timers, the maximum is 4294 seconds.
*				  _pCallBack : Function to execute when the timer expires.
*	Return Value : None
*********************************************************************************************************
*/
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack)
{
	uint32_t cnt_now;
	uint32_t cnt_tar;
	TIM_TypeDef* TIMx = TIM_HARD;
	
	/* No need for delay compensation; measured accuracy is ±1us. */
	
	cnt_now = TIMx->CNT; 
	cnt_tar = cnt_now + _uiTimeOut;			/* Calculate the counter value for capture compare */
	if (_CC == 1)
	{
		s_TIM_CallBack1 = (void (*)(void))_pCallBack;

		TIMx->CCR1 = cnt_tar; 			    /* Set capture compare counter CC1 */
		TIMx->SR = (uint16_t)~TIM_IT_CC1;   /* Clear CC1 interrupt flag */
		TIMx->DIER |= TIM_IT_CC1;			/* Enable CC1 interrupt */
	}
	else if (_CC == 2)
	{
		s_TIM_CallBack2 = (void (*)(void))_pCallBack;

		TIMx->CCR2 = cnt_tar;				/* Set capture compare counter CC2 */
		TIMx->SR = (uint16_t)~TIM_IT_CC2;	/* Clear CC2 interrupt flag */
		TIMx->DIER |= TIM_IT_CC2;			/* Enable CC2 interrupt */
	}
	else if (_CC == 3)
	{
		s_TIM_CallBack3 = (void (*)(void))_pCallBack;

		TIMx->CCR3 = cnt_tar;				/* Set capture compare counter CC3 */
		TIMx->SR = (uint16_t)~TIM_IT_CC3;	/* Clear CC3 interrupt flag */
		TIMx->DIER |= TIM_IT_CC3;			/* Enable CC3 interrupt */
	}
	else if (_CC == 4)
	{
		s_TIM_CallBack4 = (void (*)(void))_pCallBack;

		TIMx->CCR4 = cnt_tar;				/* Set capture compare counter CC4 */
		TIMx->SR = (uint16_t)~TIM_IT_CC4;	/* Clear CC4 interrupt flag */
		TIMx->DIER |= TIM_IT_CC4;			/* Enable CC4 interrupt */
	}
	else
	{
		return;
	}
}
/*
*********************************************************************************************************
*	Function Name: TIMx_IRQHandler
*	Description  : TIM interrupt service routine
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
void TIM_HARD_IRQHandler(void)
{
	uint16_t itstatus = 0x0, itenable = 0x0;
	TIM_TypeDef* TIMx = TIM_HARD;
	
    
  	itstatus = TIMx->SR & TIM_IT_CC1;
	itenable = TIMx->DIER & TIM_IT_CC1;
	if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
	{
		TIMx->SR = (uint16_t)~TIM_IT_CC1;
		TIMx->DIER &= (uint16_t)~TIM_IT_CC1;		/* Disable CC1 interrupt */

		/* Disable the interrupt first, then execute the callback function. 
		   This is because the callback function might need to restart the timer. */
		s_TIM_CallBack1();
	}

	itstatus = TIMx->SR & TIM_IT_CC2;
	itenable = TIMx->DIER & TIM_IT_CC2;
	if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
	{
		TIMx->SR = (uint16_t)~TIM_IT_CC2;
		TIMx->DIER &= (uint16_t)~TIM_IT_CC2;		/* Disable CC2 interrupt */	

		/* Disable the interrupt first, then execute the callback function. 
		   This is because the callback function might need to restart the timer. */
		s_TIM_CallBack2();
	}
	itstatus = TIMx->SR & TIM_IT_CC3;
	itenable = TIMx->DIER & TIM_IT_CC3;
	if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
	{
		TIMx->SR = (uint16_t)~TIM_IT_CC3;
		TIMx->DIER &= (uint16_t)~TIM_IT_CC3;		/* Disable CC3 interrupt */	

		/* Disable the interrupt first, then execute the callback function. 
		   This is because the callback function might need to restart the timer. */
		s_TIM_CallBack3();
	}

	itstatus = TIMx->SR & TIM_IT_CC4;
	itenable = TIMx->DIER & TIM_IT_CC4;
	if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
	TIMx->SR = (uint16_t)~TIM_IT_CC4;
	TIMx->DIER &= (uint16_t)~TIM_IT_CC4;		/* Disable CC4 interrupt */

	/* Disable the interrupt first, then execute the callback function. 
	   This is because the callback function might need to restart the timer. */
        s_TIM_CallBack4();
}	

#endif

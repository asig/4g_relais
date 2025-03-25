/*
*********************************************************************************************************
*
*	Module Name : BSP Module (For STM32F429)
*	File Name   : bsp.c
*	Version     : V1.0
*	Description : This is the main file for the hardware low-level driver. Each c file can include 
*				  "bsp.h" to include all peripheral driver modules.
*				  bsp = Board Support Package
*	Revision History :
*		Version    Date         Author      Description
*		V1.0       2018-07-29   Eric2013    Official release
*
*	Copyright (C), 2018-2030, Anfu Lai Electronics www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"



/*
*********************************************************************************************************
*	                                   Function Declarations
*********************************************************************************************************
*/
static void SystemClock_Config(void);


/*
*********************************************************************************************************
*	Function Name: bsp_Init
*	Description: Initializes all hardware devices. This function configures CPU registers and peripheral 
*                 registers and initializes some global variables. It only needs to be called once.
*	Parameters: None
*	Return Value: None
*********************************************************************************************************
*/
void bsp_Init(void)
{
	/* 
	   STM32H429 HAL library initialization. At this point, the system is still using the 16MHz HSI clock built into the F429:
	   - Calls the function HAL_InitTick to initialize the SysTick interrupt to 1ms.
	   - Sets the NVIC priority grouping to 4.
	 */
	HAL_Init();

	/* 
	   Configure the system clock to 168MHz
	   - Switch to using HSE.
	   - This function will update the global variable SystemCoreClock and reconfigure HAL_InitTick.
	*/
	SystemClock_Config();

	/* 
	   Event Recorder:
	   - Can be used for code execution time measurement, supported in MDK version 5.25 and above, not supported in IAR.
	   - Disabled by default. If you want to enable this option, be sure to refer to Chapter 8 of the V5 development board user manual.
	*/	
#if Enable_EventRecorder == 1  
	/* Initialize EventRecorder and enable it */
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
#endif
	
	bsp_InitKey();    	/* Key initialization, should be placed before the tick timer, as button detection is scanned via the tick timer */
	bsp_InitTimer();  	/* Initialize the tick timer */
	bsp_InitUart();		/* Initialize the UART */
//	bsp_InitExtIO();    /* Initialize extended IO */
//	bsp_InitLed();    	/* Initialize LED */
}

/*
*********************************************************************************************************
*	Function Name: SystemClock_Config
*	Description: Initializes the system clock
*            	System Clock source            = PLL (HSE)
*            	SYSCLK(Hz)                     = 168000000 (CPU Clock)
*            	HCLK = SYSCLK / 1              = 168000000 (AHB1Periph)
*            	PCLK2 = HCLK / 2               = 84000000  (APB2Periph)
*            	PCLK1 = HCLK / 4               = 42000000  (APB1Periph)
*            	HSE Frequency(Hz)              = 25000000
*           	PLL_M                          = 25
*            	PLL_N                          = 336
*            	PLL_P                          = 2
*            	PLL_Q                          = 4
*            	VDD(V)                         = 3.3
*            	Flash Latency(WS)              = 5
*	Parameters: None
*	Return Value: None
*********************************************************************************************************
*/

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
		Error_Handler(__FILE__, __LINE__);
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
		Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*	Function Name: Error_Handler
*	Parameters: file : Source code file name. The keyword __FILE__ represents the source code file name.
*			   line : Code line number. The keyword __LINE__ represents the source code line number.
*	Return Value: None
*		Error_Handler(__FILE__, __LINE__);
*********************************************************************************************************
*/
void Error_Handler(char *file, uint32_t line)
{
	/* 
		Users can add their own code to report the source code file name and line number, 
		for example, print the error file and line number to the UART:
		printf("Wrong parameters value: file %s on line %d\r\n", file, line) 
	*/
	
	/* This is an infinite loop. When an assertion fails, the program will hang here to help users debug. */
	if (line == 0)
	{
		return;
	}
	
	while(1)
	{
	}
}

/*
*********************************************************************************************************
*	Function Name: bsp_RunPer10ms
*	Description: This function is called once every 10ms by the Systick interrupt. See the timer interrupt 
*				 service routine in bsp_timer.c. Tasks that do not have strict time requirements can be placed 
*				 in this function, such as key scanning, buzzer control, etc.
*	Parameters: None
*	Return Value: None
*********************************************************************************************************
*/
void bsp_RunPer10ms(void)
{
//	bsp_KeyScan10ms();
}

/*
*********************************************************************************************************
*	Function Name: bsp_RunPer1ms
*	Description: This function is called once every 1ms by the Systick interrupt. See the timer interrupt 
*				 service routine in bsp_timer.c. Tasks that require periodic processing can be placed 
*				 in this function, such as touch coordinate scanning.
*	Parameters: None
*	Return Value: None
*********************************************************************************************************
*/
void bsp_RunPer1ms(void)
{
	
}

/*
*********************************************************************************************************
*	Function Name: bsp_Idle
*	Description: Function executed during idle time. Typically, the main program needs to insert the 
*				 CPU_IDLE() macro in the body of for and while loops to call this function.
*				 By default, this function performs no operation. Users can add functionality such as 
*				 watchdog feeding or setting the CPU to enter sleep mode.
*	Parameters: None
*	Return Value: None
*********************************************************************************************************
*/
void bsp_Idle(void)
{
	/* --- Feed the watchdog */

	/* --- Put the CPU into sleep mode, to be awakened by Systick timer interrupt or other interrupts */

	/* For example, for the emWin graphics library, you can insert the polling function required by the graphics library */
	//GUI_Exec();

	/* For example, for the uIP protocol, you can insert the uIP polling function */
	//TOUCH_CapScan();
}

/*
*********************************************************************************************************
*	Function Name: HAL_Delay
*	Description: Redirect the millisecond delay function. Replaces the function in HAL. The default function 
*                 in HAL relies on the Systick interrupt, which can cause a deadlock if there is a delay 
*                 function in USB or SD card interrupts. Alternatively, the Systick interrupt priority can 
*                 be raised using the HAL_NVIC_SetPriority function.
*	Parameters: None
*	Return Value: None
*********************************************************************************************************
*/
/* The current example uses the default implementation in stm32f4xx_hal.c and does not use the redirected function below */
#if 0
void HAL_Delay(uint32_t Delay)
{
	bsp_DelayUS(Delay * 1000);
}
#endif

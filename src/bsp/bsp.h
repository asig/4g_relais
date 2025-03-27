/*
*********************************************************************************************************
*
*	Module Name : BSP Module (For STM32F429)
*	File Name   : bsp.h
*	Version     : V1.0
*	Description : This file is the hardware abstraction layer for the board. Each C file should include 
*	              "bsp.h" to access all the hardware-related modules.
*	              bsp = Board Support Package
*	Revision History :
*		Version   Date         Author      Description
*		V1.0      2018-07-29   Eric2013    Initial release
*
*	Copyright (C), 2018-2030, Armfly Electronics www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H_

#define STM32_V6    


/* Check if the port model is defined */
#if !defined (STM32_V6)
	#error "Please define the board model : STM32_V6"
#endif

/* Define BSP version */
#define __STM32H7_BSP_VERSION		"1.1"

/* Function executed during CPU idle time */
//#define CPU_IDLE()		bsp_Idle()

/* Global interrupt control macros */
#define ENABLE_INT()	__set_PRIMASK(0)	/* Enable global interrupts */
#define DISABLE_INT()	__set_PRIMASK(1)	/* Disable global interrupts */

/* Code for debugging during the development phase */
#define BSP_Printf		printf
//#define BSP_Printf(...)

#define EXTI9_5_ISR_MOVE_OUT	/* bsp.h contains declarations to avoid redefinition errors in stam32f4xx_it.c and other files */

#define ERROR_HANDLER()		Error_Handler(__FILE__, __LINE__);

/* Default is off state */
#define  Enable_EventRecorder  0

#if Enable_EventRecorder == 1
	#include "EventRecorder.h"
#endif

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

/* Interrupt priority configuration */
#define NVIC_PREEMPT_PRIORITY	4

/* Uncomment the following line to include the message module */
//#include "bsp_msg.h"
//#include "bsp_user_lib.h"
#include "bsp_timer.h"
#include "bsp_led.h"
#include "bsp_key.h"

//#include "bsp_cpu_rtc.h"
//#include "bsp_cpu_adc.h"
//#include "bsp_cpu_dac.h"
#include "bsp_uart_fifo.h"
//#include "bsp_uart_gps.h"
//#include "bsp_uart_esp8266.h"
//#include "bsp_uart_sim800.h"

//#include "bsp_spi_bus.h"
//#include "bsp_spi_ad9833.h"
//#include "bsp_spi_ads1256.h"
//#include "bsp_spi_dac8501.h"
//#include "bsp_spi_dac8562.h"
//#include "bsp_spi_flash.h"
//#include "bsp_spi_tm7705.h"
//#include "bsp_spi_vs1053b.h"

//#include "bsp_fmc_sdram.h"
//#include "bsp_fmc_nand_flash.h"
//#include "bsp_fmc_ad7606.h"
//#include "bsp_fmc_oled.h"
//#include "bsp_fmc_io.h"

//#include "bsp_i2c_gpio.h"
//#include "bsp_i2c_bh1750.h"
//#include "bsp_i2c_bmp085.h"
//#include "bsp_i2c_eeprom_24xx.h"
//#include "bsp_i2c_hmc5883l.h"
//#include "bsp_i2c_mpu6050.h"
//#include "bsp_i2c_si4730.h"
//#include "bsp_i2c_wm8978.h"

//#include "bsp_tft_h7.h"
//#include "bsp_tft_lcd.h"
//#include "bsp_ts_touch.h"
//#include "bsp_ts_ft5x06.h"
//#include "bsp_ts_gt811.h"
//#include "bsp_ts_gt911.h"
//#include "bsp_ts_stmpe811.h"

//#include "bsp_beep.h"
//#include "bsp_tim_pwm.h"
//#include "bsp_sdio_sd.h"
//#include "bsp_dht11.h"
//#include "bsp_ds18b20.h"
//#include "bsp_ps2.h"
//#include "bsp_ir_decode.h"
//#include "bsp_camera.h"
//#include "bsp_rs485_led.h"
//#include "bsp_can.h"

/* Provide functions used by other C files */
void bsp_Init(void);
void bsp_Idle(void);

void bsp_GetCpuID(uint32_t *_id);
void Error_Handler(char *file, uint32_t line);

#endif


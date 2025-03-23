/*
*********************************************************************************************************
*
*	Module Name : LED Indicator Driver Module
*	File Name   : bsp_led.h
*	Version     : V1.0
*	Description : Header File
*
*	Copyright (C), 2013-2014, Anfu Lai Electronics www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_LED_H
#define __BSP_LED_H

/* Function declarations for external use */
void bsp_InitLed(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
uint8_t bsp_IsLedOn(uint8_t _no);

#endif

/***************************** Anfu Lai Electronics www.armfly.com (END OF FILE) *********************************/

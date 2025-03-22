/*
*********************************************************************************************************
*
*	Module Name : Data Watchpoint and Trace (DWT) Module
*	File Name   : bsp_dwt.h
*	Version     : V1.0
*	Description : Header File
*	Revision History :
*		Version    Date         Author    Description
*		V1.0    2015-08-18   Eric2013  Initial Release
*
*	Copyright (C), 2015-2020, Anfu Lai Electronics www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_DWT_H
#define __BSP_DWT_H

/*
*********************************************************************************************************
*                                             寄存器
*********************************************************************************************************
*/
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)


/*
*********************************************************************************************************
*                                             函数
*********************************************************************************************************
*/
void bsp_InitDWT(void);
void bsp_DelayUS(uint32_t _ulDelayTime);
void bsp_DelayMS(uint32_t _ulDelayTime);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

#ifndef __AIR780E_H
#define __AIR780E_H

#include "main.h"

#define AIR780E_BOOT_GPIO_CLK_ENABLE()    	__HAL_RCC_GPIOC_CLK_ENABLE()
#define AIR780E_BOOT_PORT              		GPIOC
#define AIR780E_BOOT_PIN                    GPIO_PIN_13

void Air780e_NITZ(void);
void Air780e_Online(void);
uint8_t  Is_Air780e_Online();

#endif


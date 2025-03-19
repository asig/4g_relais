/**
  ******************************************************************************
  * @file    Templates/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#include "bsp.h"			/* Low-level hardware driver */
#include "AIR780E.h"


#define RELAY_GPIO_CLK_ENABLE()    	__HAL_RCC_GPIOB_CLK_ENABLE()
#define RELAY_PORT                	GPIOB
#define RELAY_PIN     				GPIO_PIN_9
#define RELAY1_PIN     				GPIO_PIN_8
#define RELAY_ON					HAL_GPIO_WritePin(RELAY_PORT,RELAY_PIN,1)
#define RELAY_OFF					HAL_GPIO_WritePin(RELAY_PORT,RELAY_PIN,0)
#define RELAY1_ON					HAL_GPIO_WritePin(RELAY_PORT,RELAY1_PIN,1)
#define RELAY1_OFF				HAL_GPIO_WritePin(RELAY_PORT,RELAY1_PIN,0)
#define RELAY_TOG					HAL_GPIO_TogglePin(RELAY_PORT,RELAY_PIN)
#define RELAY1_TOG				HAL_GPIO_TogglePin(RELAY_PORT,RELAY1_PIN)



#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*
*********************************************************************************************************
*
*	Module Name : UART Interrupt + FIFO Buffer Module
*	File Name   : bsp_uart_fifo.h
*	Description : Header File
*
*	Copyright (C), 2015-2020, Armfly Electronics www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_USART_FIFO_H_
#define _BSP_USART_FIFO_H_

#include "bsp.h"

/*
	STM32-V7 UART distribution:
	Channel 1: RS232 chip, 1 channel
		PA9/USART1_TX    --- Debug port
		PA10/USART1_RX

	Channel 2: PA2 universal IO pin used for Beidou satellite RX to receive GPS signals
		PA2/USART2_TX/ETH_MDIO (used for Beidou satellite transmission)
		PA3/USART2_RX    ; GPS module reception

	Channel 3: RS485 communication - TTL level to RS485
		PB10/USART3_TX
		PB11/USART3_RX

	Channel 4: --- Reserved for external expansion, SD card occupied
	Channel 5: --- Reserved for external expansion, SD card occupied

	Channel 6: --- GPRS module or WIFI module (ESP8266)
		PC6/USART6_TX
		PC7/USART6_RX
		
	Channel 7: --- Reserved for external expansion, SPI3 occupied
	
	Channel 8: --- Reserved for external expansion, LTDC display interface
*/


#define	UART1_FIFO_EN	1
#define	UART2_FIFO_EN	1
#define	UART3_FIFO_EN	0
#define	UART4_FIFO_EN	0
#define	UART5_FIFO_EN	0
#define	UART6_FIFO_EN	0
#define	UART7_FIFO_EN	0
#define	UART8_FIFO_EN	0

/* PB2 is used for RS485 transceiver control */
#define RS485_TXEN_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define RS485_TXEN_GPIO_PORT              GPIOB
#define RS485_TXEN_PIN                    GPIO_PIN_2

#define RS485_RX_EN()	RS485_TXEN_GPIO_PORT->BSRR = (RS485_TXEN_PIN<<16)
#define RS485_TX_EN()	RS485_TXEN_GPIO_PORT->BSRR = RS485_TXEN_PIN

/* ����˿ں� */
typedef enum
{
	COM1 = 0,	/* USART1 */
	COM2 = 1,	/* USART2 */
	COM3 = 2,	/* USART3 */
	COM4 = 3,	/* UART4 */
	COM5 = 4,	/* UART5 */
	COM6 = 5,	/* USART6 */
	COM7 = 6,	/* UART7 */	
	COM8 = 7	/* UART8 */	
}COM_PORT_E;

/* Configure internal buffer sizes for FIFO. The sizes are defined for both transmit and receive buffers, supporting full-duplex communication. */
#if UART1_FIFO_EN == 1
	#define UART1_BAUD			115200
	#define UART1_TX_BUF_SIZE	1*1024
	#define UART1_RX_BUF_SIZE	1*1024
#endif

#if UART2_FIFO_EN == 1
	#define UART2_BAUD			115200
	#define UART2_TX_BUF_SIZE	1*1024
	#define UART2_RX_BUF_SIZE	1*1024
#endif

#if UART3_FIFO_EN == 1
	#define UART3_BAUD			9600
	#define UART3_TX_BUF_SIZE	1*1024
	#define UART3_RX_BUF_SIZE	1*1024
#endif

#if UART4_FIFO_EN == 1
	#define UART4_BAUD			115200
	#define UART4_TX_BUF_SIZE	1*1024
	#define UART4_RX_BUF_SIZE	1*1024
#endif

#if UART5_FIFO_EN == 1
	#define UART5_BAUD			115200
	#define UART5_TX_BUF_SIZE	1*1024
	#define UART5_RX_BUF_SIZE	1*1024
#endif

#if UART6_FIFO_EN == 1
	#define UART6_BAUD			115200
	#define UART6_TX_BUF_SIZE	1*1024
	#define UART6_RX_BUF_SIZE	1*1024
#endif

#if UART7_FIFO_EN == 1
	#define UART7_BAUD			115200
	#define UART7_TX_BUF_SIZE	1*1024
	#define UART7_RX_BUF_SIZE	1*1024
#endif

#if UART8_FIFO_EN == 1
	#define UART8_BAUD			115200
	#define UART8_TX_BUF_SIZE	1*1024
	#define UART8_RX_BUF_SIZE	1*1024
#endif

/* UART device structure */
typedef struct
{
	USART_TypeDef *uart;		/* Pointer to the internal STM32 UART device */
	uint8_t *pTxBuf;			/* Transmit buffer */
	uint8_t *pRxBuf;			/* Receive buffer */
	uint16_t usTxBufSize;		/* Transmit buffer size */
	uint16_t usRxBufSize;		/* Receive buffer size */
	__IO uint16_t usTxWrite;	/* Transmit buffer write pointer */
	__IO uint16_t usTxRead;		/* Transmit buffer read pointer */
	__IO uint16_t usTxCount;	/* Count of data waiting to be transmitted */

	__IO uint16_t usRxWrite;	/* Receive buffer write pointer */
	__IO uint16_t usRxRead;		/* Receive buffer read pointer */
	__IO uint16_t usRxCount;	/* Count of unread received data */

	void (*SendBefor)(void); 	/* Callback function pointer before starting transmission (e.g., for RS485 switching to transmit mode) */
	void (*SendOver)(void); 	/* Callback function pointer after transmission is complete (e.g., for RS485 switching back to receive mode) */
	void (*ReciveNew)(uint8_t _byte);	/* Callback function pointer for handling newly received data */
	uint8_t Sending;			/* Indicates if data is currently being transmitted */
}UART_T;

void bsp_InitUart(void);
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte);
uint8_t comGetChar(COM_PORT_E _ucPort, uint8_t *_pByte);
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comClearTxFifo(COM_PORT_E _ucPort);
void comClearRxFifo(COM_PORT_E _ucPort);
void comSetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate);

void USART_SetBaudRate(USART_TypeDef* USARTx, uint32_t BaudRate);
void bsp_SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode);

void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen);
void RS485_SendStr(char *_pBuf);
void RS485_SetBaud(uint32_t _baud);
uint8_t UartTxEmpty(COM_PORT_E _ucPort);

#endif


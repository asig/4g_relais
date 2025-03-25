/*
*********************************************************************************************************
*
*	Module Name : UART Interrupt + FIFO Driver Module
*	File Name   : bsp_uart_fifo.c
*	Version     : V1.8
*	Description : Implements simultaneous access to multiple UARTs using interrupt + FIFO mode
*	Change Log  :
*		Version   Date       Author    Description
*		V1.0      2013-02-01 armfly    Initial release
*		V1.1      2013-06-09 armfly    Added TxCount member to FIFO structure for easier buffer full detection;
*		                               Added functions to clear FIFO
*		V1.2      2014-09-29 armfly    Added RS485 MODBUS interface. Executes callback function directly upon receiving a new byte.
*		V1.3      2015-07-23 armfly    Added __IO modifier to several read/write pointer members of UART_T structure to prevent
*		                               deadlock in UART send function due to compiler optimization.
*		V1.4      2015-08-04 armfly    Fixed UART4 configuration bug: GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART1);
*		V1.5      2015-10-08 armfly    Added interface function to modify baud rate
*		V1.6      2018-09-07 armfly    Ported to STM32H7 platform
*		V1.7      2018-10-01 armfly    Added Sending flag to indicate ongoing transmission
*		V1.8      2018-11-26 armfly    Added UART8, the 8th UART
*
*	Copyright (C), 2015-2030, Anfu Lai Electronics www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/* UART1 GPIO PA9, PA10 RS232 DB9 interface */
#define USART1_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()

#define USART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART1_TX_GPIO_PORT              GPIOA
#define USART1_TX_PIN                    GPIO_PIN_9
#define USART1_TX_AF                     GPIO_AF7_USART1

#define USART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART1_RX_GPIO_PORT              GPIOA
#define USART1_RX_PIN                    GPIO_PIN_10
#define USART1_RX_AF                     GPIO_AF7_USART1

/* UART2 GPIO --- PA2 PA3 GPS (only RX is used. TX is occupied by Ethernet) */
#define USART2_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()

#define USART2_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_TX_GPIO_PORT              GPIOA
#define USART2_TX_PIN                    GPIO_PIN_2
#define USART2_TX_AF                     GPIO_AF7_USART2

#define USART2_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_RX_GPIO_PORT              GPIOA
#define USART2_RX_PIN                    GPIO_PIN_3
#define USART2_RX_AF                     GPIO_AF7_USART2

/* UART3 GPIO --- PB10 PB11 RS485 */
#define USART3_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE()

#define USART3_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART3_TX_GPIO_PORT              GPIOB
#define USART3_TX_PIN                    GPIO_PIN_10
#define USART3_TX_AF                     GPIO_AF7_USART3

#define USART3_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART3_RX_GPIO_PORT              GPIOB
#define USART3_RX_PIN                    GPIO_PIN_11
#define USART3_RX_AF                     GPIO_AF7_USART3

/* UART4 GPIO --- PC10 PC11 occupied by SD card */
#define UART4_CLK_ENABLE()              __HAL_RCC_UART4_CLK_ENABLE()

#define UART4_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART4_TX_GPIO_PORT              GPIOC
#define UART4_TX_PIN                    GPIO_PIN_10
#define UART4_TX_AF                     GPIO_AF8_UART4

#define UART4_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART4_RX_GPIO_PORT              GPIOC
#define UART4_RX_PIN                    GPIO_PIN_11
#define UART4_RX_AF                     GPIO_AF8_UART4

/* UART5 GPIO --- PC12/UART5_TX PD2/UART5_RX (occupied by SD card) */
#define UART5_CLK_ENABLE()              __HAL_RCC_UART5_CLK_ENABLE()

#define UART5_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART5_TX_GPIO_PORT              GPIOC
#define UART5_TX_PIN                    GPIO_PIN_12
#define UART5_TX_AF                     GPIO_AF8_UART5

#define UART5_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define UART5_RX_GPIO_PORT              GPIOD
#define UART5_RX_PIN                    GPIO_PIN_2
#define UART5_RX_AF                     GPIO_AF8_UART5

/* UART6 GPIO --- PG14 PC7 GPRS */
#define USART6_CLK_ENABLE()              __HAL_RCC_USART6_CLK_ENABLE()

#define USART6_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define USART6_TX_GPIO_PORT              GPIOG
#define USART6_TX_PIN                    GPIO_PIN_14
#define USART6_TX_AF                     GPIO_AF7_USART6

#define USART6_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define USART6_RX_GPIO_PORT              GPIOC
#define USART6_RX_PIN                    GPIO_PIN_7
#define USART6_RX_AF                     GPIO_AF7_USART6

/* UART7 GPIO --- PB4/UART7_TX, PB3/UART7_RX (occupied by SPI3) */
#define UART7_CLK_ENABLE()              __HAL_RCC_UART7_CLK_ENABLE()

#define UART7_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define UART7_TX_GPIO_PORT              GPIOB
#define UART7_TX_PIN                    GPIO_PIN_4
#define UART7_TX_AF                     GPIO_AF11_UART7

#define UART7_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define UART7_RX_GPIO_PORT              GPIOB
#define UART7_RX_PIN                    GPIO_PIN_3
#define UART7_RX_AF                     GPIO_AF11_UART7

/* UART8 GPIO --- PJ8/UART8_TX, PJ9/UART8_RX (occupied by RGB hardware interface) */
#define UART8_CLK_ENABLE()              __HAL_RCC_UART8_CLK_ENABLE()

#define UART8_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOJ_CLK_ENABLE()
#define UART8_TX_GPIO_PORT              GPIOJ
#define UART8_TX_PIN                    GPIO_PIN_8
#define UART8_TX_AF                     GPIO_AF8_UART8

#define UART8_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOJ_CLK_ENABLE()
#define UART8_RX_GPIO_PORT              GPIOJ
#define UART8_RX_PIN                    GPIO_PIN_9
#define UART8_RX_AF                     GPIO_AF8_UART8

/* Define structure variables for each UART */
#if UART1_FIFO_EN == 1
	static UART_T g_tUart1;
	static uint8_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* Transmit buffer */
	static uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* Receive buffer */
#endif

#if UART2_FIFO_EN == 1
	static UART_T g_tUart2;
	static uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];		/* Transmit buffer */
	static uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];		/* Receive buffer */
#endif

#if UART3_FIFO_EN == 1
	static UART_T g_tUart3;
	static uint8_t g_TxBuf3[UART3_TX_BUF_SIZE];		/* Transmit buffer */
	static uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];		/* Receive buffer */
#endif

#if UART4_FIFO_EN == 1
	static UART_T g_tUart4;
	static uint8_t g_TxBuf4[UART4_TX_BUF_SIZE];		/* Transmit buffer */
	static uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];		/* Receive buffer */
#endif

#if UART5_FIFO_EN == 1
	static UART_T g_tUart5;
	static uint8_t g_TxBuf5[UART5_TX_BUF_SIZE];		/* Transmit buffer */
	static uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];		/* Receive buffer */
#endif

#if UART6_FIFO_EN == 1
	static UART_T g_tUart6;
	static uint8_t g_TxBuf6[UART6_TX_BUF_SIZE];		/* Transmit buffer */
	static uint8_t g_RxBuf6[UART6_RX_BUF_SIZE];		/* Receive buffer */
#endif

#if UART7_FIFO_EN == 1
	static UART_T g_tUart7;
	static uint8_t g_TxBuf7[UART7_TX_BUF_SIZE];		/* Transmit buffer */
	static uint8_t g_RxBuf7[UART7_RX_BUF_SIZE];		/* Receive buffer */
#endif

#if UART8_FIFO_EN == 1
	static UART_T g_tUart8;
	static uint8_t g_TxBuf8[UART8_TX_BUF_SIZE];		/* Transmit buffer */
	static uint8_t g_RxBuf8[UART8_RX_BUF_SIZE];		/* Receive buffer */
#endif
		
static void UartVarInit(void);

static void InitHardUart(void);
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);
static void UartIRQ(UART_T *_pUart);

void RS485_InitTXE(void);
/*
*********************************************************************************************************
*	Function Name: bsp_InitUart
*	Description  : Initialize UART hardware and assign initial values to global variables.
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
	UartVarInit();		/* Must initialize global variables first, then configure hardware */

	InitHardUart();		/* Configure UART hardware parameters (baud rate, etc.) */

	RS485_InitTXE();	/* Configure RS485 chip's transmit enable hardware, set as push-pull output */
}

/*
*********************************************************************************************************
*	Function Name: ComToUart
*	Description  : Converts a COM port number to a UART pointer
*	Parameters   : _ucPort: Port number (COM1 - COM8)
*	Return Value : UART pointer
*********************************************************************************************************
*/
UART_T *ComToUart(COM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
		#if UART1_FIFO_EN == 1
			return &g_tUart1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM2)
	{
		#if UART2_FIFO_EN == 1
			return &g_tUart2;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM3)
	{
		#if UART3_FIFO_EN == 1
			return &g_tUart3;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM4)
	{
		#if UART4_FIFO_EN == 1
			return &g_tUart4;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM5)
	{
		#if UART5_FIFO_EN == 1
			return &g_tUart5;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM6)
	{
		#if UART6_FIFO_EN == 1
			return &g_tUart6;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM7)
	{
		#if UART7_FIFO_EN == 1
			return &g_tUart7;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM8)
	{
		#if UART8_FIFO_EN == 1
			return &g_tUart8;
		#else
			return 0;
		#endif
	}	
	else
	{
		Error_Handler(__FILE__, __LINE__);
		return 0;
	}
}

/*
*********************************************************************************************************
*	Function Name: ComToUart
*	Description  : Converts COM port number to USART_TypeDef* USARTx
*	Parameters   : _ucPort: Port number (COM1 - COM8)
*	Return Value : USART_TypeDef*, USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8.
*********************************************************************************************************
*/
USART_TypeDef *ComToUSARTx(COM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
		#if UART1_FIFO_EN == 1
			return USART1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM2)
	{
		#if UART2_FIFO_EN == 1
			return USART2;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM3)
	{
		#if UART3_FIFO_EN == 1
			return USART3;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM4)
	{
		#if UART4_FIFO_EN == 1
			return UART4;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM5)
	{
		#if UART5_FIFO_EN == 1
			return UART5;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM6)
	{
		#if UART6_FIFO_EN == 1
			return USART6;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM7)
	{
		#if UART7_FIFO_EN == 1
			return UART7;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM8)
	{
		#if UART8_FIFO_EN == 1
			return UART8;
		#else
			return 0;
		#endif
	}	
	
	else
	{
		/* Do nothing */
		return 0;
	}
}

/*
*********************************************************************************************************
*	Function Name: comSendBuf
*	Description  : Sends a group of data to the serial port. The data is placed in the transmit buffer 
*	               and returns immediately. The interrupt service routine completes the transmission in the background.
*	Parameters   : _ucPort: Port number (COM1 - COM8)
*	              _ucaBuf: Data buffer to be sent
*	              _usLen : Length of the data
*	Return Value : None
*********************************************************************************************************
*/
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	if (pUart->SendBefor != 0)
	{
		pUart->SendBefor();		/* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
	}

	UartSend(pUart, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	函 数 名: comSendChar
*	功能说明: 向串口发送1个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _ucByte: 待发送的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte)
{
	comSendBuf(_ucPort, &_ucByte, 1);
}

/*
*********************************************************************************************************
*	函 数 名: comGetChar
*	功能说明: 从接收缓冲区读取1字节，非阻塞。无论有无数据均立即返回。
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _pByte: 接收到的数据存放在这个地址
*	返 回 值: 0 表示无数据, 1 表示读取到有效字节
*********************************************************************************************************
*/
uint8_t comGetChar(COM_PORT_E _ucPort, uint8_t *_pByte)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return 0;
	}

	return UartGetChar(pUart, _pByte);
}

/*
*********************************************************************************************************
*	函 数 名: comClearTxFifo
*	功能说明: 清零串口发送缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearTxFifo(COM_PORT_E _ucPort)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usTxWrite = 0;
	pUart->usTxRead = 0;
	pUart->usTxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: comClearRxFifo
*	功能说明: 清零串口接收缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM8)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearRxFifo(COM_PORT_E _ucPort)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usRxWrite = 0;
	pUart->usRxRead = 0;
	pUart->usRxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: comSetBaud
*	功能说明: 设置串口的波特率. 本函数固定设置为无校验，收发都使能模式
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _BaudRate: 波特率，8倍过采样  波特率.0-12.5Mbps
*                                16倍过采样 波特率.0-6.25Mbps
*	返 回 值: 无
*********************************************************************************************************
*/
void comSetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate)
{
	USART_TypeDef* USARTx;
	
	USARTx = ComToUSARTx(_ucPort);
	if (USARTx == 0)
	{
		return;
	}
	
	bsp_SetUartParam(USARTx,  _BaudRate, UART_PARITY_NONE, UART_MODE_TX_RX);
}

/* 如果是RS485通信，请按如下格式编写函数， 我们仅举了 USART3作为RS485的例子 */

/*
*********************************************************************************************************
*	函 数 名: RS485_InitTXE
*	功能说明: 配置RS485发送使能口线 TXE
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_InitTXE(void)
{
	GPIO_InitTypeDef gpio_init;
	
	/* 打开GPIO时钟 */
	RS485_TXEN_GPIO_CLK_ENABLE();
	
	/* 配置引脚为推挽输出 */
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	gpio_init.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */
	gpio_init.Pin = RS485_TXEN_PIN;
	HAL_GPIO_Init(RS485_TXEN_GPIO_PORT, &gpio_init);	
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SetBaud
*	功能说明: 修改485串口的波特率。
*	形    参: _baud : 8倍过采样  波特率.0-12.5Mbps
*                     16倍过采样 波特率.0-6.25Mbps
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SetBaud(uint32_t _baud)
{
	comSetBaud(COM3, _baud);
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendBefor
*	功能说明: 发送数据前的准备工作。对于RS485通信，请设置RS485芯片为发送状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendBefor = RS485_SendBefor
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendBefor(void)
{
	RS485_TX_EN();	/* 切换RS485收发芯片为发送模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendOver
*	功能说明: 发送一串数据结束后的善后处理。对于RS485通信，请设置RS485芯片为接收状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendOver = RS485_SendOver
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendOver(void)
{
	RS485_RX_EN();	/* 切换RS485收发芯片为接收模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendBuf
*	功能说明: 通过RS485芯片发送一串数据。注意，本函数不等待发送完毕。
*	形    参: _ucaBuf : 数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen)
{
	comSendBuf(COM3, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendStr
*	功能说明: 向485总线发送一个字符串，0结束。
*	形    参: _pBuf 字符串，0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendStr(char *_pBuf)
{
	RS485_SendBuf((uint8_t *)_pBuf, strlen(_pBuf));
}

/*
*********************************************************************************************************
*	函 数 名: RS485_ReciveNew
*	功能说明: 接收到新的数据
*	形    参: _byte 接收到的新数据
*	返 回 值: 无
*********************************************************************************************************
*/
//extern void MODH_ReciveNew(uint8_t _byte);
void RS485_ReciveNew(uint8_t _byte)
{
//	MODH_ReciveNew(_byte);
}
/*
*********************************************************************************************************
*	Function Name: UartVarInit
*	Description  : Initialize variables related to UART
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
static void UartVarInit(void)
{
#if UART1_FIFO_EN == 1
	g_tUart1.uart = USART1;						/* STM32 UART device */
	g_tUart1.pTxBuf = g_TxBuf1;					/* Transmit buffer pointer */
	g_tUart1.pRxBuf = g_RxBuf1;					/* Receive buffer pointer */
	g_tUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* Transmit buffer size */
	g_tUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* Receive buffer size */
	g_tUart1.usTxWrite = 0;						/* Transmit FIFO write index */
	g_tUart1.usTxRead = 0;						/* Transmit FIFO read index */
	g_tUart1.usRxWrite = 0;						/* Receive FIFO write index */
	g_tUart1.usRxRead = 0;						/* Receive FIFO read index */
	g_tUart1.usRxCount = 0;						/* Number of new received data */
	g_tUart1.usTxCount = 0;						/* Number of data to be sent */
	g_tUart1.SendBefor = 0;						/* Callback function before sending data */
	g_tUart1.SendOver = 0;						/* Callback function after sending is complete */
	g_tUart1.ReciveNew = 0;						/* Callback function when new data is received */
	g_tUart1.Sending = 0;						/* Flag indicating ongoing transmission */
#endif

#if UART2_FIFO_EN == 1
	g_tUart2.uart = USART2;						/* STM32 UART device */
	g_tUart2.pTxBuf = g_TxBuf2;					/* Transmit buffer pointer */
	g_tUart2.pRxBuf = g_RxBuf2;					/* Receive buffer pointer */
	g_tUart2.usTxBufSize = UART2_TX_BUF_SIZE;	/* Transmit buffer size */
	g_tUart2.usRxBufSize = UART2_RX_BUF_SIZE;	/* Receive buffer size */
	g_tUart2.usTxWrite = 0;						/* Transmit FIFO write index */
	g_tUart2.usTxRead = 0;						/* Transmit FIFO read index */
	g_tUart2.usRxWrite = 0;						/* Receive FIFO write index */
	g_tUart2.usRxRead = 0;						/* Receive FIFO read index */
	g_tUart2.usRxCount = 0;						/* Number of new received data */
	g_tUart2.usTxCount = 0;						/* Number of data to be sent */
	g_tUart2.SendBefor = 0;						/* Callback function before sending data */
	g_tUart2.SendOver = 0;						/* Callback function after sending is complete */
	g_tUart2.ReciveNew = 0;						/* Callback function when new data is received */
	g_tUart2.Sending = 0;						/* Flag indicating ongoing transmission */
#endif

#if UART3_FIFO_EN == 1
	g_tUart3.uart = USART3;						/* STM32 UART device */
	g_tUart3.pTxBuf = g_TxBuf3;					/* Transmit buffer pointer */
	g_tUart3.pRxBuf = g_RxBuf3;					/* Receive buffer pointer */
	g_tUart3.usTxBufSize = UART3_TX_BUF_SIZE;	/* Transmit buffer size */
	g_tUart3.usRxBufSize = UART3_RX_BUF_SIZE;	/* Receive buffer size */
	g_tUart3.usTxWrite = 0;						/* Transmit FIFO write index */
	g_tUart3.usTxRead = 0;						/* Transmit FIFO read index */
	g_tUart3.usRxWrite = 0;						/* Receive FIFO write index */
	g_tUart3.usRxRead = 0;						/* Receive FIFO read index */
	g_tUart3.usRxCount = 0;						/* Number of new received data */
	g_tUart3.usTxCount = 0;						/* Number of data to be sent */
	g_tUart3.SendBefor = RS485_SendBefor;		/* Callback function before sending data */
	g_tUart3.SendOver = RS485_SendOver;			/* Callback function after sending is complete */
	g_tUart3.ReciveNew = RS485_ReciveNew;		/* Callback function when new data is received */
	g_tUart3.Sending = 0;						/* Flag indicating ongoing transmission */
#endif

#if UART4_FIFO_EN == 1
	g_tUart4.uart = UART4;						/* STM32 UART device */
	g_tUart4.pTxBuf = g_TxBuf4;					/* Transmit buffer pointer */
	g_tUart4.pRxBuf = g_RxBuf4;					/* Receive buffer pointer */
	g_tUart4.usTxBufSize = UART4_TX_BUF_SIZE;	/* Transmit buffer size */
	g_tUart4.usRxBufSize = UART4_RX_BUF_SIZE;	/* Receive buffer size */
	g_tUart4.usTxWrite = 0;						/* Transmit FIFO write index */
	g_tUart4.usTxRead = 0;						/* Transmit FIFO read index */
	g_tUart4.usRxWrite = 0;						/* Receive FIFO write index */
	g_tUart4.usRxRead = 0;						/* Receive FIFO read index */
	g_tUart4.usRxCount = 0;						/* Number of new received data */
	g_tUart4.usTxCount = 0;						/* Number of data to be sent */
	g_tUart4.SendBefor = 0;						/* Callback function before sending data */
	g_tUart4.SendOver = 0;						/* Callback function after sending is complete */
	g_tUart4.ReciveNew = 0;						/* Callback function when new data is received */
	g_tUart4.Sending = 0;						/* Flag indicating ongoing transmission */
#endif

#if UART5_FIFO_EN == 1
	g_tUart5.uart = UART5;						/* STM32 串口设备 */
	g_tUart5.pTxBuf = g_TxBuf5;					/* 发送缓冲区指针 */
	g_tUart5.pRxBuf = g_RxBuf5;					/* 接收缓冲区指针 */
	g_tUart5.usTxBufSize = UART5_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart5.usRxBufSize = UART5_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart5.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart5.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart5.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart5.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart5.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart5.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart5.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart5.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart5.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart5.Sending = 0;						/* 正在发送中标志 */
#endif


#if UART6_FIFO_EN == 1
	g_tUart6.uart = USART6;						/* STM32 串口设备 */
	g_tUart6.pTxBuf = g_TxBuf6;					/* 发送缓冲区指针 */
	g_tUart6.pRxBuf = g_RxBuf6;					/* 接收缓冲区指针 */
	g_tUart6.usTxBufSize = UART6_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart6.usRxBufSize = UART6_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart6.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart6.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart6.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart6.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart6.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart6.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart6.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart6.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart6.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart6.Sending = 0;						/* 正在发送中标志 */
#endif

#if UART7_FIFO_EN == 1
	g_tUart7.uart = UART7;						/* STM32 串口设备 */
	g_tUart7.pTxBuf = g_TxBuf7;					/* 发送缓冲区指针 */
	g_tUart7.pRxBuf = g_RxBuf7;					/* 接收缓冲区指针 */
	g_tUart7.usTxBufSize = UART7_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart7.usRxBufSize = UART7_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart7.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart7.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart7.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart7.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart7.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart7.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart7.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart7.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart7.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart7.Sending = 0;						/* 正在发送中标志 */
#endif

#if UART8_FIFO_EN == 1
	g_tUart8.uart = UART8;						/* STM32 串口设备 */
	g_tUart8.pTxBuf = g_TxBuf8;					/* 发送缓冲区指针 */
	g_tUart8.pRxBuf = g_RxBuf8;					/* 接收缓冲区指针 */
	g_tUart8.usTxBufSize = UART8_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart8.usRxBufSize = UART8_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart8.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart8.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart8.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart8.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart8.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart8.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart8.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart8.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart8.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_tUart8.Sending = 0;						/* 正在发送中标志 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetUartParam
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32- H7开发板
*	形    参: Instance   USART_TypeDef类型结构体
*             BaudRate   波特率
*             Parity     校验类型，奇校验或者偶校验
*             Mode       发送和接收模式使能
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode)
{
	UART_HandleTypeDef UartHandle;	
	
	/*##-1- 配置串口硬件参数 ######################################*/
	/* 异步串口模式 (UART Mode) */
	/* 配置如下:
	  - 字长    = 8 位
	  - 停止位  = 1 个停止位
	  - 校验    = 参数Parity
	  - 波特率  = 参数BaudRate
	  - 硬件流控制关闭 (RTS and CTS signals) */

	UartHandle.Instance        = Instance;

	UartHandle.Init.BaudRate   = BaudRate;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = Parity;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = Mode;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
	if (HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
}

/*
*********************************************************************************************************
*	Function Name: InitHardUart
*	Description  : Configure UART hardware parameters (baud rate, data bits, stop bits, start bits, 
*	               parity, interrupt enable) suitable for STM32-H7 development board.
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
static void InitHardUart(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	#if UART1_FIFO_EN == 1		/* UART1 */
	
		/* Enable GPIO TX/RX clock */
		USART1_TX_GPIO_CLK_ENABLE();
		USART1_RX_GPIO_CLK_ENABLE();
		
		/* Enable USARTx clock */
		USART1_CLK_ENABLE();	

		/* Configure TX pin */
		GPIO_InitStruct.Pin       = USART1_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	//	GPIO_InitStruct.Alternate = USART1_TX_AF;
		HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* Configure RX pin */
		GPIO_InitStruct.Pin = USART1_RX_PIN;
	//	GPIO_InitStruct.Alternate = USART1_RX_AF;
		HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);

		/* Configure NVIC for UART */   
		HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
	  
		/* Configure baud rate and parity */
		bsp_SetUartParam(USART1,  UART1_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

		CLEAR_BIT(USART1->SR, USART_SR_TC);   /* Clear TC (Transmission Complete) flag */
		CLEAR_BIT(USART1->SR, USART_SR_RXNE); /* Clear RXNE (Receive Not Empty) flag */
		// USART_CR1_PEIE | USART_CR1_RXNEIE
		SET_BIT(USART1->CR1, USART_CR1_RXNEIE);	/* Enable RX interrupt */
	#endif
	#if UART2_FIFO_EN == 1		/* UART2 */
		/* Enable GPIO TX/RX clock */
		USART2_TX_GPIO_CLK_ENABLE();
		USART2_RX_GPIO_CLK_ENABLE();
		
		/* Enable USARTx clock */
		USART2_CLK_ENABLE();	

		/* Configure TX pin */
		GPIO_InitStruct.Pin       = USART2_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	//	GPIO_InitStruct.Alternate = USART2_TX_AF;
		HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* Configure RX pin */
		GPIO_InitStruct.Pin = USART2_RX_PIN;
	//	GPIO_InitStruct.Alternate = USART2_RX_AF;
		HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);

		/* Configure NVIC for UART */   
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 2);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	  
		/* Configure baud rate and parity */
		bsp_SetUartParam(USART2,  UART2_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);	// UART_MODE_TX_RX

		CLEAR_BIT(USART2->SR, USART_SR_TC);   /* Clear TC (Transmission Complete) flag */
		CLEAR_BIT(USART2->SR, USART_SR_RXNE); /* Clear RXNE (Receive Not Empty) flag */
		SET_BIT(USART2->CR1, USART_CR1_RXNEIE);	/* Enable RX interrupt */
	#endif

#if UART3_FIFO_EN == 1			/* UART3 */
	/* Enable GPIO TX/RX clock */
	USART3_TX_GPIO_CLK_ENABLE();
	USART3_RX_GPIO_CLK_ENABLE();
	
	/* Enable USARTx clock */
	USART3_CLK_ENABLE();	

	/* Configure TX pin */
	GPIO_InitStruct.Pin       = USART3_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = USART3_TX_AF;
	HAL_GPIO_Init(USART3_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* Configure RX pin */
	GPIO_InitStruct.Pin = USART3_RX_PIN;
	GPIO_InitStruct.Alternate = USART3_RX_AF;
	HAL_GPIO_Init(USART3_RX_GPIO_PORT, &GPIO_InitStruct);

	/* Configure NVIC for UART */   
	HAL_NVIC_SetPriority(USART3_IRQn, 0, 3);
	HAL_NVIC_EnableIRQ(USART3_IRQn);
  
	/* Configure baud rate and parity */
	bsp_SetUartParam(USART3,  UART3_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	CLEAR_BIT(USART3->SR, USART_SR_TC);   /* Clear TC (Transmission Complete) flag */
	CLEAR_BIT(USART3->SR, USART_SR_RXNE); /* Clear RXNE (Receive Not Empty) flag */
	SET_BIT(USART3->CR1, USART_CR1_RXNEIE);	/* Enable RX interrupt */
#endif

#if UART4_FIFO_EN == 1			/* UART4 TX = PC10   RX = PC11 */
	/* Enable GPIO TX/RX clock */
	UART4_TX_GPIO_CLK_ENABLE();
	UART4_RX_GPIO_CLK_ENABLE();
	
	/* Enable USARTx clock */
	UART4_CLK_ENABLE();	

	/* Configure TX pin */
	GPIO_InitStruct.Pin       = UART4_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = UART4_TX_AF;
	HAL_GPIO_Init(UART4_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* Configure RX pin */
	GPIO_InitStruct.Pin = UART4_RX_PIN;
	GPIO_InitStruct.Alternate = UART4_RX_AF;
	HAL_GPIO_Init(UART4_RX_GPIO_PORT, &GPIO_InitStruct);

	/* Configure NVIC for UART */   
	HAL_NVIC_SetPriority(UART4_IRQn, 0, 4);
	HAL_NVIC_EnableIRQ(UART4_IRQn);
  
	/* Configure baud rate and parity */
	bsp_SetUartParam(UART4,  UART4_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	CLEAR_BIT(USART4->SR, USART_SR_TC);   /* Clear TC (Transmission Complete) flag */
	CLEAR_BIT(USART4->SR, USART_SR_RXNE); /* Clear RXNE (Receive Not Empty) flag */
	SET_BIT(UART4->CR1, USART_CR1_RXNEIE);	/* Enable RX interrupt */
#endif

#if UART5_FIFO_EN == 1			/* UART5 TX = PC12   RX = PD2 */
	/* Enable GPIO TX/RX clock */
	UART5_TX_GPIO_CLK_ENABLE();
	UART5_RX_GPIO_CLK_ENABLE();
	
	/* Enable USARTx clock */
	UART5_CLK_ENABLE();	

	/* Configure TX pin */
	GPIO_InitStruct.Pin       = UART5_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = UART5_TX_AF;
	HAL_GPIO_Init(UART5_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* Configure RX pin */
	GPIO_InitStruct.Pin = UART5_RX_PIN;
	GPIO_InitStruct.Alternate = UART5_RX_AF;
	HAL_GPIO_Init(UART5_RX_GPIO_PORT, &GPIO_InitStruct);

	/* Configure NVIC for UART */   
	HAL_NVIC_SetPriority(UART5_IRQn, 0, 5);
	HAL_NVIC_EnableIRQ(UART5_IRQn);
  
	/* Configure baud rate and parity */
	bsp_SetUartParam(UART5,  UART5_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	CLEAR_BIT(USART5->SR, USART_SR_TC);   /* Clear TC (Transmission Complete) flag */
	CLEAR_BIT(USART5->SR, USART_SR_RXNE); /* Clear RXNE (Receive Not Empty) flag */
	SET_BIT(UART5->CR1, USART_CR1_RXNEIE);	/* Enable RX interrupt */
#endif

#if UART6_FIFO_EN == 1			/* USART6 */
	/* Enable GPIO TX/RX clock */
	USART6_TX_GPIO_CLK_ENABLE();
	USART6_RX_GPIO_CLK_ENABLE();
	
	/* Enable USARTx clock */
	USART6_CLK_ENABLE();	

	/* Configure TX pin */
	GPIO_InitStruct.Pin       = USART6_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = USART6_TX_AF;
	HAL_GPIO_Init(USART6_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* Configure RX pin */
	GPIO_InitStruct.Pin = USART6_RX_PIN;
	GPIO_InitStruct.Alternate = USART6_RX_AF;
	HAL_GPIO_Init(USART6_RX_GPIO_PORT, &GPIO_InitStruct);

	/* Configure NVIC for UART */   
	HAL_NVIC_SetPriority(USART6_IRQn, 0, 6);
	HAL_NVIC_EnableIRQ(USART6_IRQn);
	
	/* Configure baud rate and parity */
	bsp_SetUartParam(USART6,  UART6_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	CLEAR_BIT(USART6->SR, USART_SR_TC);   /* Clear TC (Transmission Complete) flag */
	CLEAR_BIT(USART6->SR, USART_SR_RXNE); /* Clear RXNE (Receive Not Empty) flag */
	SET_BIT(USART6->CR1, USART_CR1_RXNEIE);	/* Enable RX interrupt */
#endif

#if UART7_FIFO_EN == 1			/* UART7 */
	/* Enable GPIO TX/RX clock */
	UART7_TX_GPIO_CLK_ENABLE();
	UART7_RX_GPIO_CLK_ENABLE();
	
	/* Enable USARTx clock */
	UART7_CLK_ENABLE();	

	/* Configure TX pin */
	GPIO_InitStruct.Pin       = UART7_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = UART7_TX_AF;
	HAL_GPIO_Init(UART7_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* Configure RX pin */
	GPIO_InitStruct.Pin = UART7_RX_PIN;
	GPIO_InitStruct.Alternate = UART7_RX_AF;
	HAL_GPIO_Init(UART7_RX_GPIO_PORT, &GPIO_InitStruct);

	/* Configure NVIC for UART */   
	HAL_NVIC_SetPriority(UART7_IRQn, 0, 6);
	HAL_NVIC_EnableIRQ(UART7_IRQn);
	
	/* Configure baud rate and parity */
	bsp_SetUartParam(UART7,  UART7_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

	CLEAR_BIT(USART7->SR, USART_SR_TC);   /* Clear TC (Transmission Complete) flag */
	CLEAR_BIT(USART7->SR, USART_SR_RXNE); /* Clear RXNE (Receive Not Empty) flag */
	SET_BIT(UART7->CR1, USART_CR1_RXNEIE);	/* Enable RX interrupt */
#endif

#if UART8_FIFO_EN == 1			/* UART8 */
	/* Enable GPIO TX/RX clock */
	UART8_TX_GPIO_CLK_ENABLE();
	UART7_RX_GPIO_CLK_ENABLE();
	
	/* Enable USARTx clock */
	UART8_CLK_ENABLE();	

	/* Configure TX pin */
	GPIO_InitStruct.Pin       = UART8_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = UART8_TX_AF;
	HAL_GPIO_Init(UART8_TX_GPIO_PORT, &GPIO_InitStruct);	
	
	/* Configure RX pin */
	GPIO_InitStruct.Pin = UART8_RX_PIN;
	GPIO_InitStruct.Alternate = UART8_RX_AF;
	HAL_GPIO_Init(UART8_RX_GPIO_PORT, &GPIO_InitStruct);

	/* Configure NVIC for UART */   
	HAL_NVIC_SetPriority(UART8_IRQn, 0, 6);
	HAL_NVIC_EnableIRQ(UART8_IRQn);
	
	/* Configure baud rate and parity */
	bsp_SetUartParam(UART8,  UART8_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);
	
	CLEAR_BIT(USART8->SR, USART_SR_TC);   /* Clear TC (Transmission Complete) flag */
	CLEAR_BIT(USART8->SR, USART_SR_RXNE); /* Clear RXNE (Receive Not Empty) flag */
	SET_BIT(UART8->CR1, USART_CR1_RXNEIE);	/* Enable RX interrupt */
#endif
}

/*
*********************************************************************************************************
*	Function Name: UartSend
*	Description  : Fill data into the UART transmit buffer and enable the transmit interrupt. 
*	               The interrupt handler will automatically disable the transmit interrupt after 
*	               completing the transmission.
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t i;

	for (i = 0; i < _usLen; i++)
	{
		/* If the transmit buffer is already full, wait for the buffer to become empty */
		while (1)
		{
			__IO uint16_t usCount;

			DISABLE_INT();
			usCount = _pUart->usTxCount;
			ENABLE_INT();

			if (usCount < _pUart->usTxBufSize)
			{
				break;
			}
			else if(usCount == _pUart->usTxBufSize)/* Data has filled the buffer */
			{
				if((_pUart->uart->CR1 & USART_CR1_TXEIE) == 0)
				{
					SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);
				}  
			}
		}

		/* Fill the new data into the transmit buffer */
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		_pUart->usTxCount++;
		ENABLE_INT();
	}

	SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);	/* Enable transmit interrupt (buffer empty) */
}
/*
*********************************************************************************************************
*	Function Name: UartGetChar
*	Description  : Reads 1 byte of data from the UART receive buffer (for main program calls)
*	Parameters   : _pUart : UART device
*	              _pByte : Pointer to store the read data
*	Return Value : 0 indicates no data, 1 indicates data was read
*********************************************************************************************************
*/
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte)
{
	uint16_t usCount;

	/* The usRxWrite variable is modified in the interrupt function. When the main program reads this variable, 
	   critical section protection must be applied. */
	DISABLE_INT();
	usCount = _pUart->usRxCount;
	ENABLE_INT();

	/* If the read and write indices are the same, return 0 */
	//if (_pUart->usRxRead == usRxWrite)
	if (usCount == 0)	/* No more data available */
	{
		return 0;
	}
	else
	{
		*_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* Retrieve one piece of data from the UART receive FIFO */

		/* Update the FIFO read index */
		DISABLE_INT();
		if (++_pUart->usRxRead >= _pUart->usRxBufSize)
		{
			_pUart->usRxRead = 0;
		}
		_pUart->usRxCount--;
		ENABLE_INT();
		return 1;
	}
}
/*
*********************************************************************************************************
*   Function Name: UartTxEmpty
*   Description  : Check if the transmit buffer is empty.
*   Parameters   : _pUart : UART device
*   Return Value : 1 if empty, 0 if not empty.
*********************************************************************************************************
*/
uint8_t UartTxEmpty(COM_PORT_E _ucPort)
{
   UART_T *pUart;
   uint8_t Sending;
   
   pUart = ComToUart(_ucPort);
   if (pUart == 0)
   {
      return 0;
   }

   Sending = pUart->Sending;

   if (Sending != 0)
   {
      return 0;
   }
   return 1;
}
/*
*********************************************************************************************************
*	Function Name: UartIRQ
*	Description  : General UART interrupt handler function for interrupt service routines
*	Parameters   : _pUart : UART device
*	Return Value : None
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{
	uint32_t isrflags   = READ_REG(_pUart->uart->SR);
	uint32_t cr1its     = READ_REG(_pUart->uart->CR1);
	uint32_t cr3its     = READ_REG(_pUart->uart->CR3);
	
	/* Handle receive interrupt */
	if ((isrflags & USART_SR_RXNE) != RESET)
	{
		/* Read data from the UART receive data register and store it in the receive FIFO */
		uint8_t ch;

		ch = READ_REG(_pUart->uart->DR);
		_pUart->pRxBuf[_pUart->usRxWrite] = ch;
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}

		/* Callback function to notify the application of new data, typically by sending a message or setting a flag */
		//if (_pUart->usRxWrite == _pUart->usRxRead)
		//if (_pUart->usRxCount == 1)
		{
			if (_pUart->ReciveNew)
			{
				_pUart->ReciveNew(ch); /* For example, pass it to the MODBUS decoding program to process the byte stream */
			}
		}
	}
	/* Handle transmit buffer empty interrupt */
	if ( ((isrflags & USART_SR_TXE) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* When the data in the transmit buffer has been completely fetched, 
			   disable the transmit buffer empty interrupt 
			   (Note: At this point, the last piece of data has not been fully transmitted yet) */
			//USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);

			/* Enable data transmission complete interrupt */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
			SET_BIT(_pUart->uart->CR1, USART_CR1_TCIE);
		}
		else
		{
			_pUart->Sending = 1;
			
			/* Fetch one byte from the transmit FIFO and write it to the UART transmit data register */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->DR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}

	}
	/* Interrupt triggered when all data bits have been transmitted */
	if (((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* If all data in the transmit FIFO has been sent, disable the transmission complete interrupt */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TCIE);

			/* Callback function, typically used for RS485 communication to set the RS485 chip to receive mode, avoiding bus contention */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
			
			_pUart->Sending = 0;
		}
		else
		{
			/* Normally, this branch should not be entered */

			/* If data in the transmit FIFO is not yet fully sent, fetch one piece of data from the transmit FIFO and write it to the transmit data register */
			//USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			_pUart->uart->DR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
}
/*
*********************************************************************************************************
*	Function Name: USART1_IRQHandler, USART2_IRQHandler, USART3_IRQHandler, UART4_IRQHandler, UART5_IRQHandler, etc.
*	Description  : USART interrupt service routines
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1
void USART1_IRQHandler(void)
{
	UartIRQ(&g_tUart1);
}
#endif

#if UART2_FIFO_EN == 1
void USART2_IRQHandler(void)
{
	UartIRQ(&g_tUart2);
}
#endif

#if UART3_FIFO_EN == 1
void USART3_IRQHandler(void)
{
	UartIRQ(&g_tUart3);
}
#endif

#if UART4_FIFO_EN == 1
void UART4_IRQHandler(void)
{
	UartIRQ(&g_tUart4);
}
#endif

#if UART5_FIFO_EN == 1
void UART5_IRQHandler(void)
{
	UartIRQ(&g_tUart5);
}
#endif

#if UART6_FIFO_EN == 1
void USART6_IRQHandler(void)
{
	UartIRQ(&g_tUart6);
}
#endif

#if UART7_FIFO_EN == 1
void UART7_IRQHandler(void)
{
	UartIRQ(&g_tUart7);
}
#endif

#if UART8_FIFO_EN == 1
void UART8_IRQHandler(void)
{
	UartIRQ(&g_tUart8);
}
#endif

/*
*********************************************************************************************************
*	Function Name: fputc
*	Description  : Redefine the putc function, allowing the printf function to print output via UART1
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
#if 1	/* Send the character to be printed via UART interrupt FIFO, the printf function will return immediately */
	comSendChar(COM1, ch);
	
	return ch;
#else	/* Use blocking mode to send each character, wait for the data to be sent completely */
	/* Write one byte to USART1 */
	USART1->DR = ch;
	
	/* Wait for the transmission to complete */
	while((USART1->SR & USART_SR_TC) == 0)
	{}
	
	return ch;
#endif
}

/*
*********************************************************************************************************
*	Function Name: fgetc
*	Description  : Redefine the getc function, allowing the getchar function to input data from UART1
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
int fgetc(FILE *f)
{

#if 1	/* Retrieve one piece of data from the UART receive FIFO, only return when data is obtained */
	uint8_t ucData;

	while(comGetChar(COM1, &ucData) == 0);

	return ucData;
#else
	/* Wait until data is received */
	while((USART1->SR & USART_SR_RXNE) == 0)
	{}

	return (int)USART1->DR;
#endif
}

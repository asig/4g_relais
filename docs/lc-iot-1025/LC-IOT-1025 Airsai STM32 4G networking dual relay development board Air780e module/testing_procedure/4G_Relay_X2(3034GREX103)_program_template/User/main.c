/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : Air780E测试程序。
*              实验内容：
*                1、插入SIM卡，上电。
*                2、STM32发送AT指令要求Air780e联网，如果联网成功则打开继电器。
*
*********************************************************************************************************
*/	
#include "main.h"			


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"Air780E测试程序"
#define EXAMPLE_DATE	"2024-04-22"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
void Relay_Init();
void Relay1_Init();



/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{

	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

	
	Relay_Init();
	Relay1_Init();

	Air780e_NITZ();
	Air780e_Online();
	
	bsp_StartAutoTimer(0, 1000); /* 启动1个1s的自动重装的定时器 */

	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔1s 进来一次 */  
			if( Is_Air780e_Online())
			{
				RELAY_ON;
				RELAY1_ON;
			}
			else
			{
				RELAY_OFF;
				RELAY_OFF;
			}
				
		}
		
		//通过STM32串口直接和AIR780E交互
		//UART1接收到的数据通过UART2发送
		uint8_t buf;
		if(	comGetChar(COM1,&buf))
			comSendChar(COM2,buf);
		//UART2接收到的数据通过UART1发送
		if(	comGetChar(COM2,&buf))
			comSendChar(COM1,buf);

	}
}


/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 打印操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("Air780E测试程序:\r\n");
	printf("1. 插入SIM卡，上电。\r\n");
	printf("2. STM32发送AT指令要求Air780e联网，如果联网成功则打开继电器。\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	
	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

}

void Relay_Init()
{
	
	GPIO_InitTypeDef gpio_init;
	/* 第1步：打开GPIO时钟 */
	RELAY_GPIO_CLK_ENABLE();
	gpio_init.Pin = RELAY_PIN;
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* 设置PP输出 */
	gpio_init.Pull = GPIO_NOPULL;                		/* 上下拉电阻不使能 */
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* GPIO速度等级 */
	HAL_GPIO_Init(RELAY_PORT, &gpio_init);	
}

void Relay1_Init()
{
	
	GPIO_InitTypeDef gpio_init;
	/* 第1步：打开GPIO时钟 */
	RELAY_GPIO_CLK_ENABLE();
	gpio_init.Pin = RELAY1_PIN;
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* 设置PP输出 */
	gpio_init.Pull = GPIO_NOPULL;                		/* 上下拉电阻不使能 */
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* GPIO速度等级 */
	HAL_GPIO_Init(RELAY_PORT, &gpio_init);	
}

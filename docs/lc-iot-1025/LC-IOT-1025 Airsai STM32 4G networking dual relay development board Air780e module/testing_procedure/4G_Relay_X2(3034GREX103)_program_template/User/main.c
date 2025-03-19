/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : Air780E���Գ���
*              ʵ�����ݣ�
*                1������SIM�����ϵ硣
*                2��STM32����ATָ��Ҫ��Air780e��������������ɹ���򿪼̵�����
*
*********************************************************************************************************
*/	
#include "main.h"			


/* ���������������̷������� */
#define EXAMPLE_NAME	"Air780E���Գ���"
#define EXAMPLE_DATE	"2024-04-22"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
void Relay_Init();
void Relay1_Init();



/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{

	bsp_Init();		/* Ӳ����ʼ�� */
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	
	Relay_Init();
	Relay1_Init();

	Air780e_NITZ();
	Air780e_Online();
	
	bsp_StartAutoTimer(0, 1000); /* ����1��1s���Զ���װ�Ķ�ʱ�� */

	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			/* ÿ��1s ����һ�� */  
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
		
		//ͨ��STM32����ֱ�Ӻ�AIR780E����
		//UART1���յ�������ͨ��UART2����
		uint8_t buf;
		if(	comGetChar(COM1,&buf))
			comSendChar(COM2,buf);
		//UART2���յ�������ͨ��UART1����
		if(	comGetChar(COM2,&buf))
			comSendChar(COM1,buf);

	}
}


/*
*********************************************************************************************************
*	�� �� ��: PrintfHelp
*	����˵��: ��ӡ������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("Air780E���Գ���:\r\n");
	printf("1. ����SIM�����ϵ硣\r\n");
	printf("2. STM32����ATָ��Ҫ��Air780e��������������ɹ���򿪼̵�����\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	
	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

}

void Relay_Init()
{
	
	GPIO_InitTypeDef gpio_init;
	/* ��1������GPIOʱ�� */
	RELAY_GPIO_CLK_ENABLE();
	gpio_init.Pin = RELAY_PIN;
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* ����PP��� */
	gpio_init.Pull = GPIO_NOPULL;                		/* ���������費ʹ�� */
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* GPIO�ٶȵȼ� */
	HAL_GPIO_Init(RELAY_PORT, &gpio_init);	
}

void Relay1_Init()
{
	
	GPIO_InitTypeDef gpio_init;
	/* ��1������GPIOʱ�� */
	RELAY_GPIO_CLK_ENABLE();
	gpio_init.Pin = RELAY1_PIN;
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* ����PP��� */
	gpio_init.Pull = GPIO_NOPULL;                		/* ���������費ʹ�� */
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* GPIO�ٶȵȼ� */
	HAL_GPIO_Init(RELAY_PORT, &gpio_init);	
}

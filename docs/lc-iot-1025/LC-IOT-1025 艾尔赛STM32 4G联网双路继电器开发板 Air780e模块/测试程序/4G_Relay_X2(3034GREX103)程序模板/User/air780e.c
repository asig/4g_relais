#include "main.h"			
#include "string.h"

void Air780e_Init()
{
	GPIO_InitTypeDef gpio_init;
//	/* 第1步：打开GPIO时钟 */
//	AIR780E_BOOT_GPIO_CLK_ENABLE();
//	gpio_init.Pin = AIR780E_BOOT_PIN;
//	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* 设置PP输出 */
//	gpio_init.Pull = GPIO_NOPULL;                		/* 上下拉电阻不使能 */
//	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* GPIO速度等级 */

//	HAL_GPIO_Init(AIR780E_BOOT_PORT, &gpio_init);	

}

void Air780e_Boot()
{


}

void Air780e_Shutdown()
{


}

void Air780e_Answer()
{
	uint8_t read;

	bsp_DelayMS(100);
	while(comGetChar(COM2,&read))
	{
		printf("%c",read);
	}
}

void Air780e_NITZ()
{
	uint8_t read=0,status=0,timeout=0;

	comSendBuf(COM2,"AT+RESET\r",9) ;
//收到 +NITZ数据包 意味着启动成功可以开始联网,没有收到会一直循环

	while(status<6)
	{
		comGetChar(COM2,&read);
		{
			switch(status)
			{
				case 0: {if(read=='+') status=1;else status=0;}break;
				case 1: {if(read=='N') status=2;else status=1;}break;
				case 2: {if(read=='I') status=3;else status=2;}break;
				case 3: {if(read=='T') status=4;else status=3;}break;
				case 4: {if(read=='Z') status=5;else status=4;}break;
				case 5: {if(read==':') status=6;else status=5;}break;

			}

		}
//		bsp_DelayMS(200);
//		timeout++;
//		if(timeout>50)//超时10S
//		{
//			printf("\n联网失败，检查SIM卡是否插好！按下复位键重启\n");
//			while(1);
//		}

	}


}

//连接4G网络,打印获得的IP
void Air780e_Online()
{

	comSendBuf(COM2,"AT\r",3) ;
	Air780e_Answer();
	comSendBuf(COM2,"AT+CSTT\r",8) ;
	Air780e_Answer();
	comSendBuf(COM2,"AT+CIICR\r",9) ;
	Air780e_Answer();
	comSendBuf(COM2,"AT+CIFSR\r",9) ;
	Air780e_Answer();
}

//查询是否已经联网  1:已经联网 0:未联网
uint8_t  Is_Air780e_Online()
{
	uint8_t read=0,status=0,resoult=0;

	
	comSendBuf(COM2,"AT+CIFSR\r",9) ;
	bsp_DelayMS(100);
	while(status<5)
	{
		comGetChar(COM2,&read);
		{
			switch(status)
			{
				case 0: {if(read=='\r') status=1;else status=0;}break;
				case 1: {if(read=='\n') status=2;else status=1;}break;
				case 2: {if(read=='\r') status=3;else status=2;}break;
				case 3: {if(read=='\n') status=4;else status=3;}break;
				case 4: {
							if(read=='+') 
								resoult= 0;
							if(read<='9'&& read>='0') 
								resoult= 1;
							
							bsp_DelayMS(1);
							comClearRxFifo(COM2);
							return resoult;
						}break;


			}

		}
	}
}

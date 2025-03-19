#include "main.h"
#include "string.h"

void Air780e_Init()
{
	GPIO_InitTypeDef gpio_init;
//	/* Step 1: Enable GPIO clock */
	//	AIR780E_BOOT_GPIO_CLK_ENABLE();
	//	gpio_init.Pin = AIR780E_BOOT_PIN;
	//	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* Set to push-pull output mode */
	//	gpio_init.Pull = GPIO_NOPULL;                		/* Do not enable pull-up or pull-down resistors */
	//	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* Set GPIO speed to high frequency */

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
	// Receiving the +NITZ packet means the startup was successful and networking can begin. If not received, it will keep looping.

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
		//		if(timeout>50)//Timeout 10 seconds
		//		{
		//			printf("\nNetwork connection failed, check if the SIM card is properly inserted! Press the reset button to restart\n");
		//			while(1);
		//		}

	}


}

// Connect to the 4G network and print the obtained IP
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

// Check if already connected to the network  1: connected, 0: not connected
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

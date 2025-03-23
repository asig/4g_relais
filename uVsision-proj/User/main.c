/*
*********************************************************************************************************
*
*	Module Name : Main Program Module
*	File Name   : main.c
*	Version     : V1.0
*	Description : Air780E Test Program.
*	              Experiment Content:
*	                1. Insert SIM card and power on.
*	                2. STM32 sends AT commands to request Air780e to connect to the network. If the connection is successful, the relay is turned on.
*
*********************************************************************************************************
*/	
#include "main.h"			


/* Define example name and release date */
#define EXAMPLE_NAME	"Air780E Test Program"
#define EXAMPLE_DATE	"2024-04-22"
#define DEMO_VER		"1.0"

static void PrintfLogo(void);
static void PrintfHelp(void);
void Relay_Init();
void Relay1_Init();



/*
*********************************************************************************************************
*	Function Name: main
*	Description  : Entry point of the C program
*	Parameters   : None
*	Return Value : Error code (no need to handle)
*********************************************************************************************************
*/
int main(void)
{

    bsp_Init();		/* Hardware initialization */
    PrintfLogo();	/* Print example name, version, and other information */
    PrintfHelp();	/* Print operation instructions */

    
    Relay_Init();
    Relay1_Init();

    Air780e_NITZ();
    Air780e_Online();
    
    bsp_StartAutoTimer(0, 1000); /* Start a 1-second auto-reloading timer */

    /* Enter the main program loop */
    while (1)
    {
        bsp_Idle();		/* This function is in bsp.c file. Users can modify this function to implement CPU sleep and watchdog feeding */

        /* Check if the timer timeout has occurred */
        if (bsp_CheckTimer(0))	
        {
            /* Enter here every 1 second */  
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
        
        // Direct interaction between STM32 and AIR780E via UART
        // Data received on UART1 is sent via UART2
        uint8_t buf;
        if(	comGetChar(COM1,&buf))
            comSendChar(COM2,buf);
        // Data received on UART2 is sent via UART1
        if(	comGetChar(COM2,&buf))
            comSendChar(COM1,buf);

    }
}


/*
*********************************************************************************************************
*	Function Name: PrintfHelp
*	Description  : Print operation instructions
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
    printf("Air780E Test Program:\r\n");
    printf("1. Insert SIM card and power on.\r\n");
    printf("2. STM32 sends AT commands to request Air780e to connect to the network. If the connection is successful, the relay is turned on.\r\n");
}

/*
*********************************************************************************************************
*	Function Name: PrintfLogo
*	Description  : Print example name and release date. After connecting the serial cable, open the PC's terminal software to observe the result.
*	Parameters   : None
*	Return Value : None
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
    
    printf("\n\r");
    printf("*************************************************************\n\r");
    printf("* Example Name   : %s\r\n", EXAMPLE_NAME);	/* Print example name */
    printf("* Example Version: %s\r\n", DEMO_VER);		/* Print example version */
    printf("* Release Date   : %s\r\n", EXAMPLE_DATE);	/* Print example release date */
    printf("*************************************************************\n\r");
    printf("* BUILT FROM NEW LOCATION                                   *\n\r");
    printf("*************************************************************\n\r");

}

void Relay_Init()
{
    
    GPIO_InitTypeDef gpio_init;
    /* Step 1: Enable GPIO clock */
    RELAY_GPIO_CLK_ENABLE();
    gpio_init.Pin = RELAY_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* Set to push-pull output */
    gpio_init.Pull = GPIO_NOPULL;                		/* Disable pull-up and pull-down resistors */
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* GPIO speed level */
    HAL_GPIO_Init(RELAY_PORT, &gpio_init);	
}

void Relay1_Init()
{
    
    GPIO_InitTypeDef gpio_init;
    /* Step 1: Enable GPIO clock */
    RELAY_GPIO_CLK_ENABLE();
    gpio_init.Pin = RELAY1_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			/* Set to push-pull output */
    gpio_init.Pull = GPIO_NOPULL;                		/* Disable pull-up and pull-down resistors */
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  			/* GPIO speed level */
    HAL_GPIO_Init(RELAY_PORT, &gpio_init);	
}
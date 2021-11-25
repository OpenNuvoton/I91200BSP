/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 17/08/08 2:00p $
 * @brief    WDT Driver Sample Code(Message display via UART0(GPA4,GPA5))
 * @note	 Please do not use Nu-Link Pro to try this demo code.
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"

// Pre-declare function.
void System_Initiate(void);

void DEMO_InitGPIO(void);
BOOL DEMO_IsSWB0Pressing(void);
void DEMO_FlashDA13(void);

// Show message.
void ShowMessage_Setup(void);
void ShowMessage_Process(void);
void ShowMessage_PreReset(void);

int main(void)
{
	// Initiate system clock
	System_Initiate();		
	
	// GPIO configuration for this demo sample(DA0,SWB4)]
	DEMO_InitGPIO();
	
	// Message : WDT setup.
	ShowMessage_Setup();

	// Initiate WDT
	{
		// Enable IP clock
		CLK_EnableModuleClock(WDT_MODULE);
		// Select IP clock source
		CLK_SetModuleClock(WDT_MODULE,CLK_CLKSEL1_WDTSEL_HIRC,MODULE_NoMsk);
		// Open WDT interface.
		// - WDT_TIMEOUT_2POW18 : Time-out frequency(2^12*WDT clock).
		// - TRUE : Enable reset MCU.
		WDT_Open(WDT_TIMEOUT_2POW18,TRUE);
		// Disable WDT interrupt to clear reset counter in main loop.
		WDT_DisableInt();
	}
	
	// Message : WDT in process.
	ShowMessage_Process();
	
	// If SWB4 is pressing, quit this loop and stop reset WDT counter.
	while(DEMO_IsSWB0Pressing()==FALSE)
	{
		// Reset WDT counter.
		WDT_RESET_COUNTER();
		// Flash DA10 LED.
		DEMO_FlashDA13();
	}

	// Message : WDT prepare to reset MCU
	ShowMessage_PreReset();
	
	while(1);
}

UINT32 u32DEMO_Counter = 0;
// =====================================================================================
void System_Initiate(void)
{
    // Unlock protected registers	
    SYS_UnlockReg();
    // Enable External XTL32K 
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    // Switch HCLK clock source to HXT
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	// Enable LDO 3.3V
	CLK_EnableLDO(CLK_LDOSEL_3_3V);
	// Update System Core Clock
	SystemCoreClockUpdate();
    // Lock protected registers
    SYS_LockReg();

	// Enable module clock.
	CLK_EnableModuleClock(UART0_MODULE);	
	// Reset module.
	SYS_ResetModule(UART0_RST);
	// Open UART module
	UART_Open(UART0, 115200);	
	// Set UART gpio multi-function.
	SYS->GPA_MFP = (SYS->GPA_MFP&~(SYS_GPA_MFP_PA4MFP_Msk|SYS_GPA_MFP_PA5MFP_Msk))|(SYS_GPA_MFP_PA4MFP_UART0_TX|SYS_GPA_MFP_PA5MFP_UART0_RX);	
	
}
void DEMO_InitGPIO(void)
{
	// Set gpio multi-function.
	SYS->GPA_MFP = (SYS->GPA_MFP&~(SYS_GPA_MFP_PA13MFP_Msk))|SYS_GPA_MFP_PA13MFP_GPIO;
	SYS->GPB_MFP = (SYS->GPB_MFP&~(SYS_GPB_MFP_PB0MFP_Msk))|SYS_GPB_MFP_PB0MFP_GPIO;
	
	// Set PA10 to output pin(DA13). 
	GPIO_SetMode(PA,BIT13,GPIO_MODE_OUTPUT);
	// Set PB0 to input pin(SWB0)
	GPIO_SetMode(PB,BIT0,GPIO_MODE_QUASI);	
	// Wait PB0 is ready.
	while(DEMO_IsSWB0Pressing()==TRUE)
	{
		if(u32DEMO_Counter++>300000)
			break;
	}
}
BOOL DEMO_IsSWB0Pressing(void)
{
	return ((GPIO_GET_IN_DATA(PB)&BIT0)?FALSE:TRUE);
}
void DEMO_FlashDA13(void)
{
	if((u32DEMO_Counter++)<100000)
		GPIO_SET_OUT_DATA(PA,GPIO_GET_OUT_DATA(PA)&~(BIT13));
	else 
	{
		if(u32DEMO_Counter<200000)
			GPIO_SET_OUT_DATA(PA,GPIO_GET_OUT_DATA(PA)|BIT13);
		else
			u32DEMO_Counter = 0;
	}
}

// Show Message ========================================================================
void ShowMessage_Setup(void)
{
	printf("\r\n+------------------------------------------------------------------------+\r\n");
	printf("|                       WDT Driver Sample Code                           |\r\n");
	printf("+------------------------------------------------------------------------+\r\n");	
	if( u32DEMO_Counter>=300000 )
	{
		printf("SWB0 is not ready(in LOW state).\r\n");
		while(1);
	}
	else
	{
		u32DEMO_Counter = 0;
		printf("(1) Setup WDT, includes.\r\n");
		printf("    1. Enable and set WDT IP clock.\r\n");
		printf("    2. Set WDT Time-Out frequency = (2^12)*WDT's clock.\r\n");
		printf("    3. Enable reset MCU.\r\n");	
	}		
}
void ShowMessage_Process(void)
{
	printf("(2) WDT is running now.\r\n");	
	printf("    1. DA13 is flash at the same time.\r\n");	
	printf("    2. User can press SWB0 to stop WDT reset counter clear.\r\n");	
}
void ShowMessage_PreReset(void)
{
	printf("(3) Stop flash DA13 and prepare to reset MCU.\r\n");
}

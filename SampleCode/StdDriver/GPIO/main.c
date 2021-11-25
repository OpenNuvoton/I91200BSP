
/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 16/12/28 12:10p $
 * @brief    Demonstrate the usage of gpio driver, includes
 *           (1) Output mode.
 *           (2) Input mode
 *           (3) External interrupt(trigger irq when high to low(falling))
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"

// Global variables  
volatile BOOL g_bIsIRQ = FALSE;

void System_Initiate(void);

int main(void)
{	
	// Initiate system clock.
	System_Initiate();

    printf("\r\n+------------------------------------------------------------------------+\r\n");
    printf("|                       GPIO Driver Sample Code                          |\r\n");
    printf("+------------------------------------------------------------------------+\r\n");
	printf("(1) Demo GPIO output: DA10 will be light.\r\n");
	// (1) Demo gpio output mode
	// 1. Config gpio pin multi-functon.
	SYS->GPA_MFP = ( SYS->GPA_MFP & ~SYS_GPA_MFP_PA10MFP_Msk ) | SYS_GPA_MFP_PA10MFP_GPIO;
	// 2. Config gpio mode.
	GPIO_SetMode( PA, BIT10, GPIO_MODE_OUTPUT);
	// 3. Config output low to light LED.
	GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)&(~BIT10));
	printf("    <Press any key on keyboard to next demo>\r\n");	
	getchar();
	
	printf("(2) Demo GPIO input: SWB0 will be set as the button.\r\n");
	// (2) Demo gpio input mode
	// 1. Config gpio pin multi-functon.
	SYS->GPB_MFP = ( SYS->GPB_MFP & ~SYS_GPB_MFP_PB0MFP_Msk ) | SYS_GPB_MFP_PB0MFP_GPIO;
	// 2. Config gpio debounce clock source & debounce time.
	GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_512);
	// 3. Enable debounce pin.
	GPIO_ENABLE_DEBOUNCE( PB, BIT0);
	// 4. Config gpio mode.
	GPIO_SetMode( PB, BIT0, GPIO_MODE_QUASI);
	printf("    <Press SWB0 on EVB to next demo>\r\n");	
	// 5. Wait user press key on EVB board.
	while( (GPIO_GET_IN_DATA(PB)&BIT0)==BIT0 );
	
	printf("(2) Demo GPIO interrupt: SWB2 will be set as the button. It will be trigger in irq when it's pressed.\r\n");
	// (3) Demo gpio external interrupt and trigger GPAB irq handler.
	// 1. Config gpio pin multi-functon.
	SYS->GPB_MFP = ( SYS->GPB_MFP & ~SYS_GPB_MFP_PB2MFP_Msk ) | SYS_GPB_MFP_PB2MFP_GPIO;
	// 2. Config gpio debounce clock source & debounce time.
	GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_512);
	// 3. Enable debounce pin.
	GPIO_ENABLE_DEBOUNCE( PB, BIT2);
	// 4. Config gpio mode.
	GPIO_SetMode( PB, BIT2, GPIO_MODE_QUASI);
 	// 6. NVIC process for gpio.
	// - Clear pending NVIC
 	// - Enable NVIC
	NVIC_ClearPendingIRQ(GPAB_IRQn);
	NVIC_EnableIRQ(GPAB_IRQn);
	// 5. Config gpio pin trigger irq when high to low(falling).
	GPIO_EnableInt(PB,2,GPIO_INT_FALLING);
	printf("    <Press SWB2 on EVB to trigger irq>\r\n");	
	// 8. Wait user press key on EVB board and irq happen(comment 7.)
	while( g_bIsIRQ==FALSE );
	
	printf("End demo gpio sample.\r\n");	
	while(1);
}

void GPAB_IRQHandler(void)
{
	// (3) 7. "Demo gpio external interrupt" parts
	//     - Check trigger or not.
	//     - Clear IRQ trigger flag.
	//     - Config global variable to notify main flow.
	if( GPIO_GET_INT_FLAG( PB, BIT2) )
	{
		g_bIsIRQ = TRUE;
		GPIO_CLR_INT_FLAG( PB, BIT2);
        GPIO_DisableInt(PB,2);
		printf("\n    GPIO irq has been trigger.\r\n\r\n");	
	}		
}

void System_Initiate(void)
{
    // Unlock protected registers
    SYS_UnlockReg();
    // Enable clock source 
    CLK_EnableXtalRC(CLK_PWRCTL_HIRC_EN|CLK_PWRCTL_LIRC_EN|CLK_PWRCTL_LXT_EN|CLK_PWRCTL_HXT_EN);
    // Switch HCLK clock source to HIRC
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	// Enable LDO 3.3V
	CLK_EnableLDO(CLK_LDOSEL_3_3V);
	// Update System Core Clock
    SystemCoreClockUpdate();
    // Lock protected registers
    SYS_LockReg();	
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/

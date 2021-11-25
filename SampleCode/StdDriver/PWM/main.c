/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/08/11 2:52p $
 * @brief    I91200 Series PWM Generator and Capture Timer Driver Sample Code
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "I91200.h"

void PWM0_IRQHandler(void)
{
    // Update PWM0 channel 0 period and duty
    PWM_SET_CNR(PWM0, PWM_CH0, 200);
    PWM_SET_CMR(PWM0, PWM_CH0, 50);
	
    // Clear channel 0 period interrupt flag
    PWM_ClearIntFlag(PWM0, 0);
}

void SYS_Init(void)
{
    // Unlock protected registers
    SYS_UnlockReg();
    // Enable clock source 
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    // Switch HCLK clock source to HIRC
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	// Enable LDO 3.3V
	CLK_EnableLDO(CLK_LDOSEL_3_3V);
	// Update System Core Clock
    SystemCoreClockUpdate();
    // Lock protected registers
    SYS_LockReg();
	
	// Enable PWM0 CH0-1 clock.
	CLK_EnableModuleClock(PWM0CH01_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
	/* Set GPA multi-function pins for PWM0 Channel0 */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA10MFP_Msk) ) | SYS_GPA_MFP_PA10MFP_PWM0CH0;

    /* Lock protected registers */
    SYS_LockReg();
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    printf("+------------------------------------------------------------------------+\r\n");
    printf("|                          PWM Driver Sample Code                        |\r\n");
    printf("|                                                                        |\r\n");
    printf("+------------------------------------------------------------------------+\r\n");
    printf("  This sample code will use PWM0 channel 0 to output waveform\r\n");
    printf("  I/O configuration:\r\n");
    printf("    waveform output pin: PWM0(PA.10)\r\n");
    printf("\nUse double buffer feature.\r\n");

    /*
	PWM0 channel 0 waveform of this sample shown below:

	   ______                      ______                      _____
	__| 50   |_______150__________| 50   |_______150__________| 50   PWM waveform
    */

	/* set PWM0 channel 0 output configuration */
    PWM_ConfigOutputChannel(PWM0, PWM_CH0, 30000, 50);

    /* Enable PWM Output path for PWM0 channel 0 */
    PWM_EnableOutput(PWM0, 0x1);

    // Enable PWM channel 0 period interrupt
    //PWM0->INTEN = PWM_INTEN_PIEN0_Msk;
	PWM_EnableInt(PWM0, 0, NULL);
    NVIC_EnableIRQ(PWM0_IRQn);

    // Start
    PWM_Start(PWM0, 0x1);

    while(1);
}

/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 26/10/18 16:00a $
 * @brief    This sample code is for BOD trigger in the different (SPD/Stop) Mode, need to run without
 *           ICE (Nu-Link dongle).
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "Platform.h"
#include "gpio.h"
#include "sys.h"
#include "clk.h"
#include "uart.h"

volatile uint8_t BODFlag = 0;
volatile uint8_t u8Item = 0;
#define BOD_LEVEL BOD_BODVL_28V

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();
    /* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk | CLK_PWRCTL_LXTEN_Msk | CLK_PWRCTL_LIRCEN_Msk);
    /* Switch HCLK clock source to CLK2X a frequency doubled output of OSC48M */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
    // Enable LDO 3.3V
    CLK_EnableLDO(CLK_LDOSEL_3_3V);
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();
    /* Lock protected registers */
    SYS_LockReg();
}

void UART_Init(void)
{
    /* Enable UART module clock. */
    CLK_EnableModuleClock(UART0_MODULE);
    /* Reset UART module */
    SYS_ResetModule(UART0_RST);
    /* Peripheral clock source */
    CLK_SetModuleClock(UART0_MODULE, MODULE_NoMsk, 0);

    /* Set GPG multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA4MFP_Msk) ) | SYS_GPA_MFP_PA4MFP_UART0_TX;
    SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA5MFP_Msk) ) | SYS_GPA_MFP_PA5MFP_UART0_RX;

    /* Configure UART0 and set Baudrate(115200) */
    UART_Open( UART0, 115200);
}

void BOD_IRQHandler(void)
{
		// Disable BOD interrupt
    BOD_DisableInt(BOD);
    NVIC_DisableIRQ(BOD_IRQn);
    // BODEN disable
    BOD_Close();
}

void NMI_Handler(void)
{
    SYS_UnlockReg();
    // Determine the current state of the BOD comparator. BODOUT = 1 implies that VCC is less than BODVL
    if(BOD_GetOutput(BOD))
    {
        // Get Current Status of Interrupt
        if(BOD_GetIntFlag(BOD))
        {
            BODFlag = 1;
            // If BOD trigger, light on LED DA12
            GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~BIT12));
            BOD_ClearIntFlag(BOD);
        }
    }
}

void BOD_WakeUp()
{
    SYS_UnlockReg();
    NVIC_DisableIRQ(BOD_IRQn);
    BODFlag = 0;
    // LED DA12 Light off
    GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) | (BIT12));
    // Open BOD as continous, and BOD level at 2.8V
    BOD_Open(BOD_BODEN_CONTINUOUS, BOD_LEVEL);
    printf("Adjust voltage supply below 2.8V for BOD trigger....\n");
    while(UART_GET_TX_EMPTY(UART0) == 0);
}
/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main (void)
{
    bool exit = TRUE;

    // Lock protected registers.
    if(SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
        SYS_LockReg();

    // Init System, IP clock and multi-function I/O.
    // In the end of SYS_Init() will issue SYS_LockReg() to lock protected register.
    // If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.
    SYS_Init();
    UART_Init();

    printf("\n\nWake-up And Reset.\n");
    printf("\nCPU @ %dHz\r\n", SystemCoreClock);

    if(CLK_GET_POWERDOWNFLAG(CLK, CLK_PWRSTSF_SPDF))
    {
        printf("\nLast Mode is SPD Mode.\n");
    }
    else if (CLK_GET_POWERDOWNFLAG(CLK, CLK_PWRSTSF_STOPF))
    {
        printf("\nLast Mode is Stop Mode.\n");
    }
    else if (CLK_GET_POWERDOWNFLAG(CLK, CLK_PWRSTSF_DSF))
        printf("\nLast Mode is Sleep Mode.\n");
    else
        printf("\nNow is Normal Mode.\n");

    // Clear all power state flags.
    CLK_CLEAR_POWERDOWNFLAG(CLK, CLK_PWRSTSF_SPDF | CLK_PWRSTSF_STOPF | CLK_PWRSTSF_DSF);

    do
    {
        printf("\n\nSBRAM->D[0]=%d, so previous case is %d\n\n\n", SBRAM->D[0], SBRAM->D[0]);
        printf("\n\n+----------------------------------------------------------------------+\n");
        printf("|                     I91260 BOD Driver Sample Code                    |\n");
        printf("+----------------------------------------------------------------------+\n");
        printf("|  [1~3] CBODEN = 1, we will set BODCTL manually,                      |\n");
        printf("|  [1] Standby Power Down (SPD) wake up from BOD trigger and reset,    |\n");
        printf("|  [2] Stop , wake up from BOD trigger,                                |\n");
        printf("|  [3] Normal , wake up from BOD trigger and reset,                    |\n");
        printf("|  [x] Exit the Demo                                                   |\n");
        printf("+----------------------------------------------------------------------+\n");
        printf("Please Input the Correct Number to Trigger Sample.\n\n");

        // LED DA13 Light on
        GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA) & (~BIT13));
        // Reset standy RAM
        SBRAM->D[0] = 0;
        u8Item = getchar();
        switch(u8Item)
        {
            // When a wake up event occurs the PMU will start the Cortex-M0 processor and execute the reset vector,
            // even you don't set BODRST before SPD mode.
            case '1':
                BOD_WakeUp();
                // Enable BOD interrupt
                BOD_EnableInt(BOD);
                NVIC_EnableIRQ(BOD_IRQn);
                printf("\nEnter SPD Mode! Wait for BOD wake up.\n");
                while(UART_GET_TX_EMPTY(UART0) == 0);
                //Before device go to SPD mode, we can keep our data in SBRAM.
                SBRAM->D[0] = 1;
                CLK_StandbyPowerDown(FALSE);
                break;
            // In STOP mode, if you want to do reset after device wakeup, please disable BODINTEN and enable BODRST.
            // We dodn't expect for wakeup and reset features at same time in STOP mode.
            case '2':
                BOD_WakeUp();
                // Enable BOD interrupt
                BOD_EnableInt(BOD);
                NVIC_EnableIRQ(BOD_IRQn);
                printf("\nEnter Stop Mode! Wait for BOD wake up.\n");
                while(UART_GET_TX_EMPTY(UART0) == 0);
                SBRAM->D[0] = 2;
                CLK_Stop(FALSE);
                break;
            case '3':
                BOD_WakeUp();
                // Enable BOD interrupt
                BOD_EnableInt(BOD);
                NVIC_EnableIRQ(BOD_IRQn);
                printf("\nKeeping in Normal Mode! Wait for BOD wake up.\n");
                printf("\nIf you see LED DA12 is light(BOD trigger), sample will return to do while loop.\n");
                while(UART_GET_TX_EMPTY(UART0) == 0);
                SBRAM->D[0] = 3;
                // We take PA12 as check point whether LED DA12 is light.
                while(GPIO_GET_OUT_DATA(PA)&BIT12);
                break;
            case 'x':
                /* Exit the Demo */
                exit = FALSE;
                printf("\nDemo End!!\n");
                break;
            default:
                printf("\nPlease Input the Correct Number to Trigger Sample.\n");
                break;
        }
        /* Case 1,2 will Need to Issue WFI Instruction */
        if(u8Item == '1' || u8Item == '2')
        {
            __WFI();
        }
    }
    while(exit);
}
/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/

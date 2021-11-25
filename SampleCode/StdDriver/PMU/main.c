/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 14:14a $
 * @brief    This code is for standby/ deep power down test, need to run without 
 *           ICE (Nu-Link dongle). GPA13 are used to indicate the program running
 *           status.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "RTC.h"
#include "gpio.h"
#include "sys.h"
#include "clk.h"
#include "pwm.h"
#include "UART.h"
#include "wdt.h"
#include "fmc.h"

#define LED_INDICATE (BIT13)
#define WAKE_UP_PIN	 (BIT11)

volatile uint32_t u32Counter = 0;
void delay(int ms) {while(ms>0) ms--;};

void RTC_IRQHandler(void)
{
	RTC_CLEAR_TICK_INT_FLAG;	
	RTC_CLEAR_ALARM_INT_FLAG;
}

void SYS_Init(void)
{	
	/* Unlock protected registers */
    SYS_UnlockReg();
	/* Enable PA14, 15 for X32K MFP */
	SYS->GPA_MFP = (SYS->GPA_MFP & ~(SYS_GPA_MFP_PA14MFP_Msk|SYS_GPA_MFP_PA15MFP_Msk)) | SYS_GPA_MFP_PA14MFP_X32KI|SYS_GPA_MFP_PA15MFP_X32KO;
	/* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk|CLK_PWRCTL_LXTEN_Msk);
	/* Switch HCLK clock source to CLK2X a frequency doubled output of OSC48M */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	/* Enable LDO 3.3V */
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

void RTC_Init(void)
{
	S_RTC_TIME_DATA_T sCurTime;
	
	/* Enable RTC APB clock */
	CLK_EnableModuleClock(RTC_MODULE);
	 
	/* sPt = 0, only reset RTC  */
	RTC_Open(0);
	
	RTC_EnableWakeUp();
	
	RTC_CLEAR_ALARM_INT_FLAG;

	RTC_CLEAR_TICK_INT_FLAG;

	/* Set Tick setting */
	RTC_SetTickPeriod(RTC_TICK_1_SEC);
	
	/* Time Setting */
    if (SBRAM->D[0] == 0)
	{
		sCurTime.u32Year       = 2017;
		sCurTime.u32Month      = 01;
		sCurTime.u32Day        = 01;
		sCurTime.u32Hour       = 12;
		sCurTime.u32Minute     = 00;
		sCurTime.u32Second     = 00;
		sCurTime.u32DayOfWeek  = RTC_THURSDAY;
		sCurTime.u32TimeScale  = RTC_CLOCK_24;
		/* Set current time*/
		RTC_SetDateAndTime(&sCurTime);
	}
}

void RTC_SetAlarmWakeUp(void)
{
	S_RTC_TIME_DATA_T sCurTime;
	
	/* wait writint to RTC IP */
	CLK_SysTickDelay(60);
	/* wait loading back to RTC_TIME */
	CLK_SysTickDelay(60);
	/* Get the current time */
	RTC_GetDateAndTime(&sCurTime);
	
	/* Set alarm time*/
	sCurTime.u32Second += 5;
	if(sCurTime.u32Second >= 60)
	{
		sCurTime.u32Second -= 60;
		if(++sCurTime.u32Minute >= 60)
		{
			sCurTime.u32Minute -= 60;
			if(++sCurTime.u32Hour >= 24)
				sCurTime.u32Hour -= 24;
		}
	}
	
	RTC_SetAlarmDateAndTime(&sCurTime);
	/* wait writint to RTC IP */
	CLK_SysTickDelay(60);
	/* wait loading back to RTC_TALM */
	CLK_SysTickDelay(60);
	/* Enable RTC alarm Interrupt for wake up */	
	RTC_EnableInt(RTC_INTEN_ALMIEN_Msk);
	RTC_DisableInt(RTC_INTEN_TICKIEN_Msk);
	
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);
	delay(1000);
}

void RTC_SetTickWakeUp(void)
{
	RTC_EnableInt(RTC_INTEN_TICKIEN_Msk);
	RTC_DisableInt(RTC_INTEN_ALMIEN_Msk);
	
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);
	delay(1000);
}

void GPAB_IRQHandler(void)
{
	SYS_UnlockReg();

    /* Clear all the GPIOA, GPIOB 0~15 interrupt */
	GPIO_CLR_INT_FLAG(PA, 0xFFFF);
	GPIO_CLR_INT_FLAG(PB, 0xFFFF);
}
void BOD_WakeUp()
{
	uint32_t u32Config[2]; 

	UNLOCKREG();
	NVIC_DisableIRQ(BOD_IRQn);

	/* Initiate FMC and read config. */
	SYS_UnlockReg();
	FMC_Open();
	
	FMC_ReadConfig( (uint32_t *)u32Config, 2);
	/* Check config state */
	if(((u32Config[0]>>23)&0x01) == 1)
	{
		FMC_EnableConfigUpdate();
		
		/* Enable BOD; Config0[23]:CBODEN - Brown Out Detector Enable */
		u32Config[0] &= ~BIT23;
		
		FMC_WriteConfig( (uint32_t *)u32Config, 2);
		FMC_Close();
		printf("Set Config0 Bit23 = 0, Please reset again....\r\n"); 
		while(UART_GET_TX_EMPTY(UART0)==0);
		SYS_ResetChip();
		/* wait chip reset */
		while(1);
	}

	FMC_Close();
	printf("Adjust voltage supply for BOD reset....\r\n"); 
	while(UART_GET_TX_EMPTY(UART0)==0);
	
	SYS_UnlockReg();

	// Open BOD as continous, and BOD level at 3.0V
	BOD_Open(BOD_BODEN_CONTINUOUS, BOD_BODVL_36V);
	// Set detection time to default.
	BOD_SetDetectionTime(0x0003, 0x03E3);
	// Disable BOD interrupt
	BOD_DisableInt(BOD);
	// Enable BOD reset
	BOD_EnableReset(BOD);
}
/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/    
int main (void)
{
	uint8_t u8Option = 0;
	
	S_RTC_TIME_DATA_T sCurTime;
	
	// Lock protected registers.
	if(SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
		SYS_LockReg();
	
	// Init System, IP clock and multi-function I/O.
	// In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. 
	// If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.
	SYS_Init();
    UART_Init();
	RTC_Init();
	
	printf("\r\n\r\nWake-up And Reset.\r\n");
	printf("\r\n\r\nCPU @ %dHz\r\n", SystemCoreClock);
	
	// Clear the Standby PD flag.
	CLK_CLEAR_POWERDOWNFLAG(CLK, CLK_PWRSTSF_SPDF);
	
	if ( u8Option ==0)
	{
menu:
		printf("\r\n\r\n");
		printf("+----------------------------------------------------------------------+\r\n");
		printf("|                       CLK Driver Sample Code                         |\r\n");
		printf("+----------------------------------------------------------------------+\r\n");
		printf("|  [A] Deep Power Down (PD)  wake up from POR reset,                   |\r\n");
		printf("|  [B] Deep Power Down (PD)  wake up from LIRC TIMER reset,            |\r\n");
		printf("|  [C] Deep Power Down (PD)  wake up from Wake PIN reset,              |\r\n");
		printf("|  [D] Deep Power Down (PD)  wake up from Wake PIN/LIRC reset,         |\r\n");
		printf("|  [E] Standby Power Down (SPD) wake up from RTC tick,                 |\r\n");
		printf("|      CPU will wake up at each tick, tick unit: 1 second              |\r\n");
		printf("|  [F] Standby Power Down (SPD) wake up from RTC alarm                 |\r\n");
		printf("|      CPU will wake up at alarm time, alarm unit: 5 second            |\r\n");
		printf("|  [G] Standby Power Down (SPD) wake up from GPIO reset,               |\r\n");
		printf("|  [H] Stop , wake up from RTC tick,                                   |\r\n");
		printf("|      CPU will wake up at each tick, tick unit: 1 second              |\r\n");
		printf("|  [I] Stop , wake up from RTC alarm                                   |\r\n");
		printf("|      CPU will wake up at alarm time, alarm unit: 5 second            |\r\n");
		printf("|  [J] Stop , wake up from GPIO reset,                                 |\r\n");
		printf("|  [K] Deep Sleep , wake up from RTC tick,                             |\r\n");
		printf("|  [L] Sleep , wake up from RTC tick,                                  |\r\n");
		printf("|  [q] Quit                                                            |\r\n");

		// Lights up LED
		GPIO_SET_OUT_DATA(PA, GPIO_GET_OUT_DATA(PA)&(~LED_INDICATE));
		
		u8Option = getchar();
		// Reset counter
		SBRAM->D[0] = 0;
		u32Counter = 0;
	}
	else
		// Get previous u8Option value.
		u8Option = SBRAM->D[1];
    
	switch(u8Option)
	{
		case 'a':
		case 'A':
			printf("\r\n\r\n   Enter Deep Power Down! Please Prees POR to wake up.\r\n");
			while(UART_GET_TX_EMPTY(UART0)==0);
			CLK_DeepPowerDown(CLK_DPDWAKEUP_POR,0);
		break;
		case 'b':
		case 'B':
			printf("\r\n\r\n   Enter Deep Power Down! After 6400ms timer out wake up.\r\n");
			while(UART_GET_TX_EMPTY(UART0)==0);
			CLK_DeepPowerDown(CLK_DPDWAKEUP_OSC10K,999);			
		break;
		case 'c':
		case 'C':
			printf("\r\n\r\n   Enter Deep Power Down! Please Prees Wakeup PIN to wake up.\r\n");
			while(UART_GET_TX_EMPTY(UART0)==0);
			CLK_DeepPowerDown(CLK_DPDWAKEUP_PIN,0);			
		break;
		case 'd':
		case 'D':
			printf("\r\n\r\n   Enter Deep Power Down!Please Prees Wakeup PIN  or wait v to wake up.\r\n");
			while(UART_GET_TX_EMPTY(UART0)==0);
			CLK_DeepPowerDown(CLK_DPDWAKEUP_PINOSC10K,999);
		break;
		case 'e':
		case 'E':
			RTC_SetTickWakeUp();
			RTC_GetDateAndTime(&sCurTime);
			printf("\r\n\r\n  Sleep Current Time:%d/%02d/%02d %02d:%02d:%02d\r\n",sCurTime.u32Year,sCurTime.u32Month,sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);
			while(UART_GET_TX_EMPTY(UART0)==0);
			// This API will try to enter SPD five times.
			// Each time after RTC triggers will set SPD enable again for 5 times.
			RTC_ENABLE_TICK_WAKEUP;
			CLK_StandbyPowerDown(FALSE);
		break;
		case 'f':
		case 'F':
			RTC_SetAlarmWakeUp();
			RTC_GetDateAndTime(&sCurTime);
			printf("\r\n\r\n  Sleep Current Time:%d/%02d/%02d %02d:%02d:%02d\r\n",sCurTime.u32Year,sCurTime.u32Month,sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);
			while(UART_GET_TX_EMPTY(UART0)==0);
			// This API will try to enter SPD five times.
			// Each time after RTC triggers will set SPD enable again for 5 times.
			RTC_ENABLE_TICK_WAKEUP;
			CLK_StandbyPowerDown(FALSE);
		break;		
		case 'G':
		case 'g':
			SYS_Unlock();
			// Set Quia mode
			GPIO_SetMode(PA, WAKE_UP_PIN, GPIO_MODE_QUASI);
			// Clear INT source PA0~PA15.
			GPIO_CLR_INT_FLAG(PB, 0x0000FFFF);
			// Eanble wake up pin interrupt.
			GPIO_EnableInt(PA, 11, GPIO_INT_BOTH_EDGE);
			printf("\r\n\r\n   Enter SPD Mode! Please Prees WAKEUP pin to wake up.\r\n");
			while(UART_GET_TX_EMPTY(UART0)==0);
			CLK_StandbyPowerDown(FALSE);
			NVIC_EnableIRQ(GPAB_IRQn);
		break;
		case 'H':
		case 'h':
			RTC_SetTickWakeUp();
			// Enable Stop mode and turn off flash ROM when in stop mode.
			CLK_Stop(TRUE);
			RTC_GetDateAndTime(&sCurTime);
			printf("\r\n\r\n  Sleep Current Time:%d/%02d/%02d %02d:%02d:%02d\r\n",sCurTime.u32Year,sCurTime.u32Month,sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);
			while(UART_GET_TX_EMPTY(UART0)==0);
		break;
		case 'I':
		case 'i':
			RTC_SetAlarmWakeUp();
			CLK_Stop(TRUE);
			RTC_GetDateAndTime(&sCurTime);
			printf("\r\n\r\n  Sleep Current Time:%d/%02d/%02d %02d:%02d:%02d\r\n",sCurTime.u32Year,sCurTime.u32Month,sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);
			while(UART_GET_TX_EMPTY(UART0)==0);
		break;		
		case 'J':
		case 'j':
			SYS_Unlock();
			// Set Quia mode
			GPIO_SetMode(PA, WAKE_UP_PIN, GPIO_MODE_QUASI);
			// Clear INT source PA0~PA15.
			GPIO_CLR_INT_FLAG(PA, 0x0000FFFF);
			// Eanble wake up pin interrupt.
			GPIO_EnableInt(PA, 11, GPIO_INT_BOTH_EDGE);
			printf("\r\n\r\n   Enter Stop Mode! Please Prees WAKEUP pin to wake up.\r\n");
			while(UART_GET_TX_EMPTY(UART0)==0);
			CLK_Stop(TRUE);
			NVIC_EnableIRQ(GPAB_IRQn);
		break;				
		case 'K':
		case 'k':
			// Enable RTC without alarm. 
			RTC_SetTickWakeUp();
			RTC_GetDateAndTime(&sCurTime);
			printf("\r\n\r\n  Sleep Current Time:%d/%02d/%02d %02d:%02d:%02d\r\n",sCurTime.u32Year,sCurTime.u32Month,sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);
			while(UART_GET_TX_EMPTY(UART0)==0);
			// Enable RTC clock in deep sleep mode 
			CLK_DeepSleep();
		break;
		case 'L':
		case 'l':
			// Enable RTC without alarm. 
			RTC_SetTickWakeUp();
			RTC_GetDateAndTime(&sCurTime);
			printf("\r\n\r\n  Sleep Current Time:%d/%02d/%02d %02d:%02d:%02d\r\n",sCurTime.u32Year,sCurTime.u32Month,sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);
			while(UART_GET_TX_EMPTY(UART0)==0);
			// Enable RTC clock in deep sleep mode 
			CLK_Sleep();
		break;
		case 'Q':
		case 'q':
			printf("\r\n\r\n   CLK sample code exit.\r\n");
			while(1);
	}

	while(UART_GET_TX_EMPTY(UART0)==0);		

	__WFI();
	
	printf("\r\n\r\n  Wake up.\r\n");
	while(UART_GET_TX_EMPTY(UART0)==0);

	switch(u8Option)
	{
		case 'n':
		case 'N':
		case 'm':
		case 'M':
		case 'i':
		case 'I':
		case 'j':
		case 'J':
		case 'e':
		case 'E':
		case 'f':
		case 'F':
			// Check if RTC is active or not.
			if(!RTC_INIT_ATVSTS_Msk)
			{
				RTC_Init();
			}
			RTC_GetDateAndTime(&sCurTime);
			printf("\r\n\r\n  wakeup Current Time:%d/%02d/%02d %02d:%02d:%02d\r\n",sCurTime.u32Year,sCurTime.u32Month,sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);	
		break;
	}
	goto menu;
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/

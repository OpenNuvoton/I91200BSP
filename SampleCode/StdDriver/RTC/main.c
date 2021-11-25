
/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 16/12/28 12:10p $
 * @brief    Demonstrate the usage of rtc driver, includes
 *           (1) Real time clock.
 *           (2) Interrupt handler(trigger irq when tick(1 second occur))
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "Platform.h"

// Global variables                                                                                        */
volatile uint32_t  g_u32TICK = 0;

void System_Initiate(void);

int main(void)
{	
	S_RTC_TIME_DATA_T sData, sRecordData;
	
	// Initiate system clock.
	System_Initiate();

	while(1) 
	{
		printf("\r\n+------------------------------------------------------------------------+\r\n");
		printf("|                       RTC Driver Sample Code                           |\r\n");
		printf("+------------------------------------------------------------------------+\r\n");
		printf("(1) Demo main loop pooling to count 10 seconds and display current time.\r\n");
		printf("(2) Demo interrupt trigger every second for next 15 seconds.\r\n");
				
		switch(getchar())
		{
			case '1':
			{
				// (1) Demo real time clock
				// 1. Enable RTC module clock & set RTC module clock
				CLK_EnableModuleClock(RTC_MODULE);
				CLK_SetModuleClock(RTC_MODULE,MODULE_NoMsk,MODULE_NoMsk);
				// 2. Config current time(ex.01/01/2017 12:00) to open RTC
				sData.u32Year       = 2017;
				sData.u32Month      = 1;
				sData.u32Day        = 1;
				sData.u32Hour       = 12;
				sData.u32Minute     = 30;
				sData.u32Second     = 0;
				sData.u32DayOfWeek  = RTC_SUNDAY;
				sData.u32TimeScale  = RTC_CLOCK_24;
				sData.u32AmPm       = 0;
				printf("+------------------------------------------------------------------------+\r\n");
				printf("    Set Time:%d/%d/%d/%d:%d:%d\r\n",sData.u32Year,sData.u32Month,sData.u32Day,sData.u32Hour,sData.u32Minute,sData.u32Second);
				memcpy(&sRecordData,&sData,sizeof(S_RTC_TIME_DATA_T));
				RTC_Open(&sData);
				// 3. Wait 10 seconds
				do
				{
					RTC_GetDateAndTime(&sData);
					if( memcmp(&sData,&sRecordData,sizeof(S_RTC_TIME_DATA_T)) != 0 )
					{
						printf("    Now Time:%d/%d/%d/%d:%d:%d\r\n",sData.u32Year,sData.u32Month,sData.u32Day,sData.u32Hour,sData.u32Minute,sData.u32Second);
						memcpy(&sRecordData,&sData,sizeof(S_RTC_TIME_DATA_T));
					}

				}while( sData.u32Second < 10 );
				printf("+------------------------------------------------------------------------+\r\n");
				// 4. Close RTC.
				RTC_Close();
			}break;
			
			case '2':
			{
				// (2) Demo interrupt handler.
				// 1. Enable RTC module clock & set RTC module clock
				CLK_EnableModuleClock(RTC_MODULE);
				CLK_SetModuleClock(RTC_MODULE,MODULE_NoMsk,MODULE_NoMsk);
				// 2. Config current time(ex.01/04/2017 PM 1:00) to open RTC
				sData.u32Year       = 2017;
				sData.u32Month      = 1;
				sData.u32Day        = 4;
				sData.u32Hour       = 1;
				sData.u32Minute     = 0;
				sData.u32Second     = 0;
				sData.u32DayOfWeek  = RTC_SUNDAY;
				sData.u32TimeScale  = RTC_CLOCK_12;
				sData.u32AmPm       = RTC_PM;
				printf("+------------------------------------------------------------------------+\r\n");
				RTC_Open(&sData);
				// 3. Config tick period
				RTC_SetTickPeriod(RTC_TICK_1_SEC);
				// 4. Enable tick interrupt.
				RTC_EnableInt(RTC_INTEN_TICKIEN_Msk);
				NVIC_EnableIRQ(RTC_IRQn);
				// 5. Wait tick interrupt occur 15 times.
				while(g_u32TICK<15);
				printf("+------------------------------------------------------------------------+\r\n");
				// 6. Close RTC
				RTC_Close();
			}break;
		}
	}
}

void RTC_IRQHandler(void)
{
	// tick interrupt occurred
    if ( RTC_GET_TICK_INT_FLAG )
	{      
        RTC_CLEAR_TICK_INT_FLAG;
		printf("    Trigger Interrupt: %d\r\n", ++g_u32TICK);
    }
}

void System_Initiate(void)
{
	// Enable Crystal pin for LXT.
	SYS->GPA_MFP = (SYS->GPA_MFP&~(SYS_GPA_MFP_PA14MFP_Msk|SYS_GPA_MFP_PA15MFP_Msk))|(SYS_GPA_MFP_PA14MFP_X32KI|SYS_GPA_MFP_PA15MFP_X32KO);
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

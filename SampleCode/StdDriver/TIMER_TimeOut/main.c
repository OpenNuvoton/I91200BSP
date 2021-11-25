/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 17/05/10 2:52p $
 * @brief    This sample is demo how to implement time-out macro APIs via VTIMER.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "VTIMER/VTIMER.h"

// Pre-declare function.
void System_Initiate(void);

// Timer Time-out API's function.
BOOL TIMER_TimeOut_Setup(void);
void TIMER_TimeOut_Add(UINT8 u8ID,UINT32 u32TimeOutTime);
void TIMER_TimeOut_Remove(UINT8 u8ID);
void TIMER_TimeOut_Start(void);
void TIMER_TimeOut_Stop(void);
void TIMER_TimeOut_Clear(UINT8 u8ID);
void TIMER_TimeOut_IRQProcess(void);
void TIMER_TimeOut_CallFunc(UINT8 u8ID);

// Show message.
void Show_DemoLabel(void);
void Show_TimeOutSetup(BOOL bSuccess);
void Show_TimeOutAdd(void);
void Show_TimeOutStart(void);
void Show_TimeOutStop(void);
void Show_DemoEnd(void);
void Show_TimeOutTrigger(UINT8 u8ID);

// Time-Out System =====================================================================
#define TIMEOUT_MAX_ID (4)
UINT32 u32TimeOutTime[TIMEOUT_MAX_ID];
UINT32 u32TimeOutCounter[TIMEOUT_MAX_ID];

// Global variable
UINT32 g_u32ID_TriggerCount[TIMEOUT_MAX_ID];

int main(void)
{
	//UINT8 u8Key = '0';
	
	// Initiate system clock
	System_Initiate();	
	// Message : Demo label.
	Show_DemoLabel();
	
	// Setup VTIMER0 to be a Time-out system.
	// Message : Time-out setup success/fail.
	if( TIMER_TimeOut_Setup() == TRUE ) 
		Show_TimeOutSetup(TRUE);
	else
	{
		Show_TimeOutSetup(FALSE);
		while(1);
	}
	
	// Message : Add ID into Time-out system.
	Show_TimeOutAdd();
	// Add ID(0) into Time-out system and its time-out time is 5 second.
	TIMER_TimeOut_Add(0,5000);
	// Add ID(1) into Time-out system and its time-out time is 3 second.
	TIMER_TimeOut_Add(1,3000);
	
	// Message : Start time-out system process.
	Show_TimeOutStart();
	// Start time-out system.
	TIMER_TimeOut_Start();
	
	// Message : Stop time-out system process && Clear ID in time-out system.
	Show_TimeOutStop();
	
	// Demo will stop after both ID have triggered 5 times.
	while(g_u32ID_TriggerCount[0]<5 || g_u32ID_TriggerCount[1]<5)
	{
		// Close the ID triggered 5 times first.
		if(g_u32ID_TriggerCount[0] == 5)
			TIMER_TimeOut_Remove(0);
		if(g_u32ID_TriggerCount[1] == 5)
			TIMER_TimeOut_Remove(1);
	}
	
	// Stop time-out system.
	TIMER_TimeOut_Stop();
	
	// Message : End demo(while in).
	Show_DemoEnd();
}

// Time-Out System =====================================================================
// Time-Out system provide callback API when ID frequency trigger.
// User could add some action here.
void TIMER_TimeOut_CallFunc(UINT8 u8ID)
{
	// Message : Time-out trigger happened.
	Show_TimeOutTrigger(u8ID);
}

// TIMER0's IRQ handler.
void TMR0_IRQHandler(void)
{
	TIMER_TimeOut_IRQProcess();
}

BOOL TIMER_TimeOut_Setup(void)
{
	if ( VTIMER_Open(0, 1000, E_VTIMER_CLK_DEFAULT, TRUE) )
	{
		VTIMER_Config(0,VTIMER_CFG_PERIODIC|VTIMER_CFG_INTEN);
		memset(u32TimeOutTime,'\0',sizeof(u32TimeOutTime));
		memset(u32TimeOutCounter,'\0',sizeof(u32TimeOutCounter));
		return TRUE;
	}
	return FALSE;
}
void TIMER_TimeOut_Add(UINT8 u8ID,UINT32 u32TimeOutMS)
{
	if(u8ID<TIMEOUT_MAX_ID)
	{
		u32TimeOutTime[u8ID]=u32TimeOutMS;
		u32TimeOutCounter[u8ID]=0;
		g_u32ID_TriggerCount[u8ID]=0;
	}
}
void TIMER_TimeOut_Remove(UINT8 u8ID)
{
	(u8ID<TIMEOUT_MAX_ID)?(u32TimeOutTime[u8ID]=0):0;	
}
void TIMER_TimeOut_Start(void)
{
	VTIMER_Start(0);
}
void TIMER_TimeOut_Stop(void)
{
	VTIMER_Stop(0);
}
void TIMER_TimeOut_Clear(UINT8 u8ID)
{
	(u8ID<TIMEOUT_MAX_ID)?(u32TimeOutCounter[u8ID]=0):0;
}
void TIMER_TimeOut_IRQProcess(void)
{
	VTIMER_Process(0);
}
// VTIMER provide callback API when config frequency trigger.
void VTIMER_FrequencyTrigger(UINT8 u8VTIMERNo)
{
	UINT8 u8i;
	
	for( u8i=0; u8i<TIMEOUT_MAX_ID; u8i++ )
	{
		if( u32TimeOutTime[u8i] && (u32TimeOutCounter[u8i]+=1) >= u32TimeOutTime[u8i] )
		{
			TIMER_TimeOut_CallFunc(u8i);
			u32TimeOutCounter[u8i] = 0;
			g_u32ID_TriggerCount[u8i]++;
		}
	}
}

// =====================================================================================
void System_Initiate(void)
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
}
void Show_DemoLabel(void)
{
	printf("\r\n+------------------------------------------------------------------------+\r\n");
	printf("|                      TIMER Driver Sample Code                          |\r\n");
	printf("+------------------------------------------------------------------------+\r\n");			
}
void Show_TimeOutSetup(BOOL bSuccess)
{
	printf("(1) Setup VTIMER0 to be a Time-Out system.\r\n");
	printf("    1. Open VTIMER0 interface(frequency = 1000).\r\n");
	printf("    2. Config VTIMER0 parameter(enable interrupt,periodic mode).\r\n");
    (bSuccess)?printf("    VTIMER setup success.\r\n"):printf("    VTIMER setup fail.\r\n");
}
void Show_TimeOutAdd(void)
{
	printf("(2) Add trigger time of Time-Out ID 0&1 into Time-Out system.\r\n");
	printf("    1. Time-Out ID(0) trigger time is 5 second.\r\n");
	printf("    2. Time-Out ID(1) trigger time is 3 second.\r\n");
}
void Show_TimeOutStart(void)
{
	printf("(3) Start Time-Out system to trigger.\r\n");
	printf("    (Press 'Enter' to start.)\r\n");	
	getchar();
}
void Show_TimeOutStop(void)
{
	printf("(4) Time-Out system is running.\r\n");	
}
void Show_TimeOutTrigger(UINT8 u8ID)
{
	printf("    Time-Out ID (%d) trigger.\r\n",u8ID);		
}
void Show_DemoEnd(void)
{
	printf("(5) TIMER_TimeOut demo end.\r\n"); 
	while(1);
}

/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/08/08 2:52p $
 * @brief    CAPSENSE Driver Sample Code
 *           This is TouchKey demo sample via CAPSENSE.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"

// Pre-declare function.
void System_Initiate(void);

// CAPSENSE Key API's function.
BOOL CAPSENSE_Key_Setup(UINT32 u32PinMap);
void CAPSENSE_Key_Start(void);
void CAPSENSE_Key_Stop(void);
void CAPSENSE_Key_IRQProcess(void);

// Show message.
void Show_DemoLabel(void);
void Show_KeySetup(BOOL bSuccess);
void Show_KeyStart(void);
void Show_DemoWhileloop(void);

int main(void)
{
	// Initiate system clock
	System_Initiate();
	// Message : Demo label.
	Show_DemoLabel();
	
	// Setup capture sense to be 'Touch key' and dont press any key at this time
	// Message : Touch Key setup success/fail.
	if( CAPSENSE_Key_Setup(BIT8|BIT7) == TRUE ) 
		Show_KeySetup(TRUE);
	else
	{
		Show_KeySetup(FALSE);
		while(1);
	}	
	
	// Message : When user press 'ENTER', capture sense will start to scan.
	Show_KeyStart();
	// Capture sense's touch key start to scan.
	CAPSENSE_Key_Start();
	
	// Message : (while in).
	Show_DemoWhileloop();
}

void CAPS_IRQHandler(void)
{
	CAPSENSE_Key_IRQProcess();		
}

// Provide callback API when event happened.
// User could add some action here.
// Note. This callback function was in CAPS's IRQ.
void CAPSENSE_Key_Event(UINT16 u16Event,UINT16 u16State)
{
	UINT8 u8i;
	
	for( u8i=0; u8i<16; u8i++ )
	{
		if( (u16Event&(1<<u8i))>0 )
		{
			if( (u16State&(1<<u8i))>0 )
			{
				printf("GPIOB%d is falling.\r\n",u8i); 
			}
			else
			{
				printf("GPIOB%d is rising.\r\n",u8i); 		
			}
		}
	}
}

// CAPSENSE Key =======================================================================
UINT32 au32BasicValue[16];
UINT16 const au16Threshold[16] = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
INT8 i8CalCount;
UINT16 u16PinState,u16PinEnable;

BOOL CAPSENSE_Key_Cal(void)
{
	UINT32 u32Count = 0x10000000;
	// Cal basic touch value.
	i8CalCount = 3;
	// Clear basic value buffer
	memset(au32BasicValue,'\0',sizeof(au32BasicValue));
	// Start to scan.
	CAPSENSE_Key_Start();
	// Scan for get basic value
	while( i8CalCount > 0 )
	{
		if( (u32Count-=1)==0 )
		{
			i8CalCount = 0;
			CAPSENSE_Key_Stop();
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CAPSENSE_Key_Setup(UINT32 u32PinMap)
{	
	if( u32PinMap ) 
	{
		// Disable capture power down.
		CAPSENSE_DISABLE_POWERDOWN();
		// Enable capture's NVIC
		NVIC_EnableIRQ(CAPS_IRQn);
		// Enable slow clock.
		CAPSENSE_ENABLE_SLOWCLK();
		// Number of cycles to time a CapSense.
		CAPSENSE_SET_CYCLENUMBER(CAPSENSE_CYCCNT_CYCLENUMS_3072);
		// Set capture pin map.
		CapSense_SetScanPinMap((u16PinEnable=u32PinMap));
		// memory reset for standby ram.
		memset(SBRAM,'\0',sizeof(SBRAM_T));
		// Enable duration count.
		CAPSENSE_ENABLE_DURCNT(CAPSENSE_CTRL_DURATIONCOUNT_3840);
		// Get touch basic value.
		if ( CAPSENSE_Key_Cal()==TRUE )
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CAPSENSE_Key_Start(void)
{
	// Enable interrupt.
	CAPSENSE_ENABLE_INTERRUPT();
	// Enable capture.
	CAPSENSE_ENABLE();
}

void CAPSENSE_Key_Stop(void)
{
	// Disable capture.
	CAPSENSE_DISABLE();
	// Disable interrupt.
	CAPSENSE_DISABLE_INTERRUPT();
}

void CAPSENSE_Key_IRQProcess(void)
{	
	UINT16 au16Tmp[16],u16i,u16Event = 0;
	// Disable scan for get capture value.
	CAPSENSE_DISABLE();
	// Clear interrupt flag.
	CAPSENSE_CLEAR_INT_FLAG();
	// Get value from sram.
	CapSense_GetPinMapValue(au16Tmp);
	// Check if cal state.
	if( i8CalCount > 0 )
	{
		// Get basic value for pre-start.
		for( u16i=0; u16i<16; u16i++ )
		{
			if( (u16PinEnable&(1<<u16i))>0 )
				au32BasicValue[u16i] += au16Tmp[u16i];
		}
		// Add five time for get average basic value.
		if( (i8CalCount-=1)==0 )
		{
			for( u16i=0; u16i<16; u16i++ )
				au32BasicValue[u16i] /= 3;
		}	
		else
			CAPSENSE_Key_Start();
	}
	else
	{
		// Check event for callback function.
		for( u16i=0; u16i<16; u16i++ )
		{
			if( ((u16PinEnable&(1<<u16i))>0) && (au32BasicValue[u16i]>au16Threshold[u16i]) )
			{
				if( (u16PinState&(1<<u16i))>0 )
				{
					if( (au16Tmp[u16i]>(au32BasicValue[u16i]-au16Threshold[u16i])) )
					{
						u16PinState &= ~(1<<u16i);	
						u16Event |= (1<<u16i);
					}
				}
				else 
				{
					if( (au16Tmp[u16i]<(au32BasicValue[u16i]-au16Threshold[u16i])) )
					{
						u16PinState |= (1<<u16i);
						u16Event |= (1<<u16i);
					}					
				}
			}
		}
		
		// Check event exist or not.
		if( u16Event > 0 )
		{
			// Callback event function to user.
			CAPSENSE_Key_Event(u16Event,u16PinState);
		}
		// Start to capture.
		CAPSENSE_Key_Start();
	}
}

// =====================================================================================
void System_Initiate(void)
{
    // Unlock protected registers
    SYS_UnlockReg();
    // Enable clock source 
    CLK_EnableXtalRC(CLK_PWRCTL_HIRC_EN|CLK_PWRCTL_LIRC_EN);
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
    printf("\n+------------------------------------------------------------------------+\r\n");
    printf("|         Capture Sense (Touch Key) Driver Sample Code                    |\r\n");
    printf("+------------------------------------------------------------------------+\r\n");		
}
void Show_KeySetup(BOOL bSuccess)
{
	printf("(1) Setup capture sense to be 'Touch key' and dont press any key at this time.\r\n");
	printf("    1. Set capture scan pin map.\r\n");
	printf("    2. Get touch key basic value.\r\n");
	(bSuccess)?printf("    'Touch key' setup success.\r\n"):printf("    'Touch key' setup fail.\r\n");
}
void Show_KeyStart(void)
{
	printf("(2) Press 'Enter' to start scan pin map whileloop.\r\n");	
	getchar();
}
void Show_DemoWhileloop(void)
{
	printf("(3) 'Touch key' is scan whileloop and feedback key event.\r\n"); 
	while(1);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/

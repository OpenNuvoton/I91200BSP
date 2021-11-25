/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/23 1:52p $
 * @brief    I91200 I2S Driver Sample Code
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "VI2S/VI2S.h"

// Pre-declare function.
void System_Initiate(void);

// I2S API's function. //
void I2S_Initiate_Buffer(void);
BOOL I2S_Master_Setup(void);
void I2S_Master_Configuration(void);
void I2S_Master_Start(void);

// Show message.
void Show_DemoLabel(void);
void Show_MasterSetup(BOOL);
void Show_Start(void);
void Show_End(void);

// Global variable //
UINT32 g_au32SendBuf[64],g_au32ReceiveBuf[128];// Data storage buffer

int main(void)
{
	// Initiate system.
	System_Initiate();
	// Initiate buffer data.
	I2S_Initiate_Buffer();
	
	// Setup I2S.
	// Message : I2S setup success/fail.
	if( I2S_Master_Setup() == TRUE ) 
		Show_MasterSetup(TRUE);
	else{
		Show_MasterSetup(FALSE);
		while(1);
	}
	
	// Configure VI2S_Master
	I2S_Master_Configuration();
	
	// Message : When user press 'SWB0' button key, I2S will start operating.
	Show_Start();
	// Start I2S process.
	I2S_Master_Start();
	
	// Wait I2S process finish.
	VI2S_WaitComplete(0);
	
	// Verify I2S Master demo.
	Show_End();
		
	while(1);
}

void I2S0_IRQHandler(void)
{
	UINT32 u32State;
	
	u32State = VI2S_Process(0);
	
	// If using return State Flag, user can check u32State to get which interrupt state were occurred.
	// If not using State Flag, u32State will be 0x00, and enter Callback Function.
	// VI2S_PROCESSEVENT = 1: return State Flag;  VI2S_PROCESSEVENT = 0: Callback function.
	if(u32State & VI2S_STA_SENDCOMP)// Send complete.
		;
	if(u32State & VI2S_STA_RECCOMP)// Receive complete.
		;
	if(u32State & VI2S_STA_RECREQ)// Receive request.
		// Set g_au8ReceiveBuf to VUART0 for receive data.
		VI2S_SetRxData(0,g_au32ReceiveBuf,64);
	if(u32State & VI2S_STA_LEFTZERO)// Left channel zero occurred.
		;
	if(u32State & VI2S_STA_RIGHTZERO)// Right channel zero occurred.
		;
}

// Callback Function ==================================================================
// VI2S provide callback API when data almost output completely and request input data.
// User could add some action here.
// Note. This callback function was in I2S's IRQ.
void VI2S_SendComplete(UINT8 u8VI2SNo)
{
};

void VI2S_ReceiveComplete(UINT8 u8VI2SNo)
{	
};

void VI2S_ReceiveRequest(UINT8 u8VI2SNo)
{	
};

// I2S Master =========================================================================
// I2S_Master Definition.
#define I2S0_PINS_MSK    	(SYS_GPA_MFP_PA0MFP_Msk|SYS_GPA_MFP_PA1MFP_Msk|SYS_GPA_MFP_PA2MFP_Msk|SYS_GPA_MFP_PA3MFP_Msk)
#define I2S0_PINS_FUNC		(SYS_GPA_MFP_PA0MFP_I2S0_FS|SYS_GPA_MFP_PA1MFP_I2S0_BCLK|SYS_GPA_MFP_PA3MFP_I2S0_SDO|SYS_GPA_MFP_PA2MFP_I2S0_SDI)

void I2S_Initiate_Buffer(void)
{
	UINT8 u8i;
	
	for( u8i=0; u8i<64; u8i++ )
	{
		g_au32SendBuf[u8i] = 0xABCDEF00+u8i;
	}
	for( u8i = 0; u8i<128; u8i++)
	{
		g_au32ReceiveBuf[u8i] = 0;
	}
}

BOOL I2S_Master_Setup(void)
{
	// Open I2S0 as master; sampling rate 16000; reset
	if(VI2S_Open(0,1,16000, E_VI2S_CLK_HIRC, TRUE))
	{
		// Config GPA0~3 for I2S
		SYS->GPA_MFP = (SYS->GPA_MFP &~I2S0_PINS_MSK) | I2S0_PINS_FUNC;
		// Config GPA15 for master clock
		SYS->GPA_MFP = (SYS->GPA_MFP &~SYS_GPA_MFP_PA15MFP_Msk) | SYS_GPA_MFP_PA15MFP_MCLK;
		
		return TRUE;
	}
	return FALSE;
}

void I2S_Master_Configuration(void)
{
	VI2S_Config(0,VI2S_CFG_MASTER|VI2S_CFG_STEREO|VI2S_CFG_WORD16|VI2S_CFG_FORMATI2S|VI2S_CFG_TXTH4|VI2S_CFG_RXTH4);
}

void I2S_Master_Start(void)
{
	// Set g_au8SendBuf to VI2S for send data.
	VI2S_SetTxData(0,g_au32SendBuf,64);
	// Set g_au8ReceiveBuf to VI2S0 for receive data.
	VI2S_SetRxData(0,g_au32ReceiveBuf,128);
	// Start VI2S_Master
	VI2S_Start(0);
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
	// Config gpio pin multi-functon.
	SYS->GPB_MFP = ( SYS->GPB_MFP & ~SYS_GPB_MFP_PB0MFP_Msk ) | SYS_GPB_MFP_PB0MFP_GPIO;
	// Config gpio debounce clock source & debounce time.
	GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_512);
	// Enable debounce pin.
  GPIO_ENABLE_DEBOUNCE( PB, BIT0);
	// Config gpio mode.
	GPIO_SetMode( PB, BIT0, GPIO_MODE_QUASI);
	// Light off LED DA12 and DA13.
	GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)|(BIT12));
  GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)|(BIT13));
	// Lock protected registers
	SYS_LockReg();
}

// Print Message =======================================================================
void Show_DemoLabel(void)
{
	printf("\r\n+------------------------------------------------------------------------+\r\n");
	printf("|                      I2S Driver Sample Code                           |\r\n");
	printf("+------------------------------------------------------------------------+\r\n");
}
void Show_MasterSetup(BOOL bSuccess)
{
	printf("(1) Setup I2S Master function.\r\n");
	printf("    1. Open I2S0 interface(sampling rate = 16000).\r\n");
	printf("    2. Config I2S0's gpio multi-function.\r\n");
	(bSuccess)?printf("    VI2S setup success.\r\n"):printf("    VI2S setup fail.\r\n");
}

void Show_Start()
{
	printf("(2) Please make sure GPA0 & GPA1 & GPA2 & GPA3 connected.\r\n");	
	printf("    (Press 'SWB0' button to start send data, the LED DA13 will be light on.)\r\n");
	// Wait user press key on EVB board.
	while( (GPIO_GET_IN_DATA(PB)&BIT0)==BIT0 );
	// Light on DA13
	GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)&(~BIT13));
}

void Show_End(void)
{
	printf("(3) Receive data verity ..."); 
	// Compare g_au8SendBuf & g_au8ReceiveBuf.
	if( memcmp(g_au32SendBuf,g_au32ReceiveBuf+2,64) != 0 )
		printf("fail.\r\n");
	else
	{
		printf("success.\r\n");
		GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)&(~BIT13));
	}
	printf("(4) I2S demo end.\r\n");
}

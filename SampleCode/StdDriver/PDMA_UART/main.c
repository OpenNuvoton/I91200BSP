/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/06/26 3:30p $
 * @brief    I91200 PDMA Driver Sample Code
 *
 * @note	Please do not use Nu-Link Pro to try this demo code.
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "VPDMA/VPDMA.h"
#include "VUART/VUART.h"

#define PDMA_CH_TX   	VPDMA_CHANNEL0
#define PDMA_CH_RX		VPDMA_CHANNEL1
#define BUFF_COUNT		(16)
__align(4) UINT8 g_au8Source[BUFF_COUNT], g_au8Destination[BUFF_COUNT]; // Must be algined at four byte

// Pre-declare function. //
void System_Initiate(void);
void Buffer_Initiate(void);
void UART_Init(void);

// PDMA API's function //
void PDMA_Setup(void);
void PDMA_Configuration(void);
void PDMA_Start(void);

// Show message.
void Show_DemoLabel(void);
void Show_VUARTSetup(void);
void Show_VPDMASetup(void);
void Show_Start(void);
void Show_End(void);

// Global variable //
volatile UINT8  g_u8TransCnt_TX, g_u8TransCnt_RX;

int32_t main(void)
{
	// Initiate system clock.
	System_Initiate();
	// Initiate source and destination buffer.
	Buffer_Initiate();
	// Message : PDMA demo start.
	Show_DemoLabel();
	
	// Message : VUART setup.
	Show_VUARTSetup();
	// Init UART0 module and clock.
	// Before using VPDMA, Initialize the UART.
	UART_Init();
	
	// Setup PDMA.
	PDMA_Setup();
	// Message : PDMA setup done.
	Show_VPDMASetup();
	
	// Configure PDMA.
	// User can change configuration in this API.
	PDMA_Configuration();
	
	// Message : PDMA will start operating in one-shot mode.
	Show_Start();
	// Statrt PDMA in one-shot mode.
	PDMA_Start();
	
	while(g_u8TransCnt_RX || g_u8TransCnt_TX); //do something in main routine
	
	// Message : PDMA transfer finsihed.
	Show_End();
	
	while (1);
}

// PDMA interrupt handler
void PDMA_IRQHandler(void)
{
	UINT32 u32State;
	
	u32State = VPDMA_Process(PDMA_CH_TX);
	// If using return State Flag, user can check u32State to get which interrupt state were occurred.
	// If not using State Flag, u32State will be 0x00, and enter Callback Function.
	// VPDMA_PROCESSEVENT = 1: return State Flag;  VPDMA_PROCESSEVENT = 0: Callback function.
	if(u32State & VPDMA_STA_ONESHOT)
	{
		if((g_u8TransCnt_TX-=1) != 0)
			VPDMA_Start(PDMA_CH_TX);
	}if(u32State & VPDMA_STA_WRAPHALF)
		;
	if(u32State & VPDMA_STA_WRAPFULL)
		;
	
	u32State = VPDMA_Process(PDMA_CH_RX);
	// Using return state to handle interrupt.
	if(u32State & VPDMA_STA_ONESHOT)
	{
		if((g_u8TransCnt_RX -= 1) != 0)
			VPDMA_Start(PDMA_CH_RX);
	}if(u32State & VPDMA_STA_WRAPHALF)
		;
	if(u32State & VPDMA_STA_WRAPFULL)
		;
}

// Using VPDMA callback function to handle interrupt.
void VPDMA_OneShot(UINT8 u8PDMAChNo)
{
	if(u8PDMAChNo == PDMA_CH_TX)
	{
		if((g_u8TransCnt_TX-=1) != 0)
			VPDMA_Start(PDMA_CH_TX);
	}
	else if(u8PDMAChNo == PDMA_CH_RX)
	{
		if((g_u8TransCnt_RX -= 1) != 0)
			VPDMA_Start(PDMA_CH_RX);
	}
}

void VPDMA_WrapEnd(UINT8 u8PDMAChNo)
{
	if(u8PDMAChNo == PDMA_CH_TX)
	{
		if((g_u8TransCnt_TX-=1) == 0)
			VPDMA_Stop(PDMA_CH_TX);
	}
	else if(u8PDMAChNo == PDMA_CH_RX)
	{
		if((g_u8TransCnt_RX-=1) == 0)
			VPDMA_Stop(PDMA_CH_RX);
	}
};

// PDMA's API===========================================================================
void PDMA_Setup(void)
{
	VPDMA_Open();
}

void PDMA_Configuration(void)
{
	// Configuration for UART0_TX channel
	VPDMA_Config(PDMA_CH_TX, VPDMA_CFG_UART0_TX(VPDMA_CFG_WIDTH_8, VPDMA_CFG_WRAPAROUND_FULL), BUFF_COUNT, (UINT32)g_au8Source, NULL);
	// Configuration for UART0_RX channel
	VPDMA_Config(PDMA_CH_RX, VPDMA_CFG_UART0_RX(VPDMA_CFG_WIDTH_8, VPDMA_CFG_WRAPAROUND_FULL), BUFF_COUNT, NULL, (UINT32)g_au8Destination);
}

void PDMA_Start(void)
{
	// Set total transfer count.
	g_u8TransCnt_TX = 2;
	g_u8TransCnt_RX = g_u8TransCnt_TX;
	
	// Start to transfer
	VPDMA_Start(PDMA_CH_TX);
	// Start to receive
	VPDMA_Start(PDMA_CH_RX);
}

// =====================================================================================
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

void Buffer_Initiate(void)
{
	int32_t i;
	
	memset(g_au8Source,0,BUFF_COUNT);
	memset(g_au8Destination,0,BUFF_COUNT);
	for (i=0;i<BUFF_COUNT;i++) g_au8Source[i]=i;
}

void UART_Init(void)
{
	// Open VUART module.
	VUART_Open(0, 115200, E_VUART_CLK_DEFAULT, TRUE);
	// Set GPA and GPA as TX and RX.
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~(SYS_GPA_MFP_PA4MFP_Msk|SYS_GPA_MFP_PA5MFP_Msk)) ) | (SYS_GPA_MFP_PA4MFP_UART0_TX|SYS_GPA_MFP_PA5MFP_UART0_RX);
	
	// TX/RX function using VPDMA. Not using VUART interrupt.
	VUART_Config(0, VUART_CFG_TXINTDIS|VUART_CFG_RXINTDIS);
	
	// Start VUART0
	VUART_Start(0);
}

// Show Message ========================================================================
void Show_DemoLabel()
{
	printf("\r\n+------------------------------------------------------------------------+\r\n");
	printf("|                      PDMA Driver Sample Code              	         |\r\n");
	printf("+------------------------------------------------------------------------+\r\n");
}

void Show_VUARTSetup(void)
{
	printf("(1) Setup VUART.\r\n");
	printf("    1. Config baud at 115200.\r\n");
}

void Show_VPDMASetup()
{
	printf("(2) Setup VPDMA to UART.\r\n");
	printf("    1. Set VPDMA Channel0 to UART TX function.\r\n");
	printf("    2. Set VPDMA Channel1 to UART RX function.\r\n");
	printf("    3. Config word length as 8-bit.\r\n");
	printf("    4. Config VPDMA as wraparound-full mode.\r\n");
}
	
void Show_Start()
{
	printf("(3) 1.Connect PA4(TX) and PA5(RX) together....\r\n");
	printf("	2.Press 'Enter' to start transfer UART data by PMDA....\r\n");
	getchar();
}

void Show_End()
{
	if( memcmp(g_au8Source,g_au8Destination,BUFF_COUNT) != 0 )
		printf("Verify fail.\r\n");
	else
		printf("Verify success.\r\n");
	
	printf("(4)	PDMA demo end.\r\n");
	VPDMA_Stop(PDMA_CH_TX);
	VPDMA_Stop(PDMA_CH_RX);
	VPDMA_Close();
}

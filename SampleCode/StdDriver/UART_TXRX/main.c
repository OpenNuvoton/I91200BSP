/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/01/16 2:52p $
 * @brief    I91200 UART Driver Sample Code
 * @note	 Please do not use Nu-Link Pro to try this demo code.
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "VUART/VUART.h"

// Pre-declare function. //
void System_Initiate(void);

// UART API's function. //
void UART_Initiate_Buffer(void);
BOOL UART_Setup(void);
void UART_Configuration(void);
void UART_Start(void);

// Show message.
void Show_DemoLabel(void);
void Show_Setup(BOOL);
void Show_Start(void);
void Show_End(void);

// Global variable //
UINT8 g_au8SendBuf[64],g_au8ReceiveBuf[64];
UINT8 g_u8Temp = 0;

int main(void)
{
	// Initiate system.
	System_Initiate();
	// Initiate buffer data.
	UART_Initiate_Buffer();
	// Message : demo label.
	Show_DemoLabel();
	
	// Setup UART TX/RX function.
	// Message : UART setup success/fail.
	if( UART_Setup() == TRUE ) 
		Show_Setup(TRUE);
	else
	{
		Show_Setup(FALSE);
		while(1);
	}

	// Configure UART.
	UART_Configuration();
	
	// Message : When user press 'ENTER', UART will start operating.
	Show_Start();
	// Start UART TX/RX function.
	UART_Start();
	
	// Wait TX/RX process finish. //
	while(VUART_IsBusy(0))
	{
		// Use UART with polling method
		//VUART_Process(0);
	}
	
	// Verify UART demo. //
	Show_End();

	while(1);
}

void UART0_IRQHandler(void)
{
	UINT32 u32State;
	
	u32State = VUART_Process(0);
	
	// If using return State Flag, user can check u32State to get which interrupt state were occurred.
	// If not using State Flag, u32State will be 0x00, and enter Callback Function.
	// VUART_PROCESSEVENT = 1: return State Flag;  VUART_PROCESSEVENT = 0: Callback function.
	if(u32State & VUART_STA_BUFERR)// Buffer error occurred.
		// Stop UART when buffer error occurred.
		VUART_Stop(0);
	if(u32State & VUART_STA_RXTO)// RX buffer time-out occurred.
		;
	if(u32State & VUART_STA_SENDCOMP)// Send complete.
		// Set flag for showing message.
		g_u8Temp |= 0x10;
	if(u32State & VUART_STA_RECCOMP)// Receive complete.
		// Set flag for showing message.
		g_u8Temp |= 0x01;
	if(u32State & VUART_STA_RECREQ)// Receive request.
		// Set g_au8ReceiveBuf to VUART0 for receive data.
		VUART_SetRxData(0,g_au8ReceiveBuf,64);
}

// Callback Function ==================================================================
// VUART provide callback API when data almost output completely and request input data.
// User could add some action here.
// Note. This callback function was in UART's IRQ.
void VUART_BufferError(UINT8 u8VUARTNo)
{
	// Buffer error occurred.
	// Stop UART when buffer error occurred.
	VUART_Stop(u8VUARTNo);
}

void VUART_ReceiveTimeOut(UINT8 u8VUARTNo)
{
	// RX buffer time-out occurred.
}

void VUART_ReceiveComplete(UINT8 u8VUARTNo)
{
	// Receive complete.
	// Set flag for showing message.
	g_u8Temp |= 0x01;
}

void VUART_SendComplete(UINT8 u8VUARTNo)
{
	// Send complete.
	// Set flag for showing message.
	g_u8Temp |= 0x10;
}

void VUART_ReceiveRequest(UINT8 u8VUARTNo)
{
	// Receive request.
	// Set g_au8ReceiveBuf to VUART0 for receive data.
	VUART_SetRxData(0,g_au8ReceiveBuf,64);
}

// UART ===============================================================================
#define UART0_PINS_MSK    (SYS_GPA_MFP_PA4MFP_Msk|SYS_GPA_MFP_PA5MFP_Msk)
#define UART0_PINS        (SYS_GPA_MFP_PA4MFP_UART0_TX|SYS_GPA_MFP_PA5MFP_UART0_RX)	

void UART_Initiate_Buffer(void)
{
	UINT8 u8i;
	
	for( u8i=0; u8i<64; u8i++ )
	{
		g_au8SendBuf[u8i] = u8i;
		g_au8ReceiveBuf[u8i] = 0;
	}
}

BOOL UART_Setup(void)
{	
	// Open UART and set Baud to 57600Hz 
	if( VUART_Open(0, 115200, E_VUART_CLK_DEFAULT, TRUE) )
	{
		// Init I/O multi-function.
		// UART0: GPA4=TX, GPA5= RX
		SYS->GPA_MFP = (SYS->GPA_MFP & (~UART0_PINS_MSK) ) | UART0_PINS;
		return TRUE;
	}
	return FALSE;
}

void UART_Configuration(void)
{
	// Configure VUART0 
	VUART_Config(0,VUART_CFG_RXLVL4INTEN|VUART_CFG_FIFORXRST|VUART_CFG_FIFOTXRST|VUART_CFG_BUFERRINTEN|VUART_CFG_TXINTEN|VUART_CFG_RXTOINTEN);
	VUART_Config(0,VUART_CFG_WORDL8BITS|VUART_CFG_1STOPBITS|VUART_CFG_EVENPARITY);
	// Set time-out counter
	VUART_SetTimeOut(0, 0x07);
}

void UART_Start(void)
{
	// Set g_au8SendBuf(data content is 0~63) to VUART0 for send data.
	VUART_SetTxData(0,g_au8SendBuf,64);
	
	VUART_SetRxData(0,g_au8ReceiveBuf,64);
	// Start VUART0
	VUART_Start(0);
}

// =====================================================================================
void System_Initiate(void)
{
	// Unlock protected registers
	SYS_UnlockReg();
	// Enable clock source 
	CLK_EnableXtalRC(CLK_PWRCTL_HIRC_EN);
	// Switch HCLK clock source to HIRC
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	// Enable LDO 3.3V
	CLK_EnableLDO(CLK_LDOSEL_3_3V);
	// Update System Core Clock
	SystemCoreClockUpdate();
	// Lock protected registers
	SYS_LockReg();
}

// Show Message =======================================================================
void Show_DemoLabel()
{
	printf("\r\n+------------------------------------------------------------------------+\r\n");
	printf("|                      UART Driver Sample Code                           |\r\n");
	printf("+------------------------------------------------------------------------+\r\n");
}
void Show_Setup(BOOL bSuccess)
{
	printf("(1) Setup UART0 Tx/Rx function.\r\n");
	printf("    1. Open VUART0 interface(frequency = 57600).\r\n");
	printf("    2. Config UART0's gpio multi-function.\r\n");
	(bSuccess)?printf("    VUART setup success.\r\n"):printf("    VUART setup fail.\r\n");
}

void Show_Start()
{
	printf("(2) Please make sure GPA4 & GPA5 connect.\r\n");	
	printf("    (Press 'Enter' to start send & receive data)\r\n");
	getchar();

	printf("(3) Start transform data(0~63) from Tx(GPA4) to Rx(GPA5).\r\n");
}

void Show_End()
{
	if(g_u8Temp & 0x10)
		printf("Send Complete\r\n");
	if(g_u8Temp & 0x01)
		printf("Receive Complete\r\n");
	
	printf("(4) Send & receive data verity ...\r\n");
	
	if( memcmp(g_au8SendBuf,g_au8ReceiveBuf,64) != 0 )
		printf("fail.\r\n");
	else
		printf("success.\r\n");
	
	printf("(5) UART demo end.\r\n"); 
}

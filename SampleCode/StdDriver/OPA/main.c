/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/01/23 2:52p $
 * @brief    Demo sample of Operational Amplifier.
 *
 * @note
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"

// OPA Pin define
#define PIN_A0P			(BIT0)
#define PIN_A0N			(BIT1)
#define PIN_A0X			(BIT2)
#define PIN_A1P			(BIT3)
#define PIN_A1N			(BIT4)
#define PIN_A1X			(BIT5)
#define PIN_CNP			(BIT6)
#define PIN_C1N			(BIT7)
#define PIN_C2P			(BIT8)

#define OPA_VREF		(OPA_POSIN_VBIAS_MID)

#define BUTTON_SWB5		(BIT5)
#define BUTTON_SWB4		(BIT4)
#define LED_DA10		(BIT10)

/* API pre-define */
void OPA_Init(uint8_t u8Char);
void CMP_Init(uint8_t u8Char);
void UART0_Init(void);
void System_Initiate(void);

void Delay(int nCount)
{
	while(nCount>0)  nCount--;
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	uint8_t u8Char;
	
	System_Initiate();
	UART0_Init();
	
	printf("\r\nCPU @ %d Hz\r\n", SystemCoreClock);
    printf("+------------------------------------------------+\r\n");
    printf("|  Demo Sample of OPA and Comparator.            |\r\n");
    printf("+------------------------------------------------+\r\n");
	
	printf("Select OPA and Comparator configuration.\r\n");
	printf("0: OPA0 is configured as comaprator.\r\n");
    printf("1: OPA0 is configured as voltage follower(buffer).\r\n");
	printf("2: OPA0 is configured as buffer and cascaded to OPA1.\r\n");
	printf("   OPA1 is configured as non-inverting amplifier.\r\n");
	printf("3: Enable CMP1. Input from CNP & C1N.\r\n");
    printf("4: Enable CMP2. Input from OPA0(buffer) & VL0.\r\n");
    printf("+------------------------------------------------+\r\n");
	
	u8Char = getchar();
	
	OPA_Init(u8Char);
	
	CMP_Init(u8Char);

	while(1);
}

// Button interrupt handler
// This would toggle variable: g_u8FlagALC, g_u8FlagVol to indicate whether the function is enabled or not.
void CMP_IRQHandler()
{
	if(CMP_GET_INT())
	{
		// Toggle LED_DA10
		GPIO_SET_OUT_DATA(PA,GPIO_GET_OUT_DATA(PA)^LED_DA10);
		CMP_CLEAR_INT();
	}
	
	NVIC_ClearPendingIRQ(CMP_IRQn);
}

void OPA_Init(uint8_t u8Char)
{
	SYS_ResetModule(ANA_MODULE);
	CLK_EnableModuleClock(ANA_MODULE);
	CapSense_SetScanPinMap(PIN_A0N | PIN_A0X | PIN_A0P | PIN_A1N | PIN_A1X | PIN_A1P);
	
	switch(u8Char)
	{
		case '0':
			printf("Please connect OPA0N to input.\nPress any key to continue.\r\n\r\n");
			getchar();
			if(! OPA_Enable(OPA0, OPA_POSIN_VBIAS_MID, OPA_NEGIN_PIN, 1, 0) ) 
				printf("Initial OPA0 fail.\r\n");
			break;
		case '1':
			printf("Please connect OPA0P to input.\nPress any key to continue.\r\n\r\n");
			getchar();
			if(! OPA_Enable(OPA0, OPA_POSIN_PIN, OPA_NEGIN_NOCONNECT, 1, 1) )
				printf("Initial OPA1 fail.\r\n");
			break;
		case '2':
			printf("Please connect OPA1N to ground.\nPress any key to continue.\r\n\r\n");
			getchar();
			if(! OPA_Enable(OPA0, OPA_POSIN_PIN, OPA_NEGIN_NOCONNECT, 1, 1) ) 
				printf("Initial OPA0 fail.\r\n");
			if(! OPA_Enable(OPA1, OPA_POSIN_OPA, OPA_NEGIN_PIN, 1, 0) )
				printf("Initial OPA1 fail.\r\n");
			
			OPA_GainEnable(OPA_GAIN_8);
			break;
		default:
			break;
	}
}

void CMP_Init(uint8_t u8Char)
{
	SYS_ResetModule(ANA_MODULE);
	CLK_EnableModuleClock(ANA_MODULE);
	CapSense_SetScanPinMap(PIN_CNP|PIN_C1N|PIN_C2P);
	
	switch(u8Char)
	{
		case '3':
			printf("Please connect CNP and C1N to input.\nPress any key to continue.\r\n\r\n");
			getchar();
			if(! CMP_Enable(CMP1, CMP1_POSIN_CNP, CMP1_NEGIN_C1N, 0) ) 
				printf("Initial CMP1 fail.\r\n");
			
			CMP_IntEnable(CMP1, CMP_DUAL_EDGE);
			
			NVIC_ClearPendingIRQ(CMP_IRQn);
			NVIC_EnableIRQ(CMP_IRQn);
			break;
		case '4':
			printf("Please connect OPA0P to input to work as buffer.\nPress any key to continue.\r\n\r\n");
			getchar();
			if(! OPA_Enable(OPA0, OPA_POSIN_PIN, OPA_NEGIN_NOCONNECT, 1, 1) )
				printf("Initial OPA1 fail.\r\n");
			if(! CMP_Enable(CMP2, CMP2_POSIN_VL0, CMP2_NEGIN_OPA0, 0) ) 
				printf("Initial CMP2 fail.\r\n");
			
			CMP_IntEnable(CMP2, CMP_DUAL_EDGE);
			NVIC_ClearPendingIRQ(CMP_IRQn);
			NVIC_EnableIRQ(CMP_IRQn);
			break;
		default:
			break;
	}
}

void UART0_Init(void)
{
    // Enable SARADC module clock. 
	CLK_EnableModuleClock(UART0_MODULE);
	
	// Reset UART module
    SYS_ResetModule(UART0_RST);
	
	// Peripheral clock source
    CLK_SetModuleClock(UART0_MODULE, MODULE_NoMsk, CLK_CLKDIV0_UART0(1));
	    
    // Init I/O multi-function ; UART0: GPA4=TX, GPA5= RX
	SYS->GPA_MFP  = (SYS->GPA_MFP & ~(SYS_GPA_MFP_PA4MFP_Msk|SYS_GPA_MFP_PA5MFP_Msk) ) |  (SYS_GPA_MFP_PA4MFP_UART0_TX|SYS_GPA_MFP_PA5MFP_UART0_RX)	;

    // Configure UART0 and set UART0 Baudrate
    UART_Open(UART0, 115200);
}

// Set button function for PB4, PB5.
void Button_Init(void)
{	
	GPIO_SetMode(PB, BIT2, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PB, BIT2, GPIO_MODE_INPUT);
	GPIO_EnableInt(PB, 2, GPIO_INT_FALLING);
	// 2. Config gpio debounce clock source & debounce time.
	GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_512);
	// 3. Enable debounce pin.
	GPIO_ENABLE_DEBOUNCE( PB, BIT2);
	
	NVIC_ClearPendingIRQ(GPAB_IRQn);
	NVIC_EnableIRQ(GPAB_IRQn);
}

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

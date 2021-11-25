/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/01/23 2:52p $
 * @brief    Demo Aanlog or Digital Mic input to SDADC
 *
 * @note
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"

#define SAMPLE_RATE			(16000)
#define TARGET_VOL			(eVOLCTRL_6_DB)
#define SDADC_INT_SAMPLES	(4)			//There are 4 words data in FIFO buffer, SDADC threshold will generate a interrupt

#define VOL_RAMPUP_RATIO_STEP	((1<<VOLCTRL_FIXEDPOINT_SHIFT)*0.1)		// Volume ratio step: 262144*0.1
#define VOL_RAMPUP_MS			(150)									// Unit: ms
#define VOL_RAMPUP_SPEED		(SAMPLE_RATE/1000)*(VOL_RAMPUP_MS)		// Unit: Samples

volatile uint32_t g_u32SampleCnt, u32SampleRate;
volatile uint32_t g_u32CurrentVol = eVOLCTRL_NEG30_DB;

void SDADC_IRQHandler(void)
{
	uint8_t u8i = SDADC_INT_SAMPLES;
	
	if (SDADC_GetIntFlag(SDADC_FIFO_INT))
	{
		do
		{
			while(DPWM_IS_FIFOFULL(DPWM)); // check DPWM FIFO whether full
			DPWM->DATA = SDADC_GET_FIFODATA(SDADC);
		}while(--u8i>0);
		
		// Do Ramp up
		if (g_u32CurrentVol!=TARGET_VOL)
		{
			g_u32SampleCnt+=SDADC_INT_SAMPLES;
			if (g_u32SampleCnt>=VOL_RAMPUP_SPEED)
			{
				g_u32SampleCnt = 0;
				g_u32CurrentVol+=VOL_RAMPUP_RATIO_STEP;
				if (g_u32CurrentVol>=TARGET_VOL)
					g_u32CurrentVol = TARGET_VOL;
				VOLCTRL_SET_VOLUMERATION(VOLCTRL, VOLCTRL_SDADC_SELECTION, g_u32CurrentVol);
			}
		}
	}
}

void Delay(int nCount)
{
	while(nCount>0)  nCount--;
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

void MIC_Init(int8_t i8Option)
{
	// Enable SDADC module clock. 
	CLK_EnableModuleClock(SDADC_MODULE);
	SYS_ResetModule(SDADC_RST);
	
	if (i8Option == '1')
	{
		/* The Following Means I91200BS just Support DMIC */
		#if !defined(__I91200BS__)
			// Enable analog block clock. 
			CLK_EnableModuleClock(ANA_MODULE);
			Delay(1000);
			// set the VMID Reference for analog block
			CLK_ENABLE_VMID(CLK, CLK_VMID_HIRES_DISCONNECT, CLK_VMID_LORES_CONNECT);
			// Enable the Microphone Bias
			SDADC_EnableMICBias(SDADC_MICBSEL_1P8V);
			Delay(100000);
			// SDADC power on
			SDADC_ANALOG_POWERON(SDADC);
			// Enable PGA and set 12dB gain value
			SDADC_ENABLE_PGA(SDADC, SDADC_PGACTL_GAIN_12DB);
			// PGA signal mute off
			SDADC_MUTEOFF_PGA(SDADC);
		#endif
	}else if (i8Option == '0')
	{
		// Set GPB12 for DMIC data pin; GPB14 for DMIC clk pin; 
		SYS->GPB_MFP =(SYS->GPB_MFP &  ~(SYS_GPB_MFP_PB12MFP_Msk|SYS_GPB_MFP_PB14MFP_Msk))| SYS_GPB_MFP_PB12MFP_DMIC_DAT | SYS_GPB_MFP_PB14MFP_DMIC_CLK;
		// Set GPB14 output mode for DMIC clk; 
		GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);
		// Enable DMIC function from GPIO
		SDADC_ENABLE_DMIC(SDADC);
	}
	// Set sample rate = 16000
	u32SampleRate = SDADC_SetAudioSampleRate(SAMPLE_RATE);
	
	printf("SDADC Sample rate %d Hz.\n\n", u32SampleRate);
	
	// Set 16-bit data selection for FIFO buffer 
	SDADC_SET_FIFODATABITS(SDADC, SDADC_FIFODATA_16BITS);
	// Determines the 4 words data in FIFO will generate a interrupt.
	SDADC_SET_FIFOINTLEVEL(SDADC, SDADC_INT_SAMPLES);
	// Enable FIFO Threshold interrupt
	SDADC_EnableInt(SDADC_FIFO_INT);
	
	// Enable NVIC 
	NVIC_ClearPendingIRQ(SDADC_IRQn);
	NVIC_EnableIRQ(SDADC_IRQn);
}

void Speaker_Init(void)
{
	// Enable DPWM module clock. 
	CLK_EnableModuleClock(DPWM_MODULE);
	SYS_ResetModule(DPWM_RST);
	
	// Set DPWM clock source: HIRC = 49.152M
	CLK_SetModuleClock(DPWM_MODULE, CLK_CLKSEL1_DPWMSEL_HCLK, CLK_CLKDIV0_DPWM(1));
	// Set sample rate = 16000
	DPWM_SetSampleRate(SAMPLE_RATE);
	// Set 16-bit data selection for FIFO buffer
	DPWM_SET_FIFODATAWIDTH(DPWM, DPWM_FIFO_DATAWIDTH_16BITS);
	// Enable DPWM driver
	DPWM_ENABLE_DRIVER(DPWM);
}

void Volume_Init(void)
{
	// The volume control shares a clock source with the BIQ filter, so user needs to
    // call CLK_EnableModuleClock and BIQ_LoadDefaultCoeff to enable volume control IP
	CLK_EnableModuleClock(BFAL_MODULE);
	SYS_ResetModule(BIQ_RST);
	BIQ_LoadDefaultCoeff();
	
	// Set min dB volume to ramp up
	VOLCTRL_Set_SDADCVol((E_VOLCTRL_DB)g_u32CurrentVol);
	g_u32SampleCnt = 0;
}

void UART0_Init(void)
{
    // Enable SARADC module clock. 
	CLK_EnableModuleClock(UART0_MODULE);
	
	/* Reset UART module */
    SYS_ResetModule(UART0_RST);
	
	/* Peripheral clock source */
    CLK_SetModuleClock(UART0_MODULE, MODULE_NoMsk, CLK_CLKDIV0_UART0(1));
	    
    // Init I/O multi-function ; UART0: GPA4=TX, GPA5= RX
	SYS->GPA_MFP  = (SYS->GPA_MFP & ~(SYS_GPA_MFP_PA4MFP_Msk|SYS_GPA_MFP_PA5MFP_Msk) ) |  (SYS_GPA_MFP_PA4MFP_UART0_TX|SYS_GPA_MFP_PA5MFP_UART0_RX)	;

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	int8_t item;
	System_Initiate();
	UART0_Init();
	
	printf("\r\n\r\nCPU @ %d Hz\r\n", SystemCoreClock);
  printf("+-------------------------------------------------+\r\n");
  printf("|  Demo Aanlog or Digital Mic input to SDADC	  |\r\n");
  printf("+------------------------------------------------+\r\n");
	/* The Following Means I91200BS just Support DMIC */
	#if !defined(__I91200BS__)
		printf("+Analog selcets option: 1 .\r\n");
	#endif
	printf("+Digital selcets option: 0 .\r\n");
	item = getchar();
	MIC_Init(item);
	Speaker_Init();
	Volume_Init();
	
	SDADC_START_CONV(SDADC);
	DPWM_START_PLAY(DPWM);
	
	while(1);
	
}

/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/01/23 2:52p $
 * @brief    Demo Aanlog Mic input to SDADC and using ALC to output
 *
 * @note
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"

/* SDADC define */
#define SAMPLE_RATE			(16000)
#define TARGET_VOL			(eVOLCTRL_6_DB)
#define SDADC_INT_SAMPLES	(4)			//There are 4 words data in FIFO buffer, SDADC threshold will generate a interrupt

/* Volume control define */
#define VOL_RAMPUP_RATIO_STEP	((1<<VOLCTRL_FIXEDPOINT_SHIFT)*0.1)		// Volume ratio step: 262144*0.1
#define VOL_RAMPUP_MS			(150)									// Unit: ms
#define VOL_RAMPUP_SPEED		(SAMPLE_RATE/1000)*(VOL_RAMPUP_MS)		// Unit: Samples

/* ALC define */
#define TARGET_LEVEL_VOLCTLON		(-1800) // ALC target level while volume control is on.
#define TARGET_LEVEL_VOLCTLOFF		(-600)  // ALC target level while volume control is off.
#define ALC_MIN_GAIN				(-1200) // ALC minimum gain.
#define ALC_MAX_GAIN_VOLCTLON		(2300)  // ALC maximum gain while volume control is on.
#define ALC_MAX_GAIN_VOLCTLOFF		(3500)  // ALC maximum gain while volume control is off.
#define BUTTON_SWB5					BIT5
#define BUTTON_SWB4					BIT4

/* SDADC, Volume control variable */
volatile uint32_t g_u32SampleCnt, u32SampleRate;
volatile uint32_t g_u32CurrentVol = eVOLCTRL_NEG30_DB;

/* ALC variable */
uint8_t g_u8FlagALC, g_u8FlagVol;
uint32_t g_u32MaxGain, g_u32MinGain, g_u32TartgetLevel;

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
	
	printf("SDADC Sample rate %d Hz.\r\n\r\n", u32SampleRate);
	
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
	VOLCTRL_Set_SDADCVol((E_VOLCTRL_DB)eVOLCTRL_NEG6_DB);
	g_u32CurrentVol = eVOLCTRL_NEG6_DB;
	g_u32SampleCnt = 0;
	g_u8FlagVol = 1;
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
// Set button function for PB4, PB5.
void Button_Init(void)
{
	GPIO_SetMode(PB, BUTTON_SWB5, GPIO_MODE_QUASI);
	GPIO_EnableInt(PB, 5, GPIO_INT_FALLING);
	// 2. Config gpio debounce clock source & debounce time.
	GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_512);
	// 3. Enable debounce pin.
	GPIO_ENABLE_DEBOUNCE( PB, BUTTON_SWB5);
	
	GPIO_SetMode(PB, BUTTON_SWB4, GPIO_MODE_QUASI);
	GPIO_EnableInt(PB, 4, GPIO_INT_FALLING);
	// 2. Config gpio debounce clock source & debounce time.
	GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_512);
	// 3. Enable debounce pin.
	GPIO_ENABLE_DEBOUNCE( PB, BUTTON_SWB4);
	
	NVIC_ClearPendingIRQ(GPAB_IRQn);
	NVIC_EnableIRQ(GPAB_IRQn);
}

// This API initiate ALC Module, and set parameter.
// This will set different Max-Gain and Target Level depended on volume control.
void ALC_Init(void)
{	
	// Enable ALC and set parameters by default value.
	SDADC_ENABLE_ALC(SDADC, SDADC_ALCCTL_NORMAL_MODE, SDADC_ALCCTL_ABS_PEAK, SDADC_ALCCTL_FASTDEC_ON);
	SDADC_ENABLE_NOISEGATE(SDADC, SDADC_ALCCTL_NGPEAK_P2P);
	SDADC_SET_NOISEGATE_TH(SDADC, SDADC_ALCCTL_NGTH1);
	
	// Set peak limit threshold at full scales and SDADC PGA gain at 6dB
	// Input signal will be cut off if the ALC gain is over peak limit threshold(in %) to prevent clipping.
	// Set SDADC PGA gain at mid level(+6dB) to match the microphone input.
	SDADC_SET_PEAKLIMIT_TH(SDADC, 0x7FFF);
	SDADC_ENABLE_PGA(SDADC, SDADC_PGACTL_GAIN_6DB_12K);

	// Set different Max-Gain and Target Level depended on volume control.
	if(g_u8FlagVol)
	{
		SDADC_SET_ALCRANGE(SDADC, SDADC_ALCCTL_RANGELOW);
		g_u32TartgetLevel = SDADC_SetALCTargetLevel(TARGET_LEVEL_VOLCTLON);
		g_u32MaxGain = SDADC_SetALCMaxGaindB(ALC_MAX_GAIN_VOLCTLON);
		g_u32MinGain = SDADC_SetALCMinGaindB(ALC_MIN_GAIN);
	}
	else
	{
		SDADC_SET_ALCRANGE(SDADC, SDADC_ALCCTL_RANGELOW);
		g_u32TartgetLevel = SDADC_SetALCTargetLevel(TARGET_LEVEL_VOLCTLOFF);
		g_u32MaxGain = SDADC_SetALCMaxGaindB(ALC_MAX_GAIN_VOLCTLOFF);
		g_u32MinGain = SDADC_SetALCMinGaindB(ALC_MIN_GAIN);
	}
	
	SDADC_EnableInt(SDADC_ALC_GMAX_INT);
	
	NVIC_ClearPendingIRQ(ALC_IRQn);
	NVIC_EnableIRQ(ALC_IRQn);
}
/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	int8_t item;
	System_Initiate();
	UART0_Init();
	Button_Init();
	
	printf("\r\n\r\nCPU @ %d Hz\r\n", SystemCoreClock);
  printf("+-------------------------------------------------+\r\n");
  printf("|  Demo Aanlog or Digital Mic input to SDADC	  |\r\n");
  printf("+------------------------------------------------+\r\n\r\n");
	/* The Following Means I91200BS just Support DMIC */
	#if !defined(__I91200BS__)
		printf("+Analog selcets option: 1 .\r\n");
	#endif
	printf("+Digital selcets option: 0 .\r\n");
	item = getchar();
	MIC_Init(item);
	Speaker_Init();
	Volume_Init();
	
	printf("Analog Mic and Volume Control are enabled.\r\n");
	printf("Press SWB5 to toggle ALC.\r\n");
	printf("Press SWB4 to toggle Volume Control.\r\n");
	
	SDADC_START_CONV(SDADC);
	DPWM_START_PLAY(DPWM);
	
	while(1);
}

void ALC_IRQHandler()
{
	// If ALC gain is excessed max gain, then increaseone level.
	if(SDADC_GetIntFlag(SDADC_ALC_GMAX_INT))
	{
		// Each gain level increse 6dB
		g_u32MaxGain = SDADC_SetALCMaxGaindB(g_u32MaxGain+600);
		
		SDADC_ClearIntFlag(SDADC_ALC_GMAX_INT);
	}
	
	NVIC_ClearPendingIRQ(ALC_IRQn);
}
// Button interrupt handler
// This would toggle variable: g_u8FlagALC, g_u8FlagVol to indicate whether the function is enabled or not.
void GPAB_IRQHandler()
{
	if(GPIO_GET_INT_FLAG(PB, BUTTON_SWB5))
	{
		if(g_u8FlagALC == 0)
		{
			ALC_Init();
			g_u8FlagALC = 1;
		}
		else if(g_u8FlagALC == 1)
		{
			SDADC_DISABLE_ALC();
			// Set high gain level when ALC is off.
			SDADC_ENABLE_PGA(SDADC, SDADC_PGACTL_GAIN_12DB);
			g_u8FlagALC = 0;
		}
		
		GPIO_CLR_INT_FLAG(PB, BUTTON_SWB5);
	}
	
	if(GPIO_GET_INT_FLAG(PB, BUTTON_SWB4))
	{
		if(g_u8FlagVol == 0)
		{
			Volume_Init();
			g_u8FlagVol = 1;
		}
		else if(g_u8FlagVol == 1)
		{
			VOLCTRL_DISABLE_FUNCTION(VOLCTRL, VOLCTRL_SDADC_SELECTION);
			g_u8FlagVol = 0;
		}
		
		GPIO_CLR_INT_FLAG(PB, BUTTON_SWB4);
	}
	
	NVIC_ClearPendingIRQ(GPAB_IRQn);
}

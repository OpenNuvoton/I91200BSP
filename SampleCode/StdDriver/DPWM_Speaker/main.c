/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/01/10 2:52p $
 * @brief    DPWM Driver Sample Code
 *           This is Speaker demo sample via DPWM.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "VDPWM/VDPWM.h"

#define DEMO_PLAY_LOOP (2)

// defined in 'IncludeBin_Keil.s'
extern uint32_t u32audioBegin, u32audioEnd;

// Pre-declare function.
void System_Initiate(void);

// DPWM Speaker API's function.
BOOL DPWM_Speaker_Setup(UINT32 u32SampleRate);
void DPWM_Speaker_Start(void);
void DPWM_Speaker_Stop(void);
void DPWM_Speaker_SetData(PINT16 pi16Buf,UINT32 u32BufCount);
BOOL DPWM_Speaker_IsPlaying(void);
void DPWM_Speaker_Volume(BOOL bUp);
void DPWM_Speaker_IRQProcess(void);

// Show message.
void Show_DemoLabel(void);
void Show_SpeakerSetup(BOOL bSuccess);
void Show_SpeakerStart(void);
void Show_SpeakerPlaying(void);
void Show_DemoEnd(void);

// Count current playback times.
volatile uint8_t u8PlayLoopCount = 0;
uint32_t *start, *end, *tmep;
int main(void)
{
	uint8_t u8Option = 0;
	
	// Initiate system clock
	System_Initiate();
	// Message : Demo label.
	Show_DemoLabel();
	
	// Setup DPWM to be a speaker
	// Message : DPWM speaker setup success/fail.
	if( DPWM_Speaker_Setup(8000) == TRUE ) 
		Show_SpeakerSetup(TRUE);
	else
	{
		Show_SpeakerSetup(FALSE);
		while(1);
	}	
	
        // for address check 
         start = &u32audioBegin;
         end = &u32audioEnd;
         if((&u32audioEnd-&u32audioBegin) < 0)
         {
             tmep =start;
             start = end;
             end = tmep;
          }
	while( u8Option != '4' )
	{
		// Message : Provide user select option for adjust volume.
		// 1. Playback test audio at current volume.
		// 2. Volume-down and play 2 times.
		// 3. Volume-up and play 2 times
		// 4. Exit speaker demo.
		Show_SpeakerStart();
		
		u8Option = getchar();
		u8PlayLoopCount = 0;
		
		// DPWM's speaker start to play.
		switch(u8Option)
		{
			case '2':
			case '3':
				DPWM_Speaker_Volume((u8Option-'2'));
			case '1' :				
				// Message : Show DPWM is playing and current volume value. 
				Show_SpeakerPlaying();
				DPWM_Speaker_Start();
				while(DPWM_Speaker_IsPlaying());
				break;		
		}
	}
	
	// Message : End demo.
	Show_DemoEnd();
}

void DPWM_IRQHandler(void)
{
	DPWM_Speaker_IRQProcess();		
}

// VDPWM provide callback API when data almost output completely and request input data.
// User could add some action here.
// Note. This callback function was in DPWM's IRQ.
void VDPWM_DataRequest(UINT8 u8VDPWMNo)
{
	if( ++u8PlayLoopCount < DEMO_PLAY_LOOP )
	{
		VDPWM_SetData(0,(PINT16)start,((uint32_t)end-(uint32_t)start)/sizeof(INT16));
	}
	else
	{
		if( VDPWM_IsBusy(0) == FALSE )
		{
			DPWM_Speaker_Stop();			
		}
	}
}

// DPWM Speaker =======================================================================
#include "volctrl.h"

#define DPWM_SPEAKER_PINS_MSK       (SYS_GPA_MFP_PA10MFP_Msk|SYS_GPA_MFP_PA11MFP_Msk)
#define DPWM_SPEAKER_PINS           (SYS_GPA_MFP_PA10MFP_SPKP|SYS_GPA_MFP_PA11MFP_SPKM)	

E_VOLCTRL_DB const aeDPWM_VolumeTable[] = {eVOLCTRL_NEG20_DB, eVOLCTRL_NEG6_DB, eVOLCTRL_0_DB, eVOLCTRL_6_DB, eVOLCTRL_15_DB};
int8_t const ai8DPWM_VolumeValue[] = {-20, -6, 0, 6, 15};
uint8_t u8Volume;

BOOL DPWM_Speaker_Setup(UINT32 u32SampleRate)
{	
    // Open DPWM and set playback sample rate. 
    if( VDPWM_Open(0, u32SampleRate, E_VDPWM_CLK_DEFAULT, TRUE) )
	{
		// The volume control shares a clock source with the BIQ filter, so user needs to
		// call CLK_EnableModuleClock and BIQ_LoadDefaultCoeff to enable volume control IP
		CLK_EnableModuleClock(BFAL_MODULE);
		SYS_ResetModule(BIQ_RST);
		BIQ_LoadDefaultCoeff();
		// Set middle value at aeDPWM_VolumeTable to default volume.
		VOLCTRL_Set_DPWMVol(aeDPWM_VolumeTable[(u8Volume = (sizeof(aeDPWM_VolumeTable)/sizeof(E_VOLCTRL_DB))/2)]);
		// Init I/O multi-function
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~DPWM_SPEAKER_PINS_MSK) ) | DPWM_SPEAKER_PINS;
		return TRUE;
	}
	return FALSE;
}

void DPWM_Speaker_Start(void)
{
	VDPWM_SetData(0,(PINT16)start,((uint32_t)end-(uint32_t)start)/sizeof(INT16));
	VDPWM_Start(0);
}

void DPWM_Speaker_Stop(void)
{
	VDPWM_Stop(0);
}

void DPWM_Speaker_IRQProcess(void)
{
	VDPWM_Process(0);	
}

void DPWM_Speaker_Volume(BOOL bUp)
{
	if( bUp && u8Volume<(sizeof(aeDPWM_VolumeTable)/sizeof(E_VOLCTRL_DB)) )
		VOLCTRL_Set_DPWMVol(aeDPWM_VolumeTable[(u8Volume+=1)]);
	else if( !bUp && u8Volume>0 )
		VOLCTRL_Set_DPWMVol(aeDPWM_VolumeTable[(u8Volume-=1)]);
}

BOOL DPWM_Speaker_IsPlaying(void)
{
	return VDPWM_IsBusy(0);
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
    printf("\n+------------------------------------------------------------------------+\r\n");
    printf("|                      DPWM Driver Sample Code                           |\r\n");
    printf("+------------------------------------------------------------------------+\r\n");		
}
void Show_SpeakerSetup(BOOL bSuccess)
{
	printf("(1) Setup VDPWM to be speaker.\r\n");
	printf("    1. Set playback's sample rate.\r\n");
	printf("    2. Enable hardware volume control for DPWM.\r\n");	
	printf("    3. Set default volume 0 Db.\r\n");		
	printf("    4. Config DPWM's gpio multi-function.\r\n");
	(bSuccess)?printf("    VDPWM setup success.\r\n"):printf("    VDPWM setup fail.\r\n");
}
void Show_SpeakerStart(void)
{
	printf("(2) Select option(1~4).\r\n");	
	printf("    1. Playback 2 times.\r\n");
	printf("    2. Volume down and playback 2 times.\r\n");
	printf("    3. Volume up and playback 2 times.\r\n");
	printf("    4. Exit demo.\r\n");
}
void Show_SpeakerPlaying(void)
{
	printf("(3) DPWM_Speaket is playing and current volume is %dDB.\r\n",ai8DPWM_VolumeValue[u8Volume]); 
}
void Show_DemoEnd(void)
{
	printf("(4) DPWM_Speaket demo end.\r\n"); 
	while(1);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/

//#include "CompilerOptionO2.h"
#include <string.h>
#include "VTIMER.h"
#include "VCommon.h"

#define VTIMER_COUNT (2)

// Declare hardware information about VTIMER
const S_VHW_INFO g_TimerHwInfo[VTIMER_COUNT] =                   
{                                                            
	{TMR0, TMR0_IRQn, TMR0_MODULE, TMR0_RST, CLK_CLKSEL1_TMR0SEL_Pos},
	{TMR1, TMR1_IRQn, TMR1_MODULE, TMR1_RST, CLK_CLKSEL1_TMR1SEL_Pos}
};

UINT32 VTIMER_Open(UINT8 u8VTIMERNo, UINT32 u32Frequency, enum eVTIMER_Clock eClockSource, BOOL bReset)
{	
	// Select clock source and enable module clock.
	CLK_EnableModuleClock(g_TimerHwInfo[u8VTIMERNo].u32ModuleID);	
	// Set timer's module clock source.
	CLK_SetModuleClock(g_TimerHwInfo[u8VTIMERNo].u32ModuleID, ((UINT32)eClockSource)<<g_TimerHwInfo[u8VTIMERNo].u8ClokcPos, 0);
	// Reset hardware ip.
	if(bReset)
	{
		SYS_ResetModule(g_TimerHwInfo[u8VTIMERNo].u32ResetID);
	}
	// Open timer interface.
	if( TIMER_Open((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr, TIMER_PERIODIC_MODE, u32Frequency) )
	{
		//VTIMER_Config(u8VTIMERNo,VTIMER_CFG_PERIODIC|VTIMER_CFG_INTEN);
		return TRUE;
	}
	return FALSE;
};

void VTIMER_Close(UINT8 u8VTIMERNo)
{
	TIMER_DisableInt((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr);
	TIMER_Close((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr);
	CLK_DisableModuleClock(g_TimerHwInfo[u8VTIMERNo].u32ModuleID);	
}

void VTIMER_Start(UINT8 u8VTIMERNo)
{
	TIMER_Start((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr);
}

void VTIMER_Stop(UINT8 u8VTIMERNo)
{
	TIMER_Stop((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr);
}

void VTIMER_Config(UINT8 u8VTIMERNo,UINT32 u32Configuration)
{
	UINT32 u32Tmp = 0;
	
	// Configurate operation mode(ONESHOT/CONTINUE/PERIODIC)
	if( (u32Tmp = u32Configuration&(VTIMER_CFG_ONESHOT|VTIMER_CFG_CONTINUE)) > 0 )
	{
		if( u32Tmp == VTIMER_CFG_ONESHOT )
			((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr)->CTL = (((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr)->CTL&~TMR_CTL_OPMODE_Msk)|TIMER_ONESHOT_MODE;
		else if( u32Tmp == VTIMER_CFG_CONTINUE )
			((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr)->CTL = (((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr)->CTL&~TMR_CTL_OPMODE_Msk)|TIMER_CONTINUOUS_MODE;
	}
	else
	{
		((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr)->CTL = (((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr)->CTL&~TMR_CTL_OPMODE_Msk)|TIMER_PERIODIC_MODE;
	}
	// Configurate interrupt(INTEN/INTDIS)
	if( u32Configuration&VTIMER_CFG_INTDIS )
	{
		TIMER_DisableInt(g_TimerHwInfo[u8VTIMERNo].pHwAddr);			
		NVIC_DisableIRQ(g_TimerHwInfo[u8VTIMERNo].eHwIRQn);		
		TIMER_ClearIntFlag(g_TimerHwInfo[u8VTIMERNo].pHwAddr);
		NVIC_ClearPendingIRQ(g_TimerHwInfo[u8VTIMERNo].eHwIRQn);
	}
	else
	{
		TIMER_ClearIntFlag(g_TimerHwInfo[u8VTIMERNo].pHwAddr);
		NVIC_ClearPendingIRQ(g_TimerHwInfo[u8VTIMERNo].eHwIRQn);
		NVIC_EnableIRQ(g_TimerHwInfo[u8VTIMERNo].eHwIRQn);	
		TIMER_EnableInt(g_TimerHwInfo[u8VTIMERNo].pHwAddr);			
	}
}

BOOL VTIMER_IsActive(UINT8 u8VTIMERNo)
{
	return TIMER_IS_ACTIVE((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr);
}

UINT32 VTIMER_Process(UINT8 u8VTIMERNo)
{
	UINT32 u32ProcessFlag = VTIMER_STA_NONE;
	
	if( TIMER_GetIntFlag((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr) )
	{
		TIMER_ClearIntFlag((TMR_T*)g_TimerHwInfo[u8VTIMERNo].pHwAddr);
		#if( VTIMER_PROCESSEVENT )
		u32ProcessFlag = VTIMER_STA_FREQ;
		#else
		VTIMER_FrequencyTrigger(u8VTIMERNo);
		#endif
	}
	return u32ProcessFlag;
}

__weak void VTIMER_FrequencyTrigger(UINT8 u8VTIMERNo)
{
};

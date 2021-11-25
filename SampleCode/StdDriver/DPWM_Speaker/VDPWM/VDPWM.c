//#include "CompilerOptionO2.h"
#include <string.h>
#include "VDPWM.h"
#include "VCommon.h"

#define VDPWM_COUNT         (1)

// Structure for VDPWM control handler.
typedef struct
{
	__IO UINT32  u32Count; 
    __IO PINT16  pi16Buf;
} S_VDPWM_DATABUF;

// Declare hardware information about VDPWM
const S_VHW_INFO g_DPWMHwInfo[VDPWM_COUNT] =                   
{                                                            
	{DPWM, DPWM_IRQn, DPWM_MODULE, DPWM_RST, CLK_CLKSEL1_DPWMSEL_Pos}
};

S_VDPWM_DATABUF g_asVDPWMDataBuf[VDPWM_COUNT];

UINT32 VDPWM_Open(UINT8 u8VDPWMNo, UINT32 u32Frequency, enum eVDPWM_Clock eClockSource, BOOL bReset)
{
	UINT32 u32RealFrequency = 0;
	// Enable DPWM module clock. 
	CLK_EnableModuleClock(g_DPWMHwInfo[u8VDPWMNo].u32ModuleID);
	// Reset DPWM Hardware IP 
	if(bReset)
		SYS_ResetModule(g_DPWMHwInfo[u8VDPWMNo].u32ResetID);
	// Set DPWM clock source.
	CLK_SetModuleClock(g_DPWMHwInfo[u8VDPWMNo].u32ModuleID, eClockSource<<g_DPWMHwInfo[u8VDPWMNo].u8ClokcPos, CLK_CLKDIV0_DPWM(1));
	// Open DPWM interface.
	DPWM_Open();	 
	// Set DPWM play sample rate.
	if( ( u32RealFrequency = DPWM_SetSampleRate(u32Frequency) ) > 0 )
	{
		VDPWM_Config(0,VDPWM_CFG_DATA16BITS|VDPWM_CFG_DEADTIMEDIS);
		VDPWM_ConfigFifoThresholdInt(0,4);
		memset(g_asVDPWMDataBuf,'\0',sizeof(S_VDPWM_DATABUF));		
	}
	return u32RealFrequency;
};

void VDPWM_Close(UINT8 u8VDPWMNo)
{
	DPWM_DISABLE_FIFOTHRESHOLDINT((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr));
	DPWM_Close();		
	CLK_DisableModuleClock(g_DPWMHwInfo[u8VDPWMNo].u32ModuleID);	
	g_asVDPWMDataBuf[u8VDPWMNo].u32Count = 0;
}

void VDPWM_Start(UINT8 u8VDPWMNo)
{
	DPWM_START_PLAY((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr));
	DPWM_ENABLE_DRIVER((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr));
}

void VDPWM_Stop(UINT8 u8VDPWMNo)
{
	DPWM_DISABLE_DRIVER((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr));
	DPWM_STOP_PLAY((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr));
	g_asVDPWMDataBuf[u8VDPWMNo].u32Count = 0;
}

void VDPWM_Config(UINT8 u8VDPWMNo,UINT32 u32Configuration)
{
	UINT32 u32Tmp = 0;
		
	// Config DeadTime(VDPWM_CFG_DEADTIMEEN/VDPWM_CFG_DEADTIMEDIS)
	if( u32Configuration&VDPWM_CFG_DEADTIMEEN )
	{
		DPWM_ENABLE_DEADTIME((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr));
	}
	else
	{
		DPWM_DISABLE_DEADTIME((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr));
	}
	// Config fifo data width.
	if( (u32Tmp = u32Configuration&(VDPWM_CFG_DATAMSB24BITS|VDPWM_CFG_DATA8BITS|VDPWM_CFG_DATA24BITS) ) > 0 )
	{
		switch(u32Tmp)
		{
			case VDPWM_CFG_DATAMSB24BITS:
				DPWM_SET_FIFODATAWIDTH((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr),DPWM_FIFO_DATAWIDTH_MSB24BITS);
				break;
			case DPWM_FIFO_DATAWIDTH_8BITS:
				DPWM_SET_FIFODATAWIDTH((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr),DPWM_FIFO_DATAWIDTH_8BITS);
				break;
			case VDPWM_CFG_DATA24BITS:
				DPWM_SET_FIFODATAWIDTH((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr),DPWM_FIFO_DATAWIDTH_24BITS);
				break;
		}
	}
	else
	{
		DPWM_SET_FIFODATAWIDTH((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr),DPWM_FIFO_DATAWIDTH_16BITS);
	}
}

void VDPWM_ConfigFifoThresholdInt(UINT8 u8VDPWMNo,UINT8 u8Threshold)
{
	if( u8Threshold )
	{
		DPWM_ENABLE_FIFOTHRESHOLDINT((I2C_T*)g_DPWMHwInfo[u8VDPWMNo].pHwAddr,u8Threshold);
		NVIC_ClearPendingIRQ(g_DPWMHwInfo[u8VDPWMNo].eHwIRQn);
		NVIC_EnableIRQ(g_DPWMHwInfo[u8VDPWMNo].eHwIRQn);
	}
	else
	{
		NVIC_DisableIRQ(g_DPWMHwInfo[u8VDPWMNo].eHwIRQn);
		DPWM_DISABLE_FIFOTHRESHOLDINT((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr));
		NVIC_ClearPendingIRQ(g_DPWMHwInfo[u8VDPWMNo].eHwIRQn);		
	}
}

void VDPWM_SetData(UINT8 u8VDPWMNo,PINT16 pi16Data,UINT32 u32DataCount)
{
	if( u32DataCount && pi16Data != NULL )
	{
		g_asVDPWMDataBuf[u8VDPWMNo].pi16Buf = pi16Data;
		g_asVDPWMDataBuf[u8VDPWMNo].u32Count = u32DataCount;
	}
}

BOOL VDPWM_IsBusy(UINT8 u8VDPWMNo)
{
	if( g_asVDPWMDataBuf[u8VDPWMNo].u32Count || DPWM_IS_FIFOEMPTY((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr))==FALSE)
	{
		return TRUE;
	}
	return FALSE;
}

void VDPWM_WaitComplete(UINT8 u8VDPWMNo)
{
	while(VDPWM_IsBusy(u8VDPWMNo) == TRUE);
}

UINT32 VDPWM_Process(UINT8 u8VDPWMNo)
{
	UINT32 u32StateFlag = VDPWM_STA_NONE;
	
	while( DPWM_IS_FIFOFULL((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr)) == FALSE )
	{
		if( g_asVDPWMDataBuf[u8VDPWMNo].u32Count )
		{
			((DPWM_T*)(g_DPWMHwInfo[u8VDPWMNo].pHwAddr))->DATA = *g_asVDPWMDataBuf[u8VDPWMNo].pi16Buf;
			g_asVDPWMDataBuf[u8VDPWMNo].pi16Buf++;
			g_asVDPWMDataBuf[u8VDPWMNo].u32Count--;
		}
		else
		{
			#if( VDPWM_PROCESSEVENT )
			u32StateFlag = VDPWM_STA_DATAREQUEST;
			#else
			VDPWM_DataRequest(u8VDPWMNo);	
			#endif
			break;
		}
	}
	return u32StateFlag;
}

__weak void VDPWM_DataRequest(UINT8 u8VDPWMNo)
{
};

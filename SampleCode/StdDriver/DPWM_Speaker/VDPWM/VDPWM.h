
#ifndef _VDPWM_H_
#define _VDPWM_H_

// ----------------------------------------------------------------------------------------------------------
// 	- Polling APIs call flow:
//    in main :
//	               VDPWM_Open()->VDPWM_Config()->
//              +->VDPWM_SetData()->
//              |  VDPWM_Start()->
//              |  VDPWM_Process()->
//              +- VDPWM_Stop()
//
// 	- Interrupt APIs call flow:
//    in main :
//		           VDPWM_Open()->VDPWM_Config()->
//                 VDPWM_ConfigFifoThresholdInt()->
//              +->VDPWM_SetData()
//              |  VDPWM_Start()->
//              |  VDPWM_IsBusy()/VDPWM_WaitComplete()->
//              +- VDPWM_Stop()
//    in IRQHandler :
//                 VDPWM_Process()
// -----------------------------------------------------------------------------------------------------------

#include "Platform.h"

// Clock Source Selection =============================
enum eVDPWM_Clock
{
	E_VDPWM_CLK_HCLK                 = CLK_CLKSEL1_DPWMSEL_HCLK>>CLK_CLKSEL1_DPWMSEL_Pos,
	E_VDPWM_CLK_HXT                  = CLK_CLKSEL1_DPWMSEL_HXT>>CLK_CLKSEL1_DPWMSEL_Pos,
	E_VDPWM_CLK_DEFAULT              = E_VDPWM_CLK_HCLK,
};


// Configuration Selection (Bitwise) ==================
#define VDPWM_CFG_DEADTIMEEN        (BIT0)         // Enable Dead Time
#define VDPWM_CFG_DEADTIMEDIS       (0)            // Disable Dead Time(Default)
#define VDPWM_CFG_DATA8BITS         (BIT4)         // FIFO Data Width 8Bits
#define VDPWM_CFG_DATA24BITS        (BIT5)         // FIFO Data Width 24Bits
#define VDPWM_CFG_DATAMSB24BITS     (BIT4|BIT5)    // FIFO Data Width MSB 24Bits
#define VDPWM_CFG_DATA16BITS        (0)            // FIFO Data Width 16Bits(Default)


// Virtual DPWM Common Function ========================

// u32Frequency : Bus clock(Unit:Hz)
// eClockSource : Clock source(eVSPI_Clock)
// bReset       : Reset hardware module(TRUE/FALSE)
UINT32 VDPWM_Open(UINT8 u8VDPWMNo, UINT32 u32Frequency, enum eVDPWM_Clock eClockSource, BOOL bReset);

void   VDPWM_Close(UINT8 u8VDPWMNo);

void   VDPWM_Start(UINT8 u8VDPWMNo);

void   VDPWM_Stop(UINT8 u8VDPWMNo);

UINT32 VDPWM_Process(UINT8 u8VDPWMNo);

// u32Configuration : Configuration Selection(Bitwise,ex: VDPWM_CFG_DRVEN|VDPWM_CFG_DATA24BITS etc.)
void   VDPWM_Config(UINT8 u8VDPWMNo,UINT32 u32Configuration);


// Virtual DPWM Special Function =======================

// pu32Data     : FIFO buffer data pointer for providing data to output.
// u32DataCount : Data count.
void   VDPWM_SetData(UINT8 u8VDPWMNo,PINT16 pi16Data,UINT32 u32DataCount);

// u8Threshold : Send/receive device address.
void   VDPWM_ConfigFifoThresholdInt(UINT8 u8VDPWMNo,UINT8 u8Threshold);

// Note. This API is working when enable VDPWM interrupt.
BOOL   VDPWM_IsBusy(UINT8 u8VDPWMNo);

// Note. This API is working when enable VDPWM interrupt.
void   VDPWM_WaitComplete(UINT8 u8VDPWMNo);


// ===================================================
// When event happened, 
// 0 : Process callback function
// 1 : rocess return "STATE FLAG"
#define VDPWM_PROCESSEVENT       (0)

// Virtual DPWM Callback Function ====================

// When input data count is zero.
void   VDPWM_DataRequest(UINT8 u8VDPWMNo);

// Virtual DPWM Process STATE FLAG ===================
#define VDPWM_STA_NONE               (0)            // No special feedback flag.
#define VDPWM_STA_DATAREQUEST        (1)            // Data request.

#endif

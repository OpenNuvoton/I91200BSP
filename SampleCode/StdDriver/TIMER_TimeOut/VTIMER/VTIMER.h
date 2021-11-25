
#ifndef _VTIMER_H_
#define _VTIMER_H_

// ----------------------------------------------------------------------------------------------------------
// 	- Polling call flow:
//		main loop: VTIMER_Open()->VTIMER_Config()
//          +----->VTIMER_Start()->
//          |      VTIMER_Process()->
//          +------VTIMER_Stop()
//
// 	- Interrupt call flow:
//		main loop: VTIMER_Open()->VTIMER_Config()
//          +----->VTIMER_Start()->
//          |      VTIMER_IsActive()/->
//          +------VTIMER_Stop()
//    in IRQHandler :
//                 VTIMER_Process()
// -----------------------------------------------------------------------------------------------------------

#include "Platform.h"

// Clock Source Selection =============================
enum eVTIMER_Clock
{
	E_VTIMER_CLK_LIRC                 = CLK_CLKSEL1_TMR0SEL_LIRC>>CLK_CLKSEL1_TMR0SEL_Pos,
	E_VTIMER_CLK_LXT                  = CLK_CLKSEL1_TMR0SEL_LXT>>CLK_CLKSEL1_TMR0SEL_Pos,
	E_VTIMER_CLK_HXT                  = CLK_CLKSEL1_TMR0SEL_HXT>>CLK_CLKSEL1_TMR0SEL_Pos,
	E_VTIMER_CLK_HCLK                 = CLK_CLKSEL1_TMR0SEL_HCLK>>CLK_CLKSEL1_TMR0SEL_Pos,
	E_VTIMER_CLK_DEFAULT              = E_VTIMER_CLK_HCLK,
};


// Configuration Selection (Bitwise) ==================
#define VTIMER_CFG_ONESHOT          (BIT0)         // One Shoot Mode
#define VTIMER_CFG_CONTINUE         (BIT1)         // Continue Mode
#define VTIMER_CFG_PERIODIC         (0)            // Periodic Mode(Default)
#define VTIMER_CFG_INTDIS           (BIT2)         // Disable interrupt
#define VTIMER_CFG_INTEN            (0)            // Enable interrupt(Default)


// Virtual TIMER Common Function ======================

// u32Frequency : Bus clock(Unit:Hz)
// eClockSource : Clock source(eVTIMER_Clock)
// bReset       : Reset hardware module(TRUE/FALSE)
UINT32 VTIMER_Open(UINT8 u8VTIMERNo, UINT32 u32Frequency, enum eVTIMER_Clock eClockSource, BOOL bReset);

void   VTIMER_Close(UINT8 u8VTIMERNo);

void   VTIMER_Start(UINT8 u8VTIMERNo);

void   VTIMER_Stop(UINT8 u8VTIMERNo);

UINT32 VTIMER_Process(UINT8 u8VTIMERNo);

// u32Configuration : Configuration Selection(Bitwise,ex: VTIMER_CFG_ONESHOT|VTIMER_CFG_INTEN etc.)
void   VTIMER_Config(UINT8 u8VTIMERNo,UINT32 u32Configuration);


// Virtual TIMER Special Function =======================

// return : Active(TRUE) or not(FALSE).
BOOL   VTIMER_IsActive(UINT8 u8VTIMERNo);


// ===================================================
// When event happened, 
// 0 : Process callback function
// 1 : rocess return "STATE FLAG"
#define VTIMER_PROCESSEVENT       (0)

// Virtual TIMER Callback Function =====================

// When config frequency trigger.
void   VTIMER_FrequencyTrigger(UINT8 u8VTIMERNo);

// Virtual TIMER Process STATE FLAG ====================
#define VTIMER_STA_NONE               (0)            // No special feedback flag.
#define VTIMER_STA_FREQ               (1)            // Frequency trigger.

#endif

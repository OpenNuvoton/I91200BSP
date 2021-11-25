#ifndef _VI2S_H_
#define _VI2S_H_

// ----------------------------------------------------------------------------------------------------------
// 	- Polling APIs call flow:
//	               VI2S_Open()->VI2S_Config()
//              +->VI2S_SetTxData()/VI2S_SetRxData()->
//              |  VI2S_Start()->
//              +- VI2S_Stop()
//
// 	- Interrupt APIs call flow:
//		           VI2S_Open()->VI2S_Config()->
//              +->VI2S_SetTxData()/VI2S_SetRxData()->
//              |  VI2S_Start()->
//              |  VI2S_IsBusy()/VI2S_WaitComplete()->
//              +- VI2S_Stop()
// -----------------------------------------------------------------------------------------------------------

#include "Platform.h"

// Clock Source Selection =============================
enum eVI2S_Clock
{
	E_VI2S_CLK_LIRC									= CLK_CLKSEL2_I2S0SEL_LIRC>>CLK_CLKSEL2_I2S0SEL_Pos,
	E_VI2S_CLK_LXT									= CLK_CLKSEL2_I2S0SEL_LXT>>CLK_CLKSEL2_I2S0SEL_Pos,
	E_VI2S_CLK_HXT									= CLK_CLKSEL2_I2S0SEL_HCLK>>CLK_CLKSEL2_I2S0SEL_Pos,
	E_VI2S_CLK_HIRC									= CLK_CLKSEL2_I2S0SEL_HIRC>>CLK_CLKSEL2_I2S0SEL_Pos,
	E_VI2S_CLK_DEFAULT              = 0,
};

// Configuration Selection (Bitwise) ==================
#define VI2S_CFG_MASTER             (BIT0)        // Master mode
#define VI2S_CFG_SLAVE              (BIT1)        // Slave mode
#define VI2S_CFG_STEREO	  					(BIT2)				// Stereo mode
#define VI2S_CFG_MONO   						(BIT3)				// Mono mode
#define VI2S_CFG_WORD8							(BIT4)				// Word sizes: 8 bits
#define VI2S_CFG_WORD16							(BIT5)				// Word sizes: 16 bits
#define VI2S_CFG_WORD24							(BIT6)				// Word sizes: 24 bits
#define VI2S_CFG_WORD32							(BIT7)				// Word sizes: 32 bits
#define VI2S_CFG_FORMATI2S					(BIT8)				// Data format: I2S
#define VI2S_CFG_FORMATMSB					(BIT9)				// Data format: MSB
#define VI2S_CFG_TXTH1							(BIT10)				// Transmit FIFO threshold level : 1 word
#define VI2S_CFG_TXTH4							(BIT11)				// Transmit FIFO threshold level : 4 word
#define VI2S_CFG_TXTH8							(BIT12)				// Transmit FIFO threshold level : 8 word
#define VI2S_CFG_RXTH1							(BIT13)				// Receive FIFO threshold level : 1 word
#define VI2S_CFG_RXTH4							(BIT14)				// Receive FIFO threshold level : 4 word
#define VI2S_CFG_RXTH8							(BIT15)				// Receive FIFO threshold level : 8 word
#define VI2S_CFG_TXINTDIS						(BIT16)				// Disable TX interrupt
#define VI2S_CFG_RXINTDIS						(BIT17)				// Disable RX interrupt
#define VI2S_CFG_LEFTZEROEN					(BIT18)				// Enable left channel zero interrupt
#define VI2S_CFG_LEFTZERODIS				(BIT19)				// Disable left channel zero interrupt
#define VI2S_CFG_RIGHTZEROEN				(BIT20)				// Enable right channel zero interrupt
#define VI2S_CFG_RIGHTZERODIS				(BIT21)				// Disable right channel zero interrupt

// Process State Flag (Bitwise) ==================
#define VI2S_STA_SENDCOMP						(BIT0)					// Transmit complete
#define VI2S_STA_RECCOMP						(BIT1)					// Receive complete
#define VI2S_STA_RECREQ							(BIT2)					// Data has been received but not set data pointer
#define VI2S_STA_LEFTZERO						(BIT3)					// Left channel zero occurred 
#define VI2S_STA_RIGHTZERO					(BIT4)					// Right cgannel zero occured

// ===================================================
// When event happened, 
// 0 : Process callback function
// 1 : Process return "STATE FLAG"
#define VI2S_PROCESSEVENT       (0)

// Virtual I2S Common Function ========================
// u32Frequency : Bus clock(Unit:Hz)
// eClockSource : Clock source(eVSPI_Clock)
// bReset       : Reset hardware module(TRUE/FALSE)
UINT32 VI2S_Open(UINT8 u8VI2SNo, UINT32 u32MasterSlave,UINT32 u32Frequency, enum eVI2S_Clock eClockSource, BOOL bReset);

void   VI2S_Close(UINT8 u8VI2SNo);

void   VI2S_Start(UINT8 u8VI2SNo);

void   VI2S_Stop(UINT8 u8VI2SNo);

// u32Configuration : Configuration Selection(Bitwise,ex: VI2S_CFG_TOEN|VI2S_CFG_TXINTEN etc.)
void   VI2S_Config(UINT8 u8VI2SNo,UINT32 u32Configuration);

// VI2C Interrupt request handler
UINT32 VI2S_Process(UINT8 u8VI2SNo);


// Virtual I2S Special Function =======================
//u32MasterClock				: Master clock.
//bEnable   						: Enable or disable master clock.
//												TRUE	- Enable master clock.
//												FALSE - Disable master clock.
void	 VI2S_SetMasterClock(UINT8 u8VI2SNo, UINT32 u32MasterClock, BOOL bEnable);

// pu32Data     : FIFO buffer data pointer for providing data to Tx.
// u32DataCount : Data count.
void   VI2S_SetTxData(UINT8 u8VI2SNo,PUINT32 pu32Data,UINT32 u32DataCount);

// pu32Data     : FIFO buffer data pointer for saving data from Rx.
// u32DataCount : Data count.
void   VI2S_SetRxData(UINT8 u8VI2SNo,PUINT32 pu32Data,UINT32 u32DataCount);
	
// Note. This API is working when enable VI2S Tx/Rx interrupt.
BOOL   VI2S_IsBusy(UINT8 u8VI2SNo);

// Note. This API is working when enable VI2S Tx/Rx interrupt.
void   VI2S_WaitComplete(UINT8 u8VI2SNo);


// Virtual I2S Callback Function ======================
// When send data complete.
void   VI2S_SendComplete(UINT8 u8VI2SNo);

// When receive data complete.
void   VI2S_ReceiveComplete(UINT8 u8VI2SNo);

// When get the receive data request.
void   VI2S_ReceiveRequest(UINT8 u8VI2SNo);

// When left channel zero occurred.
void	 VI2S_LeftZero(UINT8 u8VI2SNo);

// When right channel zero occured.
void 	 VI2S_RightZero(UINT8 u8VI2SNo);

#endif

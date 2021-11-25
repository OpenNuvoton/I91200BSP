
#ifndef _VI2C_H_
#define _VI2C_H_

// ----------------------------------------------------------------------------------------------------------
// 	- Polling APIs call flow:
//    in main.c :
//	               VI2C_Open()->VI2C_Config()
//                 VI2C_ConfigDeviceAddr()(in master mode)/
//                 VI2C_ConfigSlaveAddr()(in slave mode)
//            +--->VI2C_SetTxData()/VSPI_SetRxData()->
//            |    VI2C_Start()->
//            |    VI2C_Process()->
//            +--- VI2C_Stop()
//
// 	- Interrupt APIs call flow:
//    in main.c :
//		           VI2C_Open()->VI2C_Config()->
//                 VI2C_ConfigDeviceAddr()(in master mode)/
//                 VI2C_ConfigSlaveAddr()(in slave mode)
//              +->VI2C_SetTxData()/VI2C_SetRxData()->
//              |  VI2C_Start()->
//              |  VI2C_IsBusy()/VSPI_WaitComplete()->
//              +- VI2C_Stop()
//    in IRQHandler :
//                 VI2C_Process()
// -----------------------------------------------------------------------------------------------------------

#include "Platform.h"

// Clock Source Selection =============================
enum eVI2C_Clock
{
	E_VI2C_CLK_DEFAULT              = 0
};


// Configuration Selection (Bitwise) ==================
#define VI2C_CFG_MASTER             (0)            // Master mode(Default)
#define VI2C_CFG_SLAVE              (BIT0)         // Slave mode
#define VI2C_CFG_INTEN              (0)            // Enable interrupt(Default)
#define VI2C_CFG_INTDIS             (BIT1)         // Disable interrupt
#define VI2C_CFG_LTOEN              (BIT2)         // Enable time out and clock divide by 4
#define VI2C_CFG_TOEN               (BIT3)         // Enable time out.
#define VI2C_CFG_TODIS              (0)            // Disable time out(Default).


// Virtual I2C Common Function ========================

// u32Frequency : Bus clock(Unit:Hz)
// eClockSource : Clock source(eVSPI_Clock)
// bReset       : Reset hardware module(TRUE/FALSE)
// Note. Default configuration : VI2C_CFG_MASTER|VI2C_CFG_INTEN|VI2C_CFG_TODIS|VI2C_CFG_CALLBACK
UINT32 VI2C_Open(UINT8 u8VI2CNo, UINT32 u32Frequency, enum eVI2C_Clock eClockSource, BOOL bReset);

void   VI2C_Close(UINT8 u8VI2CNo);

void   VI2C_Start(UINT8 u8VI2CNo);

void   VI2C_Stop(UINT8 u8VI2CNo);

UINT32 VI2C_Process(UINT8 u8VI2CNo);

// u32Configuration : Configuration Selection(Bitwise,ex: VI2C_CFG_MASTER|VI2C_CFG_INTEN|VI2C_CFG_TODIS)
void   VI2C_Config(UINT8 u8VI2CNo,UINT32 u32Configuration);


// Virtual I2C Special Function =======================

// pu32Data     : FIFO buffer data pointer for providing data to Tx.
// u32DataCount : Data count.
void   VI2C_SetTxData(UINT8 u8VI2CNo,PUINT8 pu8Data,UINT32 u32DataCount);

// pu32Data     : FIFO buffer data pointer for saving data from Rx.
// u32DataCount : Data count.
void   VI2C_SetRxData(UINT8 u8VI2CNo,PUINT8 pu8Data,UINT32 u32DataCount);

// Note. This API is working for master mode.
// u8Address : Send/receive device address.
void   VI2C_ConfigDeviceAddress(UINT8 u8VI2CNo,UINT8 u8Address);

// Note. This API is working for slave mode.
// u8SlaveNo : Slave's number(0~3)
// u8Address : Send/receive device address.
// u8AddressMask : Mask of send/receive device address.
void   VI2C_ConfigSlaveAddress(UINT8 u8VI2CNo, UINT8 u8SlaveNo,UINT8 u8Address,UINT8 u8AddressMask);

// Note. This API is working when enable VI2C interrupt.
BOOL   VI2C_IsBusy(UINT8 u8VI2CNo);

// Note. This API is working when enable VI2C interrupt.
void   VI2C_WaitComplete(UINT8 u8VI2CNo);


// ===================================================
// When event happened, 
// 0 : Process callback function
// 1 : rocess return "STATE FLAG"
#define VI2C_PROCESSEVENT       (0)

// Virtual I2C Callback Function =====================

// When time out error happened.
void   VI2C_TimeOut(UINT8 u8VI2CNo);

// When send data complete.
void   VI2C_SendComplete(UINT8 u8VI2CNo);

// When receive data complete.
void   VI2C_ReceiveComplete(UINT8 u8VI2CNo);

// Note. This API is working for slave mode.
// When get the send data request.
void   VI2C_SendRequest(UINT8 u8VI2CNo);

// Note. This API is working for slave mode.
// When get the receive data request.
void   VI2C_ReceiveRequest(UINT8 u8VI2CNo);

// Virtual I2C Process STATE FLAG ====================
#define VI2C_STA_NONE               (0)            // No special feedback flag.
#define VI2C_STA_TIMEOUT            (1)            // Time out happen event.
#define VI2C_STA_RXREQUEST          (2)            // Receive request.
#define VI2C_STA_RXCOMPLETE         (3)            // Receive complete.
#define VI2C_STA_TXREQUEST          (4)            // Send request.
#define VI2C_STA_TXCOMPLETE         (5)            // Send complete.

#endif

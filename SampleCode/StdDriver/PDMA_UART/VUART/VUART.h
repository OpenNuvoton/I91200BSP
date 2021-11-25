
#ifndef _VUART_H_
#define _VUART_H_

// ----------------------------------------------------------------------------------------------------------
// 	- Polling APIs call flow:
//	               VUART_Open()->VUART_Config()
//              +->VUART_SetTxData()/VUART_SetRxData()->
//              |  VUART_Start()->
//							|	VUART_IsBusy()/VUART_WaitComplete()->
//							|	VUART_Process()->
//              +- VUART_Stop()
//
// 	- Interrupt APIs call flow:
//		           VUART_Open()->VUART_Config()->
//              +->VUART_SetTxData()/VUART_SetRxData()->
//              |  VUART_Start()->
//              +- VUART_Stop()
// -----------------------------------------------------------------------------------------------------------

#include "Platform.h"

// Clock Source Selection =============================
enum eVUART_Clock
{
	E_VUART_CLK_DEFAULT              = 0
};

// Configuration Selection (Bitwise) ==================
#define VUART_CFG_TXINTEN           		(BIT0)          // Enable transmit FIFO empty interrupt
#define VUART_CFG_TXINTDIS          		(BIT1)          // Disable transmit FIFO empty interrupt
#define VUART_CFG_RXLVL1INTEN       		(BIT2)          // Config%Enable receive FIFO trigger level 1 byte interrupt
#define VUART_CFG_RXLVL4INTEN       		(BIT3)          // Config%Enable receive FIFO trigger level 4 byte interrupt
#define VUART_CFG_RXLVL8INTEN       		(BIT4)          // Config%Enable receive FIFO trigger level 8 byte interrupt
#define VUART_CFG_RXINTDIS					(BIT5)			// Disable Receive data available interrupt
#define VUART_CFG_FIFORXRST         		(BIT6)          // Reset receive FIFO
#define VUART_CFG_FIFOTXRST         		(BIT7)          // Reset transmit FIFO
#define VUART_CFG_BUFERRINTEN       		(BIT8)       	// Enable buffer error interrupt.
#define VUART_CFG_BUFERRINTDIS      		(BIT9)			// Disable buffer error interrupt.
#define VUART_CFG_RXTOINTEN         		(BIT10)         // Enable receive time out interrupt.
#define VUART_CFG_RXTOINTDIS        		(BIT11)         // Disable receive time out interrupt.
#define VUART_CFG_WORDL8BITS				(BIT12)			// Word length select as 8bits
#define VUART_CFG_WORDL7BITS				(BIT13)			// Word length select as 7bits
#define VUART_CFG_WORDL6BITS				(BIT14)			// Word length select as 6bits
#define VUART_CFG_WORDL5BITS				(BIT15)			// Word length select as 5bits
#define VUART_CFG_1STOPBITS					(BIT16)			// Generate 1 stop bits after transmitted data
#define VUART_CFG_2STOPBITS					(BIT17)			// 2 stop bits are generated when 6,7 and 8-bit word length is selected. 1.5 stop bits are generated when 5-bit word length is selected
#define VUART_CFG_ODDPARITY					(BIT18)			// Odd number of logic 1 are transmitted or checked in the data word and parity bits.
#define VUART_CFG_EVENPARITY				(BIT19)			// Even number of logic 1 are transmitted or checked in the data word and parity bits.
#define VUART_CFG_PARITYDIS					(BIT20)			// Parity bit is not generated or checked during transfer.

// Process State Flag (Bitwise) ==================
#define VUART_STA_BUFERR					(BIT0)			// Buffer error occurred
#define VUART_STA_RXTO						(BIT1)			// Receive FIFO time-out occurred
#define VUART_STA_SENDCOMP					(BIT2)			// Transmit complete
#define VUART_STA_RECCOMP					(BIT3)			// Receive complete
#define VUART_STA_RECREQ					(BIT4)			// Data has been received but not set data pointer

// =====================================================
// This define when event happens how VUART process respones.
// 0 : Process will enter callback function.
// 1 : Process will return "STATE FLAG".
#define VUART_PROCESSEVENT       			(0)

// Virtual UART Common Function ========================
// u8VUARTNo	: Module port number
// u32Frequency : Bus clock(Unit:Hz)
// eClockSource : Clock source(eVSPI_Clock)
// bReset       : Reset hardware module(TRUE/FALSE)
// Return		: If open sucessfully then return TRUE, and vice versa.
// Detail		: Call this API to open VUART function before using UART.
UINT32 VUART_Open(UINT8 u8VUARTNo, UINT32 u32Frequency, enum eVUART_Clock eClockSource, BOOL bReset);
// u8VUARTNo	: Module port number
// Detail		: Call this API to close VUART function.
void   VUART_Close(UINT8 u8VUARTNo);
// u8VUARTNo	: Module port number
// Detail		: Call this API to start VUART function after seting TX data or RX data.
//				  If TX data has been set, then hardware will start to transmit.
//				  If RX data has been set, then hardware will be ready to receive data.
void   VUART_Start(UINT8 u8VUARTNo);
// u8VUARTNo	: Module port number
// Detail		: Call this API to stop VUART if VUART is in action.
void   VUART_Stop(UINT8 u8VUARTNo);
// u8VUARTNo	: Module port number
// u32Configuration : Configuration selection in bitwise.
//					  User can choose multiple configuration and using bitwise or "|" to combine each configuration
//					  (ex: VUART_CFG_FIFORXRST|VUART_CFG_TXINTEN etc.)
void   VUART_Config(UINT8 u8VUARTNo,UINT32 u32Configuration);
// u8VUARTNo	: Module port number
// Detail		: VUART main process. Call this API to process TX/RX data.
UINT32 VUART_Process(UINT8 u8VUARTNo);


// Virtual UART Special Function =======================
//u32Baud				: Baud rate.
void	 VUART_SetTimeOut(UINT8 u8VUARTNo, UINT32 u32Baud);

// pu32Data     : FIFO buffer data pointer for providing data to Tx.
// u32DataCount : Data count.
void   VUART_SetTxData(UINT8 u8VUARTNo,PUINT8 pu8Data,UINT32 u32DataCount);

// pu32Data     : FIFO buffer data pointer for saving data from Rx.
// u32DataCount : Data count.
void   VUART_SetRxData(UINT8 u8VUARTNo,PUINT8 pu8Data,UINT32 u32DataCount);
	
// Note. This API is working when enable VUART Tx/Rx interrupt.
BOOL   VUART_IsBusy(UINT8 u8VUARTNo);

// Note. This API is working when enable VUART Tx/Rx interrupt.
void   VUART_WaitComplete(UINT8 u8VUARTNo);


// Virtual UART Callback Function ======================
// When buffer error happened.
void   VUART_BufferError(UINT8 u8VUARTNo);

// When receive time out error happened.
void   VUART_ReceiveTimeOut(UINT8 u8VUARTNo);

// When send data complete.
void   VUART_SendComplete(UINT8 u8VUARTNo);

// When receive data complete.
void   VUART_ReceiveComplete(UINT8 u8VUARTNo);

// When get the receive data request.
void   VUART_ReceiveRequest(UINT8 u8VUARTNo);

#endif

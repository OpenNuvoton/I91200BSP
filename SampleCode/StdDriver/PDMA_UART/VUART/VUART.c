//#include "CompilerOptionO2.h"
#include <string.h>
#include "VUART.h"
#include "VCommon.h"

#define VUART_COUNT        (2)

// Structure for VUART interrupt control. //
typedef struct
{
		__IO BOOL		 bRXINT:1;			// RX interrupt enable flag.
		__IO BOOL		 bTXINT:1;			// TX interrupt enable flag.
		__IO BOOL		 bBUFERRINT:1;	// Buffer error interrupt flag.
		__IO BOOL		 bRXTOINT:1;		// RX time-out interrupt flag.
} S_VUART_CTRL;

// Structure for VUART buffer control. //
typedef struct
{
	__IO UINT16  u16TxCount;			// Number of data in TX buffer to transmit. 
	__IO UINT16  u16RxCount;			// Number of data to receive to RX buffer.
	__IO PUINT8	 pu8TxBuf;  			// TX buffer pointer.
	__IO PUINT8  pu8RxBuf;				// RX buffer pointer.
} S_VUART_DATABUF;

// Declare hardware information of VUART. //
const S_VHW_INFO g_UARTHwInfo[VUART_COUNT] =          
{                                                            
	{UART0, UART0_IRQn, UART0_MODULE, UART0_RST, NULL},
	{UART1, UART1_IRQn, UART1_MODULE, UART1_RST, NULL}
};

S_VUART_CTRL g_asVUARTControl[VUART_COUNT];
S_VUART_DATABUF g_asVUARTDataBuf[VUART_COUNT];

UINT32 VUART_Open(UINT8 u8VUARTNo, UINT32 u32Frequency, enum eVUART_Clock eClockSource, BOOL bReset)
{
	UART_T *pUART = g_UARTHwInfo[u8VUARTNo].pHwAddr;
	
	// Enable module clock.
	CLK_EnableModuleClock(g_UARTHwInfo[u8VUARTNo].u32ModuleID);	
	// Reset module.
	if(bReset)
		SYS_ResetModule(g_UARTHwInfo[u8VUARTNo].u32ResetID);	
	// Open UART module
	UART_Open(pUART, u32Frequency);
	// Initiate VUART control structure.
	memset(g_asVUARTControl, '\0', sizeof(g_asVUARTControl));
	memset(g_asVUARTDataBuf, '\0', sizeof(g_asVUARTDataBuf));
	// Default Config.
	VUART_Config(u8VUARTNo, VUART_CFG_TXINTEN|VUART_CFG_RXLVL4INTEN);
	// Disable module clock.
	CLK_DisableModuleClock(g_UARTHwInfo[u8VUARTNo].u32ModuleID);
	
	return TRUE;
}

void VUART_Close(UINT8 u8VUARTNo)
{
	// Close UART module.
	UART_Close((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr);	
	// Disable UART module clock.
	CLK_DisableModuleClock(g_UARTHwInfo[u8VUARTNo].u32ModuleID);	
	// Reset VUART contol structure.
	memset(g_asVUARTControl, '\0', sizeof(g_asVUARTControl));
	memset(g_asVUARTDataBuf, '\0', sizeof(g_asVUARTDataBuf));
}

void VUART_Start(UINT8 u8VUARTNo)
{
	UART_T *pUART = (UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr;
	
	// Enable module clock.
	CLK_EnableModuleClock(g_UARTHwInfo[u8VUARTNo].u32ModuleID);
	// Check interrupt flag to enable interrupt 
	{
	if(g_asVUARTControl[u8VUARTNo].bBUFERRINT == TRUE)
		UART_ENABLE_INT(pUART, UART_INTEN_BUFERRIEN_Msk);	
	if(g_asVUARTControl[u8VUARTNo].bRXTOINT == TRUE)
		UART_ENABLE_INT(pUART, UART_INTEN_RXTOIEN_Msk);	
	if(g_asVUARTControl[u8VUARTNo].bRXINT == TRUE)
		UART_ENABLE_INT(pUART, UART_INTEN_RDAIEN_Msk);
	if((g_asVUARTControl[u8VUARTNo].bTXINT == TRUE) && (g_asVUARTDataBuf[u8VUARTNo].pu8TxBuf != 0))
		UART_ENABLE_INT(pUART, UART_INTEN_THREIEN_Msk);
	}
}

void VUART_Stop(UINT8 u8VUARTNo)
{
	UART_T *pUART = (UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr;
	
	// Disable interrupt.
	UART_DISABLE_INT(pUART,UART_INTEN_RDAIEN_Msk|UART_INTEN_THREIEN_Msk|UART_INTEN_RXTOIEN_Msk|UART_INTEN_BUFERRIEN_Msk);
	// Reset buffer data count.
	g_asVUARTDataBuf[u8VUARTNo].u16RxCount = 0;
	g_asVUARTDataBuf[u8VUARTNo].u16TxCount = 0;
}

void VUART_Config(UINT8 u8VUARTNo,UINT32 u32Configuration)
{
	UINT32 u32Tmp;
	UART_T *pUART = (UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr;
	
	CLK_EnableModuleClock(g_UARTHwInfo[u8VUARTNo].u32ModuleID);	
	
	// Reset receive fifo buffer.
	if( u32Configuration&VUART_CFG_FIFORXRST )
	{
		pUART->FIFO |= UART_FIFO_RXRST_Msk;
	}	
	// Reset transmit fifo buffer.
	if( u32Configuration&VUART_CFG_FIFOTXRST )
	{
		pUART->FIFO |= UART_FIFO_TXRST_Msk;
	}
	// Configure TX FIFO empty interrupt enable/disable
	if( (u32Tmp = u32Configuration&(VUART_CFG_TXINTEN|VUART_CFG_TXINTDIS)) >0 )
	{
		// Set VUART interrupt flag.
		if(u32Tmp == VUART_CFG_TXINTEN)
			g_asVUARTControl[u8VUARTNo].bTXINT = TRUE;
		else if(u32Tmp == VUART_CFG_TXINTDIS)
		{
			g_asVUARTControl[u8VUARTNo].bTXINT = FALSE;
			UART_DISABLE_INT(pUART,UART_INTEN_BUFERRIEN_Msk);
		}
	}
	// Configure receive FIFO trigger level interrupt enable/disable.
	if( (u32Tmp = u32Configuration&(VUART_CFG_RXLVL1INTEN|VUART_CFG_RXLVL4INTEN|VUART_CFG_RXLVL8INTEN|VUART_CFG_RXINTDIS)) >0 ) 
	{
		if( u32Tmp == VUART_CFG_RXINTDIS )
		{
			// Set VUART interrupt flag.
			g_asVUARTControl[u8VUARTNo].bRXINT = FALSE;
			// Reset trigger level.
			pUART->FIFO = (pUART->FIFO & (~UART_FIFO_RFITL_Msk));
			// Disable interrupt.
			UART_DISABLE_INT(pUART,UART_INTEN_RDAIEN_Msk);
		}
		else
		{
			// Set VUART interrupt flag.
			g_asVUARTControl[u8VUARTNo].bRXINT = TRUE;
			// Set trigger level.
			pUART->FIFO = (pUART->FIFO&(~UART_FIFO_RFITL_Msk))|((u32Tmp == VUART_CFG_RXLVL1INTEN)?UART_FIFO_RFITL_1BYTE:((u32Tmp == VUART_CFG_RXLVL4INTEN)?UART_FIFO_RFITL_4BYTES:UART_FIFO_RFITL_8BYTES));
		}
	}
	// Config receive time out counter and interrupt enable/disable.
	if( (u32Tmp = u32Configuration&(VUART_CFG_RXTOINTEN|VUART_CFG_RXTOINTDIS)) >0 ) 
	{
		if(u32Tmp == VUART_CFG_RXTOINTEN)
		{
			// Set VUART interrupt flag.
			g_asVUARTControl[u8VUARTNo].bRXTOINT = TRUE;
			// Config receive time out counter.
			pUART->TOUT = UART_TOUT_TOIC_Msk;
		}
		else if(u32Tmp == VUART_CFG_RXTOINTDIS)
		{
			// Set VUART interrupt flag.
			g_asVUARTControl[u8VUARTNo].bRXTOINT = FALSE;
			// Disable interrupt.
			UART_DISABLE_INT(pUART,UART_INTEN_TOCNTEN_Msk);
		}
	}
	// Config buffer error interrupt enable/disable.
	if( (u32Tmp = u32Configuration&(VUART_CFG_BUFERRINTEN|VUART_CFG_BUFERRINTDIS)) >0 )
	{
		if(u32Tmp == VUART_CFG_BUFERRINTEN)
			// Set VUART interrupt flag.
			g_asVUARTControl[u8VUARTNo].bBUFERRINT = TRUE;
		else if (u32Tmp == VUART_CFG_BUFERRINTDIS)
		{
			// Set VUART interrupt flag.
			g_asVUARTControl[u8VUARTNo].bBUFERRINT = FALSE;
			// Disable interrupt.
			UART_DISABLE_INT(pUART,UART_INTEN_BUFERRIEN_Msk);
		}
	}
	// Config word length.
	if( (u32Tmp = u32Configuration&(VUART_CFG_WORDL8BITS|VUART_CFG_WORDL7BITS|VUART_CFG_WORDL6BITS|VUART_CFG_WORDL5BITS)) >0 )
	{
		if(u32Tmp == VUART_CFG_WORDL8BITS)
			pUART->LINE = ((pUART->LINE & ~UART_LINE_WLS_Msk) | UART_WORD_LEN_8);
		else if(u32Tmp == VUART_CFG_WORDL7BITS)
			pUART->LINE = ((pUART->LINE & ~UART_LINE_WLS_Msk) | UART_WORD_LEN_7);
		else if(u32Tmp == VUART_CFG_WORDL6BITS)
			pUART->LINE = ((pUART->LINE & ~UART_LINE_WLS_Msk) | UART_WORD_LEN_6);
		else if(u32Tmp == VUART_CFG_WORDL5BITS)
			pUART->LINE = ((pUART->LINE & ~UART_LINE_WLS_Msk) | UART_WORD_LEN_5);
	}
	// Config Stop Bits
	if( (u32Tmp = u32Configuration & (VUART_CFG_1STOPBITS|VUART_CFG_2STOPBITS)) >0 )
	{
		if(u32Tmp == VUART_CFG_1STOPBITS)
			pUART->LINE = (pUART->LINE & ~UART_LINE_NSB_Msk) | UART_STOP_BIT_1;
		else if(u32Tmp == VUART_CFG_2STOPBITS)
			pUART->LINE = (pUART->LINE & ~UART_LINE_NSB_Msk) | UART_STOP_BIT_2;
	}
	// Config Parity Bits
	if( (u32Tmp = u32Configuration & (VUART_CFG_ODDPARITY|VUART_CFG_EVENPARITY|VUART_CFG_PARITYDIS)) >0 )
	{
		if(u32Tmp == VUART_CFG_PARITYDIS)
			pUART->LINE = (pUART->LINE & ~(UART_LINE_PBE_Msk|UART_LINE_EPE_Msk|UART_LINE_SPE_Msk)) | UART_PARITY_NONE;
		else if(u32Tmp == VUART_CFG_ODDPARITY)
			pUART->LINE = (pUART->LINE & ~(UART_LINE_PBE_Msk|UART_LINE_EPE_Msk|UART_LINE_SPE_Msk)) | UART_PARITY_ODD;
		else if(u32Tmp == VUART_CFG_EVENPARITY)
			pUART->LINE = (pUART->LINE & ~(UART_LINE_PBE_Msk|UART_LINE_EPE_Msk|UART_LINE_SPE_Msk)) | UART_PARITY_EVEN;
	}
	// Config NVIC if any interrupt configuration is set.
	if( u32Configuration&(VUART_CFG_TXINTEN|VUART_CFG_RXLVL1INTEN|VUART_CFG_RXLVL4INTEN|VUART_CFG_RXLVL8INTEN|VUART_CFG_BUFERRINTEN|VUART_CFG_RXTOINTEN) )
	{
		NVIC_ClearPendingIRQ(g_UARTHwInfo[u8VUARTNo].eHwIRQn);
		NVIC_EnableIRQ(g_UARTHwInfo[u8VUARTNo].eHwIRQn);
	}
	
	// Disable Module Clock.
	CLK_DisableModuleClock(UART0_MODULE);
}

void VUART_SetTimeOut(UINT8 u8VUARTNo, UINT32 u32TOIC)
{
	UART_T *pUART = (UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr;
	
	// Enable module clock.
	CLK_EnableModuleClock(g_UARTHwInfo[u8VUARTNo].u32ModuleID);	
	
	if( u32TOIC < UART_TOUT_TOIC_Msk )
	{
		// Set time-out trigger at u32TOIC
		pUART->TOUT = u32TOIC;
	}
	else if( u32TOIC >= UART_TOUT_TOIC_Msk)// If u32TOIC excess TOUT register range.
	{
		// Set time-out trigger at max value.
		pUART->TOUT = UART_TOUT_TOIC_Msk;
	}
	
	// Disable module clock.
	CLK_DisableModuleClock(UART0_MODULE);
}

void VUART_SetTxData(UINT8 u8VUARTNo,PUINT8 pu8Data,UINT32 u32DataCount)
{
	if( u32DataCount && pu8Data != NULL )
	{
		g_asVUARTDataBuf[u8VUARTNo].u16TxCount = u32DataCount;
		g_asVUARTDataBuf[u8VUARTNo].pu8TxBuf = pu8Data;

		if(g_asVUARTControl[u8VUARTNo].bTXINT == TRUE)
			UART_ENABLE_INT(((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr), UART_INTEN_THREIEN_Msk);
	}
}

void VUART_SetRxData(UINT8 u8VUARTNo,PUINT8 pu8Data,UINT32 u32DataCount)
{
	if( u32DataCount && pu8Data != NULL )
	{
		g_asVUARTDataBuf[u8VUARTNo].u16RxCount = u32DataCount;
		g_asVUARTDataBuf[u8VUARTNo].pu8RxBuf = pu8Data;
	}
}

BOOL VUART_IsBusy(UINT8 u8VUARTNo)
{
	UART_T *pUART = g_UARTHwInfo[u8VUARTNo].pHwAddr;
	// Check if UART is busy.
	if( g_asVUARTDataBuf[u8VUARTNo].u16RxCount || g_asVUARTDataBuf[u8VUARTNo].u16TxCount || !(pUART->FIFOSTS&UART_FIFOSTS_TXEMPTY_Msk) || !(pUART->FIFOSTS&UART_FIFOSTS_RXEMPTY_Msk))
		return TRUE;
	
	return FALSE;
}

void VUART_WaitComplete(UINT8 u8VUARTNo)
{
	while(VUART_IsBusy(u8VUARTNo) == TRUE);
}

UINT32 VUART_TxProcess(UINT8 u8VUARTNo)
{	
	if( g_asVUARTDataBuf[u8VUARTNo].u16TxCount == 0 )
	{
		// If TX data count = 0 then disable TX interrupt.
		UART_DISABLE_INT(((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr),UART_INTEN_THREIEN_Msk);
		return 0;
	}
	while( UART_IS_TX_FULL(((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr))!=TRUE )
	{
		// Write data from buffer to UART data register.
		UART_WRITE(((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr),*(g_asVUARTDataBuf[u8VUARTNo].pu8TxBuf));
		// Point to next data.
		g_asVUARTDataBuf[u8VUARTNo].pu8TxBuf++;
		// Check if TX buffer is empty.
		if( (g_asVUARTDataBuf[u8VUARTNo].u16TxCount-=1)==0)
		{
			// Disable TX interrupt.
			UART_DISABLE_INT(((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr),UART_INTEN_THREIEN_Msk);
			// Return SendComplete State Flag or enter Call Back function.
			#if (VUART_PROCESSEVENT)
			return VUART_STA_SENDCOMP;
			#else
			VUART_SendComplete(u8VUARTNo);
			return 0;
			#endif
		}
	}
	
	return 0;
}

UINT32 VUART_RxProcess(UINT8 u8VUARTNo)
{
	uint8_t u8Temp;
	
	if( g_asVUARTDataBuf[u8VUARTNo].u16RxCount == 0 )
	{
		if(UART_GET_RX_EMPTY(((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr)) == FALSE)
		{
			// If RX buffer data count = 0 and RX has data to receive
			// Return ReceiveRequest State Flag or enter Call Back function.
			#if (VUART_PROCESSEVENT)
			return VUART_STA_RECREQ;
			#else
			VUART_ReceiveRequest(u8VUARTNo);
			#endif
		}
		else
			return 0;
	}
	while(UART_GET_RX_EMPTY(((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr))==FALSE)
	{
		// Read data from UART data register to buffer.
		u8Temp = UART_READ(((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr));
		*(g_asVUARTDataBuf[u8VUARTNo].pu8RxBuf) = u8Temp;
		// Point to next data.
		g_asVUARTDataBuf[u8VUARTNo].pu8RxBuf++;
		if((g_asVUARTDataBuf[u8VUARTNo].u16RxCount-=1)==0)
		{
			// Return ReceiveComplete State Flag or enter Call Back function.
			#if (VUART_PROCESSEVENT)
			return VUART_STA_RECCOMP;
			#else
			VUART_ReceiveComplete(u8VUARTNo);
			return 0;
			#endif
		}
	}
	
	return 0;
}

UINT32 VUART_Process(UINT8 u8VUARTNo)
{
	UART_T *pUART = ((UART_T*)g_UARTHwInfo[u8VUARTNo].pHwAddr);
	S_VUART_CTRL S_Handler = g_asVUARTControl[u8VUARTNo];
	UINT32 u32State = 0;
	
	// Buffer error occurred. //
	if(pUART->INTSTS & UART_INTSTS_BUFERRINT_Msk)
	{
		// Set BufferError State Flag or enter Call Back function. 
		#if (VUART_PROCESSEVENT)
		u32State |= VUART_STA_BUFERR;
		#else
		VUART_BufferError(u8VUARTNo);
		#endif
	}
	// Receive FIFO time-out occurred. //
	if(pUART->INTSTS & UART_INTSTS_RXTOINT_Msk)
	{
		// Set RxTimeOut State Flag or enter Call Back function.
		#if (VUART_PROCESSEVENT)
		u32State |= VUART_STA_RXTO;
		#else
		VUART_ReceiveTimeOut(u8VUARTNo);
		#endif
	}
	// Receive FIFO threshold interrupt occurred. //
	if(pUART->INTSTS & UART_INTSTS_RDAINT_Msk)
	{
		// Enter RX Process.
		u32State |= VUART_RxProcess(u8VUARTNo);
	}
	// Transmit FIFO empty interrupt occurred. //
	if(pUART->INTSTS & UART_INTSTS_THERINT_Msk)
	{
		// Enter TX Process.
		u32State |= VUART_TxProcess(u8VUARTNo);
	}
	
	// Clear Pending Interrupt Request
	NVIC_ClearPendingIRQ(g_UARTHwInfo[u8VUARTNo].eHwIRQn);
	
	// Use polling to enter TX/RX Process.
	if(!S_Handler.bTXINT)
		u32State |= VUART_TxProcess(u8VUARTNo);
	if(!S_Handler.bRXINT && !S_Handler.bRXTOINT)
		u32State |= VUART_RxProcess(u8VUARTNo);
	
	// If Using Call Back Function return value u32State is 0.
	return u32State;
}

#if (!VUART_PROCESSEVENT)
__weak void VUART_BufferError(UINT8 u8VUARTNo)
{
};
__weak void VUART_ReceiveTimeOut(UINT8 u8VUARTNo)
{
};
__weak void VUART_SendComplete(UINT8 u8VUARTNo)
{
};
__weak void VUART_ReceiveComplete(UINT8 u8VUARTNo)
{	
};
__weak void VUART_ReceiveRequest(UINT8 u8VUARTNo)
{	
};
#endif

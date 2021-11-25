//#include "CompilerOptionO2.h"
#include <string.h>
#include "VI2S.h"
#include "VCommon.h"

#define VI2S_COUNT					(1)
#define I2S_FIFO_SIZE				(8)

// Structure for VUART interrupt control. //
typedef struct
{
		__IO BOOL		 	bRXINT:1;			// RX interrupt enable flag.
		__IO BOOL		 	bTXINT:1;			// TX interrupt enable flag.
		__IO BOOL			bLZINT:1;			// Left zero interrupt flag.
		__IO BOOL			bRZINT:1;			// Right zero interrupt flag.
} S_VI2S_CTRL;

// Structure for VI2S buffer control. //
typedef struct
{
    __IO UINT16		u16TxCount;
    __IO UINT16		u16RxCount;
    __IO PUINT32	pu32TxBuf;
    __IO PUINT32	pu32RxBuf;
} S_VI2S_DATABUF;

// Declare hardware information about VI2S
const S_VHW_INFO g_I2SHwInfo[VI2S_COUNT] =                   
{                                                            
	{I2S, I2S0_IRQn, I2S0_MODULE, I2S0_RST, CLK_CLKSEL2_I2S0SEL_Pos},
};

S_VI2S_DATABUF g_asVI2SDataBuf[VI2S_COUNT];
S_VI2S_CTRL g_asVI2SControl[VI2S_COUNT];

UINT32 VI2S_Open(UINT8 u8VI2SNo, UINT32 u32MasterSlave, UINT32 u32Frequency, enum eVI2S_Clock eClockSource, BOOL bReset)
{
	I2S_T *pI2S = (I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr;
	
	//Enable module clock
	CLK_EnableModuleClock(g_I2SHwInfo[u8VI2SNo].u32ModuleID);
	// Set I2S module clock source.
	CLK_SetModuleClock(g_I2SHwInfo[u8VI2SNo].u32ModuleID, ((UINT32)eClockSource)<<g_I2SHwInfo[u8VI2SNo].u8ClokcPos, 0);
	// Reset module.
	if(bReset)
		SYS_ResetModule(g_I2SHwInfo[u8VI2SNo].u32ResetID);	
	// I2S open as master or slave mode
	if(u32MasterSlave == VI2S_CFG_MASTER)
	{
		// Open I2S module
		I2S_Open(pI2S, I2S_MODE_MASTER, u32Frequency, I2S_DATABIT_16, I2S_STEREO, I2S_FORMAT_I2S, NULL);
		// Enable master clok; set frequency at 256 * sampling rate
		I2S_EnableMCLK((I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr, 256*u32Frequency);
	}
	else
		I2S_Open(pI2S, I2S_MODE_SLAVE, u32Frequency, I2S_DATABIT_16, I2S_STEREO, I2S_FORMAT_I2S, NULL);
	// Initiate I2S data buffer structure.
	memset(g_asVI2SDataBuf,'\0',sizeof(g_asVI2SDataBuf));
	// Default Configuration
	VI2S_Config(u8VI2SNo, VI2S_CFG_STEREO|VI2S_CFG_WORD16|VI2S_CFG_FORMATI2S|VI2S_CFG_TXINTDIS|VI2S_CFG_RXINTDIS);
	// Disable I2S operation.
	pI2S->CTL &= ~I2S_CTL_I2SEN_Msk;

	return TRUE;
}

void VI2S_Close(UINT8 u8VI2SNo)
{
	I2S_T *pI2S = g_I2SHwInfo[u8VI2SNo].pHwAddr;
	
	// Disable TX/RX I2S.
	pI2S->CTL &= ~(I2S_CTL_I2SEN_Msk|I2S_CTL_TXEN_Msk|I2S_CTL_RXEN_Msk);
	// Disable all interrupt.
	pI2S->IEN = 0;
	// Close I2S module.
	I2S_Close(g_I2SHwInfo[u8VI2SNo].pHwAddr);
	// Disable module clock.
	CLK_DisableModuleClock(g_I2SHwInfo[u8VI2SNo].u32ModuleID);	
	// Reset I2S data buffer structure.
	memset(g_asVI2SDataBuf,'\0',sizeof(S_VI2S_DATABUF));
}

void VI2S_Start(UINT8 u8VI2SNo)
{
	I2S_T *pI2S = (I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr;
	
	// Enable module clock
	CLK_EnableModuleClock(g_I2SHwInfo[u8VI2SNo].u32ModuleID);

	// Enable RX interrupt	
	if(g_asVI2SControl[u8VI2SNo].bRXINT == TRUE)
		I2S_EnableInt(pI2S, I2S_IEN_RXTHIEN_Msk);
	// Enable TX interrupt
	if((g_asVI2SControl[u8VI2SNo].bTXINT == TRUE) && (g_asVI2SDataBuf[u8VI2SNo].pu32TxBuf != 0) )
		I2S_EnableInt(pI2S, I2S_IEN_TXTHIEN_Msk);
	
	// Enable receive/transmit data.
	if(g_asVI2SDataBuf[u8VI2SNo].pu32RxBuf != NULL)
		I2S_ENABLE_RX(pI2S);
	if(g_asVI2SDataBuf[u8VI2SNo].pu32TxBuf != NULL)
		I2S_ENABLE_TX(pI2S);
	
	// ENable I2S
	pI2S->CTL |= I2S_CTL_I2SEN_Msk;
}

void VI2S_Stop(UINT8 u8VI2SNo)
{
	I2S_T *pI2S = g_I2SHwInfo[u8VI2SNo].pHwAddr;

	// Disable TX/RX I2S.
	pI2S->CTL &= ~(I2S_CTL_I2SEN_Msk|I2S_CTL_TXEN_Msk|I2S_CTL_RXEN_Msk);
}

void VI2S_Config(UINT8 u8VI2SNo, UINT32 u32Configuration)
{
	I2S_T *pI2S = g_I2SHwInfo[u8VI2SNo].pHwAddr;
	
	// Master/Slave
	if( (u32Configuration & (VI2S_CFG_MASTER|VI2S_CFG_SLAVE)) == VI2S_CFG_MASTER)
		pI2S->CTL &= ~I2S_CTL_SLAVE_Msk;
	else if( (u32Configuration & (VI2S_CFG_MASTER|VI2S_CFG_SLAVE)) == VI2S_CFG_SLAVE)
	{
		pI2S->CTL |= I2S_CTL_SLAVE_Msk;
		I2S_DisableMCLK(pI2S);// Disable master clock
	}
	
	// Mono/Stereo
	if( (u32Configuration & (VI2S_CFG_STEREO|VI2S_CFG_MONO)) == VI2S_CFG_MONO)
		pI2S->CTL |= I2S_CTL_MONO_Msk;
	else if( (u32Configuration & (VI2S_CFG_STEREO|VI2S_CFG_MONO)) == VI2S_CFG_STEREO)
		pI2S->CTL &= ~I2S_CTL_MONO_Msk;
	
	// Word sizes setting
	switch((u32Configuration & (VI2S_CFG_WORD8|VI2S_CFG_WORD16|VI2S_CFG_WORD24|VI2S_CFG_WORD32)))
	{
		case VI2S_CFG_WORD8:
			pI2S->CTL = (pI2S->CTL&~I2S_CTL_WDWIDTH_Msk)|I2S_DATABIT_8;
			break;
		case VI2S_CFG_WORD16:
			pI2S->CTL = (pI2S->CTL&~I2S_CTL_WDWIDTH_Msk)|I2S_DATABIT_16;
			break;
		case VI2S_CFG_WORD24:
			pI2S->CTL = (pI2S->CTL&~I2S_CTL_WDWIDTH_Msk)|I2S_DATABIT_24;
			break;
		case VI2S_CFG_WORD32:
			pI2S->CTL = (pI2S->CTL&~I2S_CTL_WDWIDTH_Msk)|I2S_DATABIT_32;
			break;
		default:
			break;
	}
	
	// Format setting
	if( (u32Configuration & (VI2S_CFG_FORMATI2S|VI2S_CFG_FORMATMSB)) == VI2S_CFG_FORMATI2S)
		pI2S->CTL &= ~I2S_CTL_FORMAT_Msk;
	else if( (u32Configuration & (VI2S_CFG_FORMATI2S|VI2S_CFG_FORMATMSB)) == VI2S_CFG_FORMATMSB)
		pI2S->CTL |= I2S_CTL_FORMAT_Msk;
	
	// Set TX threshold or interrupt disable
	switch(u32Configuration & (VI2S_CFG_TXTH1|VI2S_CFG_TXTH4|VI2S_CFG_TXTH8|VI2S_CFG_TXINTDIS))
	{
		case VI2S_CFG_TXTH1:
			pI2S->CTL &= ~I2S_CTL_TXTH_Msk;
			g_asVI2SControl[u8VI2SNo].bTXINT = TRUE;
		break;
		case VI2S_CFG_TXTH4:
			pI2S->CTL = I2S_FIFO_TX_LEVEL_WORD_3  | (pI2S->CTL & ~I2S_CTL_TXTH_Msk);
			g_asVI2SControl[u8VI2SNo].bTXINT = TRUE;
		break;
		case VI2S_CFG_TXTH8:
			pI2S->CTL = I2S_FIFO_TX_LEVEL_WORD_7  | (pI2S->CTL & ~I2S_CTL_TXTH_Msk);
			g_asVI2SControl[u8VI2SNo].bTXINT = TRUE;
		break;
		case VI2S_CFG_TXINTDIS:
			g_asVI2SControl[u8VI2SNo].bTXINT = FALSE;
			I2S_DisableInt(pI2S,I2S_IEN_TXTHIEN_Msk);
		break;
		default:
			break;
	}

	// Set RX threshold or interrupt disable
	switch(u32Configuration & (VI2S_CFG_RXTH1|VI2S_CFG_RXTH4|VI2S_CFG_RXTH8|VI2S_CFG_RXINTDIS))
	{
		case VI2S_CFG_RXTH1:
			pI2S->CTL &= ~I2S_CTL_RXTH_Msk;
			g_asVI2SControl[u8VI2SNo].bRXINT = TRUE;
		break;
		case VI2S_CFG_RXTH4:
			pI2S->CTL = I2S_FIFO_RX_LEVEL_WORD_4  | (pI2S->CTL & ~I2S_CTL_RXTH_Msk);
			g_asVI2SControl[u8VI2SNo].bRXINT = TRUE;
		break;
		case VI2S_CFG_RXTH8:
			pI2S->CTL = I2S_FIFO_RX_LEVEL_WORD_8  | (pI2S->CTL & ~I2S_CTL_RXTH_Msk);
			g_asVI2SControl[u8VI2SNo].bRXINT = TRUE;
		break;
		case VI2S_CFG_RXINTDIS:
			g_asVI2SControl[u8VI2SNo].bRXINT = FALSE;
			I2S_DisableInt(pI2S,I2S_IEN_RXTHIEN_Msk);
		break;
		default:
			break;
	}
	
	// Set left channel zero interrupt
	switch(u32Configuration & (VI2S_CFG_LEFTZEROEN | VI2S_CFG_LEFTZERODIS))
	{
		case VI2S_CFG_LEFTZEROEN:
			g_asVI2SControl[u8VI2SNo].bLZINT = TRUE;
			pI2S->CTL |= I2S_CTL_LZCEN_Msk;
			pI2S->IEN |= I2S_IEN_LZCIEN_Msk;
		break;
		case VI2S_CFG_LEFTZERODIS:
			g_asVI2SControl[u8VI2SNo].bLZINT = FALSE;
			pI2S->CTL &= ~I2S_CTL_LZCEN_Msk;
			pI2S->IEN &= ~I2S_IEN_LZCIEN_Msk;
		break;
		default:
			break;
	}
	// Set right channel zero interrupt
	switch(u32Configuration & (VI2S_CFG_RIGHTZEROEN | VI2S_CFG_RIGHTZERODIS))
	{
		case VI2S_CFG_RIGHTZEROEN:
			g_asVI2SControl[u8VI2SNo].bRZINT = TRUE;
			pI2S->CTL |= I2S_CTL_RZCEN_Msk;
			pI2S->IEN |= I2S_IEN_RZCIEN_Msk;
		break;
		case VI2S_CFG_RIGHTZERODIS:
			g_asVI2SControl[u8VI2SNo].bRZINT = FALSE;
			pI2S->CTL &= ~I2S_CTL_RZCEN_Msk;
			pI2S->IEN &= ~I2S_IEN_RZCIEN_Msk;
		break;
		default:
			break;
	}
}

void VI2S_SetMasterClock(UINT8 u8VI2SNo, UINT32 u32MasterClock, BOOL bEnable)
{
	I2S_T *pI2S = g_I2SHwInfo[u8VI2SNo].pHwAddr;
	
	if(bEnable)
		// Enable master clock and set frequency at u32MasterClock.
		I2S_EnableMCLK(pI2S, u32MasterClock);
	else
		// Disable master clock.
		I2S_DisableMCLK(pI2S);
}

void VI2S_SetTxData(UINT8 u8VI2SNo,PUINT32 pu32Data,UINT32 u32DataCount)
{
	I2S_T *pI2S = (I2S_T *)g_I2SHwInfo[u8VI2SNo].pHwAddr;
	// Set data buffer pointer and count.
	g_asVI2SDataBuf[u8VI2SNo].u16TxCount = u32DataCount;
	g_asVI2SDataBuf[u8VI2SNo].pu32TxBuf = pu32Data;
	// Shift data to FIFO buffer.
	while(!(pI2S->STATUS & I2S_STATUS_TXFULL_Msk))
	{
		I2S_WRITE_TX_FIFO(pI2S,*(g_asVI2SDataBuf[u8VI2SNo].pu32TxBuf));
		g_asVI2SDataBuf[u8VI2SNo].pu32TxBuf++;
		g_asVI2SDataBuf[u8VI2SNo].u16TxCount--;
	}
	// If I2S is running then enable TX.
	if(pI2S->CTL & I2S_CTL_I2SEN_Msk)
	{
		I2S_ENABLE_TX((I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr);
		if( g_asVI2SControl[u8VI2SNo].bTXINT == TRUE )
			// If interrupt is configured enable TX interrupt.
			I2S_EnableInt((I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr, I2S_IEN_TXTHIEN_Msk);
	}
}

void VI2S_SetRxData(UINT8 u8VI2SNo,PUINT32 pu32Data,UINT32 u32DataCount)
{
	I2S_T *pI2S = (I2S_T *)g_I2SHwInfo[u8VI2SNo].pHwAddr;
	// Set data buffer pointer and count.
	g_asVI2SDataBuf[u8VI2SNo].u16RxCount = u32DataCount;
	g_asVI2SDataBuf[u8VI2SNo].pu32RxBuf = pu32Data;
	// If I2S is running then enable RX.
	if(pI2S->CTL & I2S_CTL_I2SEN_Msk)
	{
		I2S_ENABLE_RX((I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr);
		if( g_asVI2SControl[u8VI2SNo].bRXINT == TRUE )
			// If interrupt is configured then enable RX interrupt.
			I2S_EnableInt((I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr, I2S_IEN_RXTHIEN_Msk);
	}
}

BOOL VI2S_IsBusy(UINT8 u8VI2SNo)
{
	if( g_asVI2SDataBuf[u8VI2SNo].u16RxCount || g_asVI2SDataBuf[u8VI2SNo].u16TxCount || ((I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr)->STATUS&I2S_STATUS_TXBUSY_Msk)
		return TRUE;

	return FALSE;
}

void VI2S_WaitComplete(UINT8 u8VI2SNo)
{
	while(VI2S_IsBusy(u8VI2SNo) == TRUE);
}

UINT32 VI2S_RxProcess(UINT8 u8VI2SNo)
{
        UINT32 u32Data = 0;
	I2S_T *pI2S = g_I2SHwInfo[u8VI2SNo].pHwAddr;
	
	if( g_asVI2SDataBuf[u8VI2SNo].u16RxCount == 0 )
	{
		if( (pI2S->STATUS & I2S_STATUS_RXEMPTY_Msk) == FALSE)
		{
			// If RX buffer data count = 0 and RX has data to receive
			// Return ReceiveRequest State Flag or enter Call Back function.
			#if (VI2S_PROCESSEVENT)
			return VI2S_STA_RECREQ;
			#else
			VI2S_ReceiveRequest(u8VI2SNo);
			#endif
		}
		else
			return 0;
	}
	while(!(pI2S->STATUS & I2S_STATUS_RXEMPTY_Msk))
	{
		// Read data from I2S data register to buffer.
                u32Data = I2S_READ_RX_FIFO(pI2S);
		*(g_asVI2SDataBuf[u8VI2SNo].pu32RxBuf) = u32Data;
		// Point to next data.
		g_asVI2SDataBuf[u8VI2SNo].pu32RxBuf++;
		if((g_asVI2SDataBuf[u8VI2SNo].u16RxCount-=1)==0)
		{
			I2S_DisableInt(pI2S, I2S_IEN_RXTHIEN_Msk);
			// Return ReceiveComplete State Flag or enter Call Back function.
			#if (VI2S_PROCESSEVENT)
			return VI2S_STA_RECCOMP;
			#else
			VI2S_ReceiveComplete(u8VI2SNo);
			return 0;
			#endif
		}
	}
	
	return 0;
}

UINT32 VI2S_TxProcess(UINT8 u8VI2SNo)
{
	I2S_T *pI2S = g_I2SHwInfo[u8VI2SNo].pHwAddr;
	
	if( g_asVI2SDataBuf[u8VI2SNo].u16TxCount == 0 )
	{
		// If TX data count = 0 then disable TX interrupt.
		I2S_DisableInt(pI2S, I2S_IEN_TXTHIEN_Msk);
		return 0;
	}
	while(!(pI2S->STATUS & I2S_STATUS_TXFULL_Msk))
	{
		// Write data from buffer to I2S data register.
		I2S_WRITE_TX_FIFO(pI2S,*(g_asVI2SDataBuf[u8VI2SNo].pu32TxBuf));
		// Point to next data.
		g_asVI2SDataBuf[u8VI2SNo].pu32TxBuf++;
		// Check if TX buffer is empty.
		if( (g_asVI2SDataBuf[u8VI2SNo].u16TxCount-=1)==0)
		{
			// Disable TX interrupt.
			I2S_DisableInt(pI2S, I2S_IEN_TXTHIEN_Msk);
			// Enable Tx Underflow iterrupt to check if TX transmission is finished.
			pI2S->IEN |= I2S_IEN_TXUDIEN_Msk;
			return 0;
		}
	}
	
	return 0;
}

UINT32 VI2S_Process(UINT8 u8VI2SNo)
{
	I2S_T *pI2S = ((I2S_T*)g_I2SHwInfo[u8VI2SNo].pHwAddr);
	UINT32 u32State = 0;
	
	// Receive FIFO threshold interrupt occurred. //
	if((pI2S->STATUS&I2S_STATUS_RXTHIF_Msk) && (pI2S->CTL&I2S_CTL_RXEN_Msk) && (pI2S->IEN&I2S_IEN_RXTHIEN_Msk))
	{
		// Enter RX Process.
		u32State |= VI2S_RxProcess(u8VI2SNo);
	}
	// Transmit FIFO threshold interrupt occurred. //
	if((pI2S->STATUS&I2S_STATUS_TXTHIF_Msk) && (pI2S->CTL&I2S_CTL_TXEN_Msk) && (pI2S->IEN&I2S_IEN_TXTHIEN_Msk))
	{
		// Enter TX Process.
		u32State |= VI2S_TxProcess(u8VI2SNo);
	}
	// Transmit FIFO empty interrupt occurred. //
	if((pI2S->STATUS&I2S_STATUS_TXUDIF_Msk) && (pI2S->IEN&I2S_IEN_TXUDIEN_Msk) && g_asVI2SDataBuf[u8VI2SNo].u16TxCount==0)
	{
		// When TX FIFO is empty then disable TX and TX underflow interrupt
		I2S_DISABLE_TX(pI2S);
		I2S_DisableInt(pI2S, I2S_IEN_TXUDIEN_Msk);
		// Return SendComplete State Flag or enter Call Back function.
		#if (VI2S_PROCESSEVENT)
		u32State |= VI2S_STA_SENDCOMP;
		#else
		VI2S_SendComplete(u8VI2SNo);
		#endif
	}
	// When left channel zero occurred.
	if((pI2S->STATUS & I2S_STATUS_LZCIF_Msk) && (pI2S->IEN & I2S_IEN_LZCIEN_Msk))
	{
		#if (VI2S_PROCESSEVENT)
		u32State |= VI2S_STA_LEFTZERO;
		#else
		VI2S_LeftZero(u8VI2SNo);
		#endif
	}
	// When right channel zero occurred.
	if((pI2S->STATUS & I2S_STATUS_RZCIF_Msk) && (pI2S->IEN & I2S_IEN_RZCIEN_Msk))
	{
		#if (VI2S_PROCESSEVENT)
		u32State |= VI2S_STA_RIGHTZERO;
		#else
		VI2S_RightZero(u8VI2SNo);
		#endif
	}
	
	// Clear Pending Interrupt Request
	NVIC_ClearPendingIRQ(g_I2SHwInfo[u8VI2SNo].eHwIRQn);
	
	// Use polling to enter TX/RX Process.
	if(!g_asVI2SControl[u8VI2SNo].bTXINT)
		u32State |= VI2S_TxProcess(u8VI2SNo);
	if(!g_asVI2SControl[u8VI2SNo].bRXINT)
		u32State |= VI2S_RxProcess(u8VI2SNo);
	if(!g_asVI2SControl[u8VI2SNo].bLZINT && (pI2S->STATUS & I2S_STATUS_LZCIF_Msk))
	{
		#if (VI2S_PROCESSEVENT)
		u32State |= VI2S_STA_LEFTZERO;
		#else
		VI2S_LeftZero(u8VI2SNo);
		#endif
	}
	if(!g_asVI2SControl[u8VI2SNo].bRZINT && (pI2S->STATUS & I2S_STATUS_RZCIF_Msk))
	{
		#if (VI2S_PROCESSEVENT)
		u32State |= VI2S_STA_RIGHTZERO;
		#else
		VI2S_RightZero(u8VI2SNo);
		#endif
	}
	
	return u32State;
}

#if (!VI2S_PROCESSEVENT)
__weak void VI2S_SendComplete(UINT8 u8VI2SNo)
{
};

__weak void VI2S_ReceiveComplete(UINT8 u8VI2SNo)
{	
};

__weak void VI2S_ReceiveRequest(UINT8 u8VI2SNo)
{	
};

__weak void	 VI2S_LeftZero(UINT8 u8VI2SNo)
{
};

__weak void 	 VI2S_RightZero(UINT8 u8VI2SNo)
{
};
#endif

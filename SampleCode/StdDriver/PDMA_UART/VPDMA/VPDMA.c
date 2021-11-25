/*******************************************************************************
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
 #include "VPDMA.h"
 
 #define VPDMA_COUNT								(1)
 #define VPDMA_CHANNEL_COUNT        (4)
 
// Structure for VPDMA channel control. //
typedef struct
{
		const void		*pChanAdd;				// PDMA channel address.
		__IO UINT32		u32Module;				// Module
		__IO BOOL		bONESHOTINT:1;			// RX interrupt enable flag.
		__IO BOOL		bWRAPINT:1;				// TX interrupt enable flag.
		__IO BOOL		bABORTINT:1;			// Abort interrupt enable flag.
} S_VPDMA_CTRL;

// Declare information of each VPDMA channel.
S_VPDMA_CTRL g_asVPDMAControl[VPDMA_CHANNEL_COUNT] = 
{
	{PDMA0, 0x10, FALSE, FALSE, FALSE},
	{PDMA1, 0x10, FALSE, FALSE, FALSE},
	{PDMA2, 0x10, FALSE, FALSE, FALSE},
	{PDMA3, 0x10, FALSE, FALSE, FALSE},
};

// Declare hardware information of VPDMA. //
const S_VHW_INFO g_PDMAHwInfo[VPDMA_COUNT] =          
{                                             
	{PDMAC, PDMA_IRQn, PDMA_MODULE, PDMA_RST, NULL}
};

// Local functions==========================
void VPDMA_SetOneShot(UINT32 u32Ch)
{
	PDMA_T *pdma = (PDMA_T *)g_asVPDMAControl[u32Ch].pChanAdd;
	// Set one-shot mode
	pdma->CTL = (pdma->CTL & (~PDMA_CTL_WAINTSEL_Msk));
	// Enable one-shot interrupt
	pdma->INTEN = (pdma->INTEN | ~PDMA_INTEN_WRAPIEN_Msk) | PDMA_INTEN_TXIEN_Msk;
}

void VPDMA_SetWraparound(UINT32 u32Ch, UINT32 u32Config)
{
	PDMA_T *pdma = (PDMA_T *)g_asVPDMAControl[u32Ch].pChanAdd;
	UINT32 u32Tmp;
	// Set wraparound mode
	u32Tmp = u32Config & (VPDMA_CFG_WRAPAROUND_HALF|VPDMA_CFG_WRAPAROUND_FULL|VPDMA_CFG_WRAPAROUND_BOTH);
	switch(u32Tmp)
	{
		case VPDMA_CFG_WRAPAROUND_HALF:
			pdma->CTL = (pdma->CTL & (~PDMA_CTL_WAINTSEL_Msk) ) | PDMA_HALF_WRAP_MODE << PDMA_CTL_WAINTSEL_Pos;
		break;
		case VPDMA_CFG_WRAPAROUND_FULL:
			pdma->CTL = (pdma->CTL & (~PDMA_CTL_WAINTSEL_Msk) ) | PDMA_FULL_WRAP_MODE << PDMA_CTL_WAINTSEL_Pos;
		break;
		case VPDMA_CFG_WRAPAROUND_BOTH:
			pdma->CTL = (pdma->CTL & (~PDMA_CTL_WAINTSEL_Msk) ) | PDMA_BOTH_WRAP_MODE << PDMA_CTL_WAINTSEL_Pos;
		break;
	}
	// Enable wraparound interrupt
	pdma->INTEN |= PDMA_INTEN_WRAPIEN_Msk;
}

void VPDMA_SetSource(UINT32 u32Ch, UINT32 u32Configuration)
{
	switch(u32Configuration)
	{
		case PDMA_SPI0_RX:
			PDMA_SET_SRC_ADDR(u32Ch, (UINT32)&SPI0->RX);
			SPI0->PDMACTL |= SPI0_PDMACTL_RXPDMAEN_Msk;
			break;
		case PDMA_I2S_RX:
			PDMA_SET_SRC_ADDR(u32Ch, (UINT32)&I2S->RX);
			I2S->CTL |= I2S_CTL_RXPDMAEN_Msk;
			break;
		case PDMA_UART0_RX:
			PDMA_SET_SRC_ADDR(u32Ch, (UINT32)&UART0->DAT);
			UART0->INTEN |= UART_INTEN_DMARXEN_Msk;
			break;
		case PDMA_SPI1_RX:
			PDMA_SET_SRC_ADDR(u32Ch, (UINT32)&SPI1->RX0);
			SPI1->PDMACTL |= SPI1_PDMACTL_RXMDAEN_Msk;
			break;
		case PDMA_UART1_RX:
			PDMA_SET_SRC_ADDR(u32Ch, (UINT32)&UART1->DAT);
			UART1->INTEN |= UART_INTEN_DMARXEN_Msk;
			break;
		case PDMA_SDADC:
			PDMA_SET_SRC_ADDR(u32Ch, (UINT32)&SDADC->DAT);
			SDADC->PDMACTL |= SDADC_PDMACTL_PDMAEN_Msk;
			break;
		case PDMA_SARADC:
			//PDMA_SET_SRC_ADDR(u32Ch, (UINT32)&SARADC->DAT[0]+((u32Configuration>>VPDMA_SHIFT_SARADC_CH)&0xf)*0x04);
			SARADC->CTL |= SARADC_CTL_PDMAEN_Msk;
			break;
		default:
			break;
	}
}

void VPDMA_SetDestination(UINT32 u32Ch, UINT32 u32Configuration)
{
	switch(u32Configuration)
	{
		case PDMA_SPI0_TX:
			PDMA_SET_DST_ADDR(u32Ch, (UINT32)&SPI0->TX);
			break;
		case PDMA_I2S_TX:
			PDMA_SET_DST_ADDR(u32Ch, (UINT32)&I2S->TX);
			break;
		case PDMA_UART0_TX:
			PDMA_SET_DST_ADDR(u32Ch, (UINT32)&UART0->DAT);
			break;
		case PDMA_SPI1_TX:
			PDMA_SET_DST_ADDR(u32Ch, (UINT32)&SPI1->TX0);
			break;
		case PDMA_UART1_TX:
			PDMA_SET_DST_ADDR(u32Ch, (UINT32)&UART1->DAT);
			break;
		case PDMA_DPWM:
			PDMA_SET_DST_ADDR(u32Ch, (UINT32)&DPWM->DATA);
			break;
		default:
			break;
	}
}

void VPDMA_ModuleEnable(UINT32 u32Module)
{
	switch(u32Module)
	{
		case PDMA_SPI0_RX:
			SPI0->PDMACTL |= SPI0_PDMACTL_RXPDMAEN_Msk;
			break;
		case PDMA_I2S_RX:
			I2S->CTL |= I2S_CTL_RXPDMAEN_Msk;
			break;
		case PDMA_UART0_RX:
			UART0->INTEN |= UART_INTEN_DMARXEN_Msk;
			break;
		case PDMA_SPI1_RX:
			SPI1->PDMACTL |= SPI1_PDMACTL_RXMDAEN_Msk;
			break;
		case PDMA_UART1_RX:
			UART1->INTEN |= UART_INTEN_DMARXEN_Msk;
			break;
		case PDMA_SDADC:
			SDADC->PDMACTL |= SDADC_PDMACTL_PDMAEN_Msk;
			break;
		case PDMA_SARADC:
			SARADC->CTL |= SARADC_CTL_PDMAEN_Msk;
			break;
		case PDMA_SPI0_TX:
			SPI0->PDMACTL |= SPI0_PDMACTL_TXPDMAEN_Msk;
			break;
		case PDMA_I2S_TX:
			I2S->CTL |= I2S_CTL_TXPDMAEN_Msk;
			break;
		case PDMA_UART0_TX:
			UART0->INTEN |= UART_INTEN_DMATXEN_Msk;
			break;
		case PDMA_SPI1_TX:
			SPI1->PDMACTL |= SPI1_PDMACTL_TXMDAEN_Msk;
			break;
		case PDMA_UART1_TX:
			UART1->INTEN |= UART_INTEN_DMATXEN_Msk;
			break;
		case PDMA_DPWM:
			DPWM->DMACTL |= DPWM_DMACTL_DMAEN_Msk;
			break;
		default:
			break;
	}
}

// Virtual PDMA Common Function ========================
void VPDMA_Open(void)
{
	// Select clock source and enable module clock.
	CLK_EnableModuleClock(PDMA_MODULE);	
	// Reset IP 
	SYS_ResetModule(PDMA_RST);
}

void VPDMA_Close(void)
{
	// Close PDMA module.
	PDMA_Close();
	// Disable PDMA module clock.
	CLK_DisableModuleClock(PDMA_MODULE);
}


// u32Ch - PDMA channel number. 
void VPDMA_Start(UINT32 u32Ch)
{
	// Enable module's PDMA function
	VPDMA_ModuleEnable(g_asVPDMAControl[u32Ch].u32Module);
	// Enable PDMA channel
	PDMA_Trigger(u32Ch);
}

// u32Ch - PDMA channel number. 
void VPDMA_Stop(UINT32 u32Ch)
{
	PDMA_STOP(u32Ch);
}

// u32Ch - PDMA channel number. 
// u32PeripheralCfg - Peripheral Configuration Selection
// u16Count - Transfer byte count.
// u32SrcAdd - Source address.
// u32DesAdd - Destination address.
// e.g. VPDMA_Config(VPDMA_CHANNEL0, VPDMA_CFG_SPI0_TX(VPDMA_WIDTH_8, VPDMA_ONESHOT_MODE), 16, &SourceBuff, NULL):
void VPDMA_Config(UINT32 u32Ch, UINT32 u32PeripheralCfg, UINT16 u16Count, UINT32 u32SrcAdd, UINT32 u32DesAdd)
{
	PDMA_T *pdma = (PDMA_T *)g_asVPDMAControl[u32Ch].pChanAdd;
	//UINT32 u32Tmp = u32PeripheralCfg >> VPDMA_SHIFT_CFG;
	g_asVPDMAControl[u32Ch].u32Module = u32PeripheralCfg >> VPDMA_SHIFT_CFG;
	
	// Reset channel configuration.
	VPDMA_ChannelReset(u32Ch);
	// PDMA Controller Channel Clock Enable
	PDMA_Open( BIT0 << u32Ch );	
	
	// PDMA channel is connected to which peripheral transmit request.
	PDMA_SetTransferMode( u32Ch, g_asVPDMAControl[u32Ch].u32Module);
	
	// Configuration for Memory-to-Memory mode
	if (g_asVPDMAControl[u32Ch].u32Module == PDMA_MEM)
	{
		// Set address behavior mode.
		pdma->CTL = (pdma->CTL & ~(PDMA_CTL_SASEL_Msk | PDMA_CTL_DASEL_Msk)) | (PDMA_SAR_INC | PDMA_DAR_INC);
		// Set transfer mode to one-shot.
		VPDMA_SetOneShot(u32Ch);
		// Set source address.
		PDMA_SET_SRC_ADDR(u32Ch, u32SrcAdd);
		// Set destination address.
		PDMA_SET_DST_ADDR(u32Ch, u32DesAdd);
	}
	// Configuration for Memory-to-IP/IP-to-Memory mode
	// VPDMA_CFG_RX
	if ((u32PeripheralCfg & (VPDMA_CFG_RX | VPDMA_CFG_TX)) == VPDMA_CFG_RX)
	{
		if ( u32PeripheralCfg & VPDMA_CFG_ONESHOT ) //VPDMA_CFG_ONESHOT
		{
			// Set address behavior mode.
			pdma->CTL = (pdma->CTL & ~(PDMA_CTL_SASEL_Msk | PDMA_CTL_DASEL_Msk)) | (PDMA_SAR_FIX | PDMA_DAR_INC);
			// Set transfer mode to one-shot.
			VPDMA_SetOneShot(u32Ch);
		}
		else
		{
			// Set address behavior mode.
			pdma->CTL = (pdma->CTL & ~(PDMA_CTL_SASEL_Msk | PDMA_CTL_DASEL_Msk)) | (PDMA_SAR_FIX | PDMA_DAR_WRA);
			// Set transfer mode to wraparound.
			VPDMA_SetWraparound(u32Ch, u32PeripheralCfg);
		}
		// Set source address.
		VPDMA_SetSource(u32Ch, g_asVPDMAControl[u32Ch].u32Module);
		// SARADC has multiple data registers
		if(g_asVPDMAControl[u32Ch].u32Module == PDMA_SARADC)
			PDMA_SET_SRC_ADDR(u32Ch, (UINT32)&SARADC->DAT[0]+((u32PeripheralCfg>>VPDMA_SHIFT_SARADC_CH)&0xf)*0x04);
		// Set destination address.
		PDMA_SET_DST_ADDR(u32Ch, u32DesAdd);
	}
	else//VPDMA_CFG_TX
	{
		if (u32PeripheralCfg & VPDMA_CFG_ONESHOT) //VPDMA_CFG_ONESHOT
		{
			// Set address behavior mode.
			pdma->CTL = (pdma->CTL & ~(PDMA_CTL_SASEL_Msk | PDMA_CTL_DASEL_Msk)) | (PDMA_SAR_INC | PDMA_DAR_FIX);
			// Set transfer mode to one-shot.
			VPDMA_SetOneShot(u32Ch);
		}
		else //VPDMA_CFG_WRAPAROUND
		{
			// Set address behavior mode.
			pdma->CTL = (pdma->CTL & ~(PDMA_CTL_SASEL_Msk | PDMA_CTL_DASEL_Msk)) | (PDMA_SAR_WRA | PDMA_DAR_FIX);
			// Set transfer mode to wraparound.
			VPDMA_SetWraparound(u32Ch, u32PeripheralCfg);
		}
		// Set source address.
		PDMA_SET_SRC_ADDR(u32Ch, u32SrcAdd);
		// Set destination address.
		VPDMA_SetDestination(u32Ch, g_asVPDMAControl[u32Ch].u32Module);
	}
	
	// 8/16/32 bits is transferred for every PDMA operation	
	PDMA_SetTransferCnt( u32Ch, (u32PeripheralCfg&PDMA_CTL_TXWIDTH_Msk), u16Count);
	// Set transfer mode: (SRAM-to-APB)/(SRAM-to-APB)/(SRAM-to-SRAM).
	PDMA_SetTransferDirection( u32Ch, (u32PeripheralCfg&PDMA_CTL_MODESEL_Msk) );	
	
	// Enable NVIC for PDMA.
	NVIC_ClearPendingIRQ(PDMA_IRQn);
	NVIC_EnableIRQ(PDMA_IRQn);
}

// Virtual UART Special Function =======================
// u32Ch - PDMA channel number.
// e.g. VPDMA_IsActive(VPDMA_CHANNEL0):
// Check PDMA CH0 -> Idle or Finished.
// return : VPDMA is active(TRUE) or not(FALSE).
BOOL VPDMA_IsActive(UINT32 u32Ch)
{
	return PDMA_IS_CH_BUSY(u32Ch);
}

// u32Ch - PDMA channel number.
// return : State of interrupt source.
UINT32 VPDMA_Process(UINT32 u32Ch)
{
	UINT32 u32State = 0;
	
	if (VPDMA_GetIntFlag(u32Ch, VPDMA_ONESHOT_FLAG))
	{
		#if (VPDMA_PROCESSEVENT)
		// Set state of interrupt type
		u32State |= VPDMA_STA_ONESHOT;
		#else
		VPDMA_OneShot(u32Ch);
		#endif
		// Clear interrupt flag.
		PDMA_CLR_CH_INT_FLAG(u32Ch, VPDMA_ONESHOT_FLAG);
	}else if (VPDMA_GetIntFlag(u32Ch, VPDMA_WRAPAROUND_HALF_FLAG))
	{	
		#if (VPDMA_PROCESSEVENT)
		// Set state of interrupt type
		u32State |= VPDMA_STA_WRAPHALF;
		#else
		VPDMA_WrapHalf(u32Ch);
		#endif
		// Clear interrupt flag.
		PDMA_CLR_CH_INT_FLAG(u32Ch, VPDMA_WRAPAROUND_HALF_FLAG);
	}else if (VPDMA_GetIntFlag(u32Ch, VPDMA_WRAPAROUND_END_FLAG))
	{
		#if (VPDMA_PROCESSEVENT)
		// Set state of interrupt type
		u32State |= VPDMA_STA_WRAPFULL;
		#else
		VPDMA_WrapEnd(u32Ch);
		#endif
		// Clear interrupt flag.
		PDMA_CLR_CH_INT_FLAG(u32Ch, VPDMA_WRAPAROUND_END_FLAG);
	}
	if(VPDMA_GetIntFlag(u32Ch, VPDMA_ABORT_FLAG))
	{
		#if (VPDMA_PROCESSEVENT)
		// Set state of interrupt type
		u32State |= VPDMA_STA_ABORT;
		#else
		VPDMA_Abort(u32Ch);
		#endif
		// Clear interrupt flag.
		PDMA_CLR_CH_INT_FLAG(u32Ch, VPDMA_ABORT_FLAG);
	}
	
	return u32State;
}

//---------------------------------------------------------------//
// u32IntSelection - Interrupt Enabled Definitions
// e.g. VPDMA_EnableInt(VPDMA_CHANNEL0, VPDMA_ONESHOT_INT):
//		Enable PDMA Transfer Done Interrupt.
void VPDMA_EnableInt(UINT32 u32Ch, uint32_t u32IntSel)
{
	PDMA_T *pdma = (PDMA_T *)((uint32_t) PDMA0_BASE + (0x100 * u32Ch));
	
	//Wrap Interrupt: Both half and end buffer.
	if ((u32IntSel&0x7) == PDMA_INTEN_WRAPIEN_Msk)
		pdma->CTL = (pdma->CTL & (~PDMA_CTL_WAINTSEL_Msk) ) | (u32IntSel&PDMA_CTL_WAINTSEL_Msk);
	
	PDMA_EnableInt(u32Ch, (u32IntSel&0x7));
}

void VPDMA_DisableInt(UINT32 u32Ch, uint32_t u32IntSel)
{
	PDMA_DisableInt(u32Ch, (u32IntSel&0x7));
}

// u32IntSelection - Interrupt Enabled Definitions
// e.g. VPDMA_GetIntFlag(VPDMA_CHANNEL0, VPDMA_ONESHOT_FLAG):
//		Get PDMA Transfer Done Interrupt flag.
// return : Interrupt flag(TRUE/FALSE)
BOOL VPDMA_GetIntFlag(UINT32 u32Ch, uint32_t u32FlagSel)
{
	return ((PDMA_GET_CH_INT_STS(u32Ch)&(u32FlagSel))?TRUE:FALSE);
}

void VPDMA_ClearIntFlag(UINT32 u32Ch, uint32_t u32FlagSel)
{
	PDMA_CLR_CH_INT_FLAG(u32Ch, u32FlagSel);
}

// e.g. VPDMA_ChannelReset(VPDMA_CHANNEL0):
//		Reset PDMA CH0 internal state machine and pointers.
//		The contents of the control register will not be cleared.
void VPDMA_ChannelReset(UINT32 u32Ch)
{
	PDMA_T *pdma;
    pdma = (PDMA_T *)((uint32_t) PDMA0_BASE + (0x100 * u32Ch));
	
	PDMA_SoftwareReset(u32Ch);
	// Wait for reset, this bit will auto clear after a few clock cycles
	while( pdma->CTL&PDMA_CTL_SWRST_Msk);
}

// pu32CurSrcAddr - returns the source address from which the PDMA transfer is occurring 
// pu32CurDstAddr - returns the destination address to which the PDMA transfer is occurring
// pu32CurCnt - returns the current remaining count of PDMA transfer
// e.g. VPDMA_GetTransferStatus(VPDMA_CHANNEL0, &u32CurSrcAddr, &u32CurDstAddr, &u32CurCnt):
void VPDMA_GetTransferStatus(UINT32 u32Ch, UINT32 *pu32CurSrcAddr, UINT32 *pu32CurDstAddr, UINT32 *pu32CurCnt)
{
	PDMA_T *pdma = (PDMA_T *)((uint32_t) PDMA0_BASE + (0x100 * u32Ch));
	
	*pu32CurSrcAddr = pdma->CURSADDR;
	*pu32CurDstAddr = pdma->CURDADDR;
	
	if ((pdma->CTL & PDMA_CTL_TXWIDTH_Msk) == VPDMA_CFG_WIDTH_32)
        *pu32CurCnt = pdma->CURTXCNT>>2;
    else if ((pdma->CTL & PDMA_CTL_TXWIDTH_Msk) == PDMA_WIDTH_8)
        *pu32CurCnt = pdma->CURTXCNT; 
    else if ((pdma->CTL & PDMA_CTL_TXWIDTH_Msk) == PDMA_WIDTH_16)
        *pu32CurCnt = pdma->CURTXCNT>>1;
}

//---------------------------------------------------------------//
#if (!VPDMA_PROCESSEVENT)
__weak void VPDMA_WrapHalf(UINT8 u8PDMAChNo)
{
};
__weak void VPDMA_WrapEnd(UINT8 u8PDMAChNo)
{
};
__weak void VPDMA_OneShot(UINT8 u8PDMAChNo)
{
};
__weak void VPDMA_Abort(UINT8 u8PDMAChNo)
{
};
#endif

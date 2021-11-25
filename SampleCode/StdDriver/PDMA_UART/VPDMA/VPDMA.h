#ifndef _VPDMA_H_
#define _VPDMA_H_
// ----------------------------------------------------------------------------------------------------------
//	- PDMA call flow:
//		main loop:
//					Vmodule_Open()->Vmodule_Config() (ex.VUART)
//				+-> Vmodule_Start()
//				|	VPDMA_Open()->VPDMA_Config()
//          	|	VPDMA_Start()->
//          	|   VPDMA_Stop()
//				+-> Vmodule_Stop()
// -----------------------------------------------------------------------------------------------------------
#include "VCommon.h"

// Define PDMA channel number                                                                       	  
#define VPDMA_CHANNEL0		(0)
#define VPDMA_CHANNEL1		(1)
#define VPDMA_CHANNEL2		(2)
#define VPDMA_CHANNEL3		(3)

// Peripheral Configuration Selection                                                    	   
#define VPDMA_CFG_TX					(BIT4)
#define VPDMA_CFG_RX					(BIT5)

// PDMA Transfer IP Cpnfiguration Position
#define VPDMA_SHIFT_CFG				(24)				
#define VPDMA_SHIFT_SARADC_CH		(12)

// Configuration Selection  ==================                                                                     
// e.g. VPDMA_CFG_SPI0_TX(VPDMA_WIDTH_8, VPDMA_ONESHOT_MODE): SPI0 uses PDMA CH0 One-Shot mode to send 8-bit width data from SRAM buffer
//		VPDMA_CFG_SARADC has u32ChNo to specify which SARADC data register to be connected to PDMA(SARADC has 16 data registers: 0~15).
// 		e.g. VPDMA_CFG_SARADC(VPDMA_WIDTH_16, VPDMA_ONESHOT_MODE, 5): SARADC data register 5 use PDMA One-Shot mode and to return 16-bit width data.
#define VPDMA_CFG_SPI0_TX(u32Width, u32Mode)        	(PDMA_SPI0_TX<<VPDMA_SHIFT_CFG|VPDMA_CFG_TX|u32Width|PDMA_SRAM_APB|u32Mode)        				/*!<DMA Connect to SPI0 TX */
#define VPDMA_CFG_SPI0_RX(u32Width, u32Mode)        	(PDMA_SPI0_RX<<VPDMA_SHIFT_CFG|VPDMA_CFG_RX|u32Width|PDMA_APB_SRAM|u32Mode)        				/*!<DMA Connect to SPI0 RX */
#define VPDMA_CFG_I2S_TX(u32Width, u32Mode)         	(PDMA_I2S_TX<<VPDMA_SHIFT_CFG|VPDMA_CFG_TX|u32Width|PDMA_SRAM_APB|u32Mode)						/*!<DMA Connect to I2S TX */
#define VPDMA_CFG_I2S_RX(u32Width, u32Mode)         	(PDMA_I2S_RX<<VPDMA_SHIFT_CFG|VPDMA_CFG_RX|u32Width|PDMA_APB_SRAM|u32Mode)         				/*!<DMA Connect to I2S RX */
#define VPDMA_CFG_UART0_TX(u32Width, u32Mode)       	(PDMA_UART0_TX<<VPDMA_SHIFT_CFG|VPDMA_CFG_TX|u32Width|PDMA_SRAM_APB|u32Mode)       				/*!<DMA Connect to UART0 TX */
#define VPDMA_CFG_UART0_RX(u32Width, u32Mode)       	(PDMA_UART0_RX<<VPDMA_SHIFT_CFG|VPDMA_CFG_RX|u32Width|PDMA_APB_SRAM|u32Mode)      				/*!<DMA Connect to UART0 RX */
#define VPDMA_CFG_SDADC(u32Width, u32Mode)          	(PDMA_SDADC<<VPDMA_SHIFT_CFG|VPDMA_CFG_RX|u32Width|PDMA_APB_SRAM|u32Mode)          				/*!<DMA Connect to SDADC */
#define VPDMA_CFG_DPWM(u32Width, u32Mode)           	(PDMA_DPWM<<VPDMA_SHIFT_CFG|VPDMA_CFG_TX|u32Width|PDMA_SRAM_APB|u32Mode)       					/*!<DMA Connect to DPWM */
#define VPDMA_CFG_SPI1_TX(u32Width, u32Mode)        	(PDMA_SPI1_TX<<VPDMA_SHIFT_CFG|VPDMA_CFG_TX|u32Width|PDMA_SRAM_APB|u32Mode)        				/*!<DMA Connect to SPI1 TX */
#define VPDMA_CFG_SPI1_RX(u32Width, u32Mode)        	(PDMA_SPI1_RX<<VPDMA_SHIFT_CFG|VPDMA_CFG_RX|u32Width|PDMA_APB_SRAM|u32Mode)        				/*!<DMA Connect to SPI1 RX */
#define VPDMA_CFG_UART1_TX(u32Width,u32Mode)       		(PDMA_UART1_TX<<VPDMA_SHIFT_CFG|VPDMA_CFG_TX|u32Width|PDMA_SRAM_APB|u32Mode)       				/*!<DMA Connect to UART1 TX */
#define VPDMA_CFG_UART1_RX(u32Width, u32Mode)       	(PDMA_UART1_RX<<VPDMA_SHIFT_CFG|VPDMA_CFG_RX|u32Width|PDMA_APB_SRAM|u32Mode)       				/*!<DMA Connect to UART1 RX */
#define VPDMA_CFG_SARADC(u32Width, u32Mode, u32ChNo)	(PDMA_SARADC<<VPDMA_SHIFT_CFG|VPDMA_CFG_RX|u32Width|PDMA_APB_SRAM|u32Mode|u32ChNo<<VPDMA_SHIFT_SARADC_CH)			/*!<DMA Connect to SARADC */
#define VPDMA_CFG_MEM(u32Width, u32Mode)				(PDMA_MEM<<VPDMA_SHIFT_CFG|u32Width|PDMA_SRAM_SRAM|u32Mode)										/*!<DMA As memory to memory */
// Width configuration and Mode configuration ========
#define VPDMA_CFG_WIDTH_8        										(PDMA_WIDTH_8)         	/*!<DMA Transfer Width 8-bit */
#define VPDMA_CFG_WIDTH_16       										(PDMA_WIDTH_16)        	/*!<DMA Transfer Width 16-bit */
#define VPDMA_CFG_WIDTH_32       										(PDMA_WIDTH_32)         /*!<DMA Transfer Width 32-bit */                                                                   
#define VPDMA_CFG_ONESHOT												(BIT8)     				/*!<DMA One-Shot Transfer Mode */
#define VPDMA_CFG_WRAPAROUND_HALF										(BIT9)      			/*!<DMA Wraparound Half Transfer Mode */
#define VPDMA_CFG_WRAPAROUND_FULL										(BIT10)					/*!<DMA Wraparound Full Transfer Mode */
#define VPDMA_CFG_WRAPAROUND_BOTH										(BIT11)					/*!<DMA Wraparound Both Transfer Mode */
// End of Configuration Selection  ==================

// Process State Flag (Bitwise) ==================
#define VPDMA_STA_ONESHOT												(BIT0)					// One-Shot transfer complete
#define VPDMA_STA_WRAPHALF												(BIT1)					// Wraparound half complete
#define VPDMA_STA_WRAPFULL												(BIT2)					// Wraparound full complete
#define VPDMA_STA_ABORT													(BIT3)					// Transfer target abort interrupt occured

// ===================================================
// When event happened, 
// 0 : Process with callback function
// 1 : Process with return "STATE FLAG"
#define VPDMA_PROCESSEVENT       (0)

// Peripheral FIFO Address Definitions
#define	VPDMA_SPI0_TXFIFO_ADDR											((UINT32)&SPI0->TX)
#define	VPDMA_SPI0_RXFIFO_ADDR											((UINT32)&SPI0->RX)
#define	VPDMA_SPI1_TXFIFO_ADDR											((UINT32)&SPI1->TX)
#define	VPDMA_SPI1_RXFIFO_ADDR											((UINT32)&SPI1->RX)
#define	VPDMA_I2S_TXFIFO_ADDR											((UINT32)&I2S->TX)
#define	VPDMA_I2S_RXFIFO_ADDR											((UINT32)&I2S->RX)
#define	VPDMA_UART0_DATAFIFO_ADDR										((UINT32)&UART0->DAT)
#define	VPDMA_UART1_DATAFIFO_ADDR										((UINT32)&UART1->DAT)
#define	VPDMA_SDADC_DATAFIFO_ADDR										((UINT32)&SDADC->DAT)
#define	VPDMA_DPWM_DATAFIFO_ADDR										((UINT32)&DPWM->DATA)
#define	VPDMA_SARADC_DATAFIFO_ADDR										((UINT32)&SARADC->DAT[0])

// PDMA Interrupt Enabled Definitions
#define VPDMA_ONESHOT_INT												(PDMA_INTEN_TXIEN_Msk)
#define VPDMA_WRAPAROUND_HALF_INT										(PDMA_INTEN_WRAPIEN_Msk|(PDMA_HALF_WRAP_MODE<<PDMA_CTL_WAINTSEL_Pos)) 
#define VPDMA_WRAPAROUND_END_INT										(PDMA_INTEN_WRAPIEN_Msk|(PDMA_FULL_WRAP_MODE<<PDMA_CTL_WAINTSEL_Pos)) 
#define VPDMA_WRAPAROUND_BOTH_INT										(PDMA_INTEN_WRAPIEN_Msk|(PDMA_BOTH_WRAP_MODE<<PDMA_CTL_WAINTSEL_Pos)) 
#define VPDMA_ABORT_INT													(PDMA_INTEN_ABTIEN_Msk)

// PDMA Interrupt Flag Definitions
#define VPDMA_ONESHOT_FLAG												(PDMA_INTSTS_TXIF_Msk)
#define VPDMA_WRAPAROUND_HALF_FLAG										(PDMA_HALF_WRAP_MODE<<PDMA_INTSTS_WRAPIF_Pos) 
#define VPDMA_WRAPAROUND_END_FLAG										(PDMA_FULL_WRAP_MODE<<PDMA_INTSTS_WRAPIF_Pos) 
#define VPDMA_ABORT_FLAG												(PDMA_INTSTS_ABTIF_Msk)

// Virtual PDMA Common Function ========================
void VPDMA_Open(void);

void VPDMA_Close(void);

void VPDMA_Config(UINT32 u32Ch, UINT32 u32PeripheralCfg, UINT16 u16Count, UINT32 u32SrcAddr, UINT32 u32DesAddr);

void VPDMA_Start(UINT32 u32Ch);

void VPDMA_Stop(UINT32 u32Ch);

UINT32 VPDMA_Process(UINT32 u32Ch);

// Virtual PDMA Special Function =======================
BOOL VPDMA_IsActive(UINT32 u32Ch);

//-----------------------------------------------//
void VPDMA_EnableInt(UINT32 u32Ch, uint32_t u32IntSel);

void VPDMA_DisableInt(UINT32 u32Ch, uint32_t u32IntSel);

BOOL VPDMA_GetIntFlag(UINT32 u32Ch, uint32_t u32FlagSel);

void VPDMA_ClearIntFlag(UINT32 u32Ch, uint32_t u32FlagSel);

void VPDMA_ChannelReset(UINT32 u32Ch);

void VPDMA_GetTransferStatus(UINT32 u32Ch, UINT32 *pu32CurSrcAddr, UINT32 *pu32CurDstAddr, UINT32 *pu32CurCnt);

// Virtual PDMA Callback Function ======================
void VPDMA_WrapHalf(UINT8 u8PDMAChNo);

void VPDMA_WrapEnd(UINT8 u8PDMAChNo);

void VPDMA_OneShot(UINT8 u8PDMAChNo);

void VPDMA_Abort(UINT8 u8PDMAChNo);

#endif //_VPDMA_H_

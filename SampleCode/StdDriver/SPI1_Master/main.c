/****************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 20/01/09 19:22p $
 * @brief    I91200 SPI master demo sample source code.
 *
 * @note
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include "Platform.h"

#define TEST_COUNT 16	  //Test data count

__align(4) uint32_t g_au32SourceData[TEST_COUNT];
__align(4) uint32_t g_au32DestinationData[TEST_COUNT];

uint32_t g_u32TxDataCount;
uint32_t g_u32RxDataCount;

// SPI1 Master =========================================================================
#define SPI1_DEVICE0_MASTER_PINS_MSK    (SYS_GPB_MFP_PB0MFP_Msk|SYS_GPB_MFP_PB1MFP_Msk|SYS_GPB_MFP_PB2MFP_Msk|SYS_GPB_MFP_PB3MFP_Msk)
#define SPI1_DEVICE0_MASTER_PINS        (SYS_GPB_MFP_PB0MFP_SPI1_MOSI|SYS_GPB_MFP_PB1MFP_SPI1_SCLK|SYS_GPB_MFP_PB2MFP_SPI1_SSB|SYS_GPB_MFP_PB3MFP_SPI1_MISO)	

void SPI1_Init(void);
void SPI1_IRQHandler(void);

int main(void)
{
	  uint32_t u32DataCount;
	
    // Unlock protected registers
    SYS_UnlockReg();
    // Enable clock source 
    CLK_EnableXtalRC(CLK_PWRCTL_HIRC_EN|CLK_PWRCTL_LIRC_EN);
    // Switch HCLK clock source to HIRC
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	  // Enable LDO 3.3V
	  CLK_EnableLDO(CLK_LDOSEL_3_3V);
	  // Update System Core Clock
    SystemCoreClockUpdate();
	  // Config gpio pin multi-functon.
	  SYS->GPB_MFP = ( SYS->GPB_MFP & ~SYS_GPB_MFP_PB4MFP_Msk ) | SYS_GPB_MFP_PB4MFP_GPIO;
	  // Config gpio debounce clock source & debounce time.
	  GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_512);
	  // Enable debounce pin.
	  GPIO_ENABLE_DEBOUNCE( PB, BIT4);
	  // Config gpio mode.
	  GPIO_SetMode( PB, BIT4, GPIO_MODE_QUASI);
	  // Light off LED DA12 and DA13.
		GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)|(BIT12));
    GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)|(BIT13));
    // Lock protected registers
    SYS_LockReg();

	  /* Init SPI */
	  SPI1_Init();

    printf("\r\n+------------------------------------------------------------------------+\r\n");
    printf("|                      SPI1 Driver Sample Code                           |\r\n");
    printf("+------------------------------------------------------------------------+\r\n");
    printf("Configure SPI1 as a master.\r\n");
    printf("SPI1 clock rate: %d Hz\r\n", SPI1_GetBusClock(SPI1));
    printf("SPI1 controller will transfer %d data to a off-chip slave device.\r\n", TEST_COUNT);
    printf("In the meanwhile the SPI1 controller will receive %d data from the off-chip slave device.\r\n", TEST_COUNT);
    printf("After the transfer is done, the %d received data will be printed out.\r\n", TEST_COUNT);
    printf("The SPI1 master configuration is ready.\r\n");

    for(u32DataCount=0; u32DataCount<TEST_COUNT; u32DataCount++)
    {
        g_au32SourceData[u32DataCount] = 0x55AA5500 + u32DataCount;
        g_au32DestinationData[u32DataCount] = 0;
    }

    printf("Before starting the data transfer, make sure the slave device is ready.\r\n");
    printf("<<Press 'SWB4' button key to start the transfer, the LED DA13 will be light on.>>\r\n");
	  // Wait user press key on EVB board.
	  while( (GPIO_GET_IN_DATA(PB)&BIT4)==BIT4 );
	  // Light on DA13
	  GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)&(~BIT13));
    printf("\r\n");

	  g_u32TxDataCount = 0;
	  g_u32RxDataCount = 0;
	
		SPI1_ENABLE_INTERRUPT(SPI1);
	  NVIC_EnableIRQ(SPI1_IRQn);
		
		// Write first data and 
		SPI1_WRITE_TX0(SPI1, g_au32SourceData[g_u32TxDataCount]);
		SPI1_GO(SPI1);
	
    /* Wait for transfer done */
    while(g_u32RxDataCount<TEST_COUNT)
		{ 
			// To DO
		}

    /* Disable Tx FIFO threshold interrupt and RX FIFO time-out interrupt */
    SPI1_DISABLE_INTERRUPT(SPI1);
	  NVIC_DisableIRQ(SPI1_IRQn);

    printf("Received data:\r\n");
    for(u32DataCount=0; u32DataCount<TEST_COUNT; u32DataCount++)
        printf("%d:\t0x%X\r\n", u32DataCount, g_au32DestinationData[u32DataCount]);
   
    printf("\r\nThe data transfer was done.\r\n"); 
    printf("Exit SPI1 driver master sample code.\r\n");
    while(1);
}

void SPI1_Init(void)
{	 
	/* Enable module clock */
	CLK_EnableModuleClock(SPI1_MODULE);

	/* Reset SPI1 module */
	SYS_ResetModule(SPI1_RST);

	/* Act as Master, type1, width = 32 bit */
	SPI1_Open(SPI1, SPI1_MASTER, SPI1_MODE_0, 4000000, 0);

	SPI1_SET_MSB_FIRST(SPI1);
	
	SPI1_SET_SUSPEND_CYCLE(SPI1,0);
	
	SPI1_DISABLE_BYTE_REORDER(SPI1);
	
	SPI1_SET_DATA_WIDTH(SPI1,32);
	
	SPI1_SET_TX_NUM(SPI1,SPI1_TXNUM_ONE);

	/* Enable the automatic hardware slave select function. Select the SPI1_SS0 pin and configure as low-active. */
	SPI1_ENABLE_AUTOSS(SPI1, SPI1_SS0, SPI1_SS_ACTIVE_LOW);

	// Init I/O multi-function ; 
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SPI1_DEVICE0_MASTER_PINS_MSK) ) | SPI1_DEVICE0_MASTER_PINS;
}

void SPI1_IRQHandler(void)
{
	while(SPI1_GET_RX_FIFO_EMPTY_FLAG(SPI1)==0)
		g_au32DestinationData[g_u32RxDataCount++] = SPI1_READ_RX0(SPI1);
	
	/* Disable TX FIFO threshold interrupt */
	if(++g_u32TxDataCount>=TEST_COUNT)
		SPI1_DISABLE_INTERRUPT(SPI1);
	else
	{
		while(SPI1_GET_TX_FIFO_FULL_FLAG(SPI1)==0)
			SPI1_WRITE_TX0(SPI1, g_au32SourceData[g_u32TxDataCount]);
		
		SPI1_GO(SPI1);
	}
		
	SPI1_CLR_UNIT_TRANS_INT_FLAG(SPI1);
}

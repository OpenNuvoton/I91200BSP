/****************************************************************************//**
 * @file     spi0.h
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/08/17 10:10a $
 * @brief    I91200 SPI driver source file
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include "Platform.h"

#define TEST_COUNT 16	  //Test data count

__align(4) uint32_t g_au32SourceData[TEST_COUNT];
__align(4) uint32_t g_au32DestinationData[TEST_COUNT];

uint32_t g_u32TxDataCount;
uint32_t g_u32RxDataCount;

// SPI0 Master =========================================================================
#define SPI0_DEVICE1_MASTER_PINS_MSK    (SYS_GPA_MFP_PA1MFP_Msk|SYS_GPA_MFP_PA2MFP_Msk|SYS_GPA_MFP_PA3MFP_Msk|SYS_GPA_MFP_PA4MFP_Msk)
#define SPI0_DEVICE1_MASTER_PINS        (SYS_GPA_MFP_PA1MFP_SPI0_MOSI0|SYS_GPA_MFP_PA2MFP_SPI0_SCLK|SYS_GPA_MFP_PA3MFP_SPI0_SSB|SYS_GPA_MFP_PA4MFP_SPI0_MISO0)	

void SPI0_Init(void)
{	 
	/* Enable module clock */
    CLK_EnableModuleClock(SPI0_MODULE);
	
	/* Reset SPI0 module */
	SYS_ResetModule(SPI0_RST);
	
    /* Act as Master, type1, width = 32 bit */
    SPI0_Open(SPI0, SPI0_MASTER, SPI0_MODE_0, 32, 4000000);
	
	SPI0_SET_SUSPEND_CYCLE(SPI0, 0xf);

    /* Enable the automatic hardware slave select function. Select the SPI0_SS0 pin and configure as low-active. */
    SPI0_EnableAutoSS(SPI0, SPI0_SS0, SPI0_SS_ACTIVE_LOW);
	
	SPI0_DISABLE_BYTE_REORDER(SPI0);
	
    /* Enable SPI transfer */
    SPI0_TRIGGER(SPI0);
	
	// Init I/O multi-function ; 
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SPI0_DEVICE1_MASTER_PINS_MSK) ) | SPI0_DEVICE1_MASTER_PINS;
}

void SPI0_IRQHandler(void)
{
    while((SPI0_GET_STATUS(SPI0) & SPI0_STATUS_RXEMPTY_Msk)==0) 
        g_au32DestinationData[g_u32RxDataCount++] = SPI0_READ_RX(SPI0);
	
    while( ((SPI0_GET_STATUS(SPI0) & SPI0_STATUS_TXFULL_Msk)==0) && (g_u32TxDataCount<TEST_COUNT) ) 
        SPI0_WRITE_TX(SPI0, g_au32SourceData[g_u32TxDataCount++]);
	
	/* Disable TX FIFO threshold interrupt */
    if(g_u32TxDataCount>=TEST_COUNT)
        SPI0_DisableInt(SPI0, SPI0_FIFO_TXTHIEN_MASK); 
 
    /* Check the Rx FIFO time-out interrupt flag */
    if( ( SPI0_GET_STATUS(SPI0) & SPI0_STATUS_RXTOIF_Msk ) == 1 ) 
    {
        while((SPI0_GET_STATUS(SPI0) & SPI0_STATUS_RXEMPTY_Msk)==0) 
            g_au32DestinationData[g_u32RxDataCount++] = SPI0_READ_RX(SPI0);
    }
}

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
	  SPI0_Init();

    printf("\r\n+------------------------------------------------------------------------+\r\n");
    printf("|                      SPI0 Driver Sample Code                           |\r\n");
    printf("+------------------------------------------------------------------------+\r\n");
    printf("Configure SPI0 as a master.\r\n");
    printf("SPI0 clock rate: %d Hz\r\n", SPI0_GetBusClock(SPI0));
    printf("SPI0 controller will transfer %d data to a off-chip slave device.\r\n", TEST_COUNT);
    printf("In the meanwhile the SPI0 controller will receive %d data from the off-chip slave device.\r\n", TEST_COUNT);
    printf("After the transfer is done, the %d received data will be printed out.\r\n", TEST_COUNT);
    printf("The SPI0 master configuration is ready.\r\n");

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
    /* Set Tx FIFO threshold, enable Tx FIFO threshold interrupt and RX FIFO time-out interrupt */
    SPI0_SetFIFOThreshold(SPI0, 4, 4);
	
	  SPI0_EnableInt(SPI0, SPI0_FIFO_RXTOIEN_MASK | SPI0_FIFO_TXTHIEN_MASK );
	  NVIC_EnableIRQ(SPI0_IRQn);
	
    /* Wait for transfer done */
    while(g_u32RxDataCount<TEST_COUNT);

    /* Disable Tx FIFO threshold interrupt and RX FIFO time-out interrupt */
    SPI0_DisableInt(SPI0, SPI0_FIFO_RXTOIEN_MASK|SPI0_FIFO_TXTHIEN_MASK);
	  NVIC_DisableIRQ(SPI0_IRQn);

    printf("Received data:\r\n");
    for(u32DataCount=0; u32DataCount<TEST_COUNT; u32DataCount++)
        printf("%d:\t0x%X\r\n", u32DataCount, g_au32DestinationData[u32DataCount]);
   
    printf("\r\nThe data transfer was done.\r\n"); 
    printf("Exit SPI0 driver master sample code.\r\n");
    while(1);
}

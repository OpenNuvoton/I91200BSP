/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/08/17 10:00p $
 * @brief    I91200 SPI slave demo sample source.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"

#define TEST_COUNT 			(16)	    /* Test data count */

__align(4) uint32_t g_au32SourceData[TEST_COUNT];
__align(4) uint32_t g_au32DestinationData[TEST_COUNT];

// SPI Slave =========================================================================
#define SPI1_SLAVE_PINS_MSK    (SYS_GPB_MFP_PB0MFP_Msk|SYS_GPB_MFP_PB1MFP_Msk|SYS_GPB_MFP_PB2MFP_Msk|SYS_GPB_MFP_PB3MFP_Msk)
#define SPI1_SLAVE_PINS        (SYS_GPB_MFP_PB0MFP_SPI1_MOSI|SYS_GPB_MFP_PB1MFP_SPI1_SCLK|SYS_GPB_MFP_PB2MFP_SPI1_SSB|SYS_GPB_MFP_PB3MFP_SPI1_MISO)	

void SPI1_Init()
{
	/* Enable module clock */
    CLK_EnableModuleClock(SPI1_MODULE);
	
	/* Reset SPI0 module */
	SYS_ResetModule(SPI1_RST);
	
    /* Configure SPI1 as a slave, MSB first, SPI Mode-0 timing, clock is from master provide */
    SPI1_Open(SPI1, SPI1_SLAVE, SPI1_MODE_0, 4000000, 0 );

	SPI1_SET_MSB_FIRST(SPI1);
	
	SPI1_SET_SUSPEND_CYCLE(SPI1,0);
	
	SPI1_DISABLE_BYTE_REORDER(SPI1);
	
	SPI1_SET_DATA_WIDTH(SPI1,32);
	
	SPI1_SET_TX_NUM(SPI1,SPI1_TXNUM_ONE);
	
    /* Disable the automatic hardware slave select function */
	SPI1_DISABLE_AUTOSS(SPI1);
	
	SPI1_SET_SS(SPI1,SPI1_SS0);
	
	SPI1_SET_SLAVE_ACTIVE_LEVEL(SPI1,SPI1_SS_ACTIVE_LOW);
	
	// Init I/O multi-function ; 
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SPI1_SLAVE_PINS_MSK) ) | SPI1_SLAVE_PINS;
}

int main()
{
	uint32_t u32Counter = 0;
	
    // Unlock protected registers
    SYS_UnlockReg();
    // Enable clock source 
    CLK_EnableXtalRC(CLK_PWRCTL_HIRC_EN|CLK_PWRCTL_LIRC_EN);
    // Switch HCLK clock source to HIRC
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	  // Enable LDO 3.3V
	  CLK_EnableLDO(CLK_LDOSEL_3_3V);
	  // Update System Core Clock
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
    SystemCoreClockUpdate();
    // Lock protected registers
    SYS_LockReg();	
	
	  /* Initiate SPI */
	  SPI1_Init();
	
    printf("\r\n+------------------------------------------------------------------------+\r\n");
    printf("|                      SPI1 Driver Sample Code                           |\r\n");
    printf("+------------------------------------------------------------------------+\r\n");
	  printf("Configure SPI1 as a slave.\r\n");
    printf("SPI controller will transfer %d data to a off-chip master device.\r\n", TEST_COUNT);
    printf("In the meanwhile the SPI controller will receive %d data from the off-chip master device.\r\n", TEST_COUNT);
    printf("After the transfer is done, the %d received data will be printed out.\r\n", TEST_COUNT);
    printf("The SPI1 slave configuration is ready.\r\n");

    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
    {
        g_au32SourceData[u32Counter] = 0xAA55AA00 + u32Counter;
        g_au32DestinationData[u32Counter] = 0;
    }

    printf("<<Press 'SWB4' button key if the master device configuration is ready, the LED DA13 will be light on.>>\r\n");
	  // Wait user press key on EVB board.
	  while( (GPIO_GET_IN_DATA(PB)&BIT4)==BIT4 );
	  // Light on DA13
	  GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)&(~BIT13));
    printf("\r\n");
	
	  /* Wait for transfer done */
    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
	  {
		  SPI1_WRITE_TX0(SPI1, g_au32SourceData[u32Counter] );
		  SPI1_GO(SPI1);
		  while( SPI1_IS_BUSY(SPI1) );
		  g_au32DestinationData[u32Counter] = SPI1_READ_RX0(SPI1);
	  }
	
    printf("Received data:\r\n");
    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
        printf("%d:\t0x%X\r\n", u32Counter, g_au32DestinationData[u32Counter]);
	
    printf("\r\nThe data transfer was done.\r\n"); 
    printf("Exit SPI1 driver slave sample code.\r\n");
    while(1);
}

/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate SPI data transfer.
    (1) Implement SPI transfer
    (2) Configure SPI1 as SPI Slave mode and demonstrate how SPI works in Slave mode.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connect Pins: 
		 Master < - > SPI1_Slave
         MOSI0  < - > PB0(Slave) MOSI
         SCLK   < - > PB1(Slave) SCLK
         SSB    < - > PB2(Slave) SSB
         MISO0  < - > PB3(Slave) MISO
    (2) Compiled to execute.
    (3) Program test procedure -
		1. Press 'SWB4' button key to start wait for master transmision
		2. We can see slave receive data from master
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample code needs to work with SPI1_Master or SPI0_Master on another chip.
	(2) The received data will be the following:
		0:      0x55AA5500
		1:      0x55AA5501
		2:      0x55AA5502
		3:      0x55AA5503
		4:      0x55AA5504
		5:      0x55AA5505
		6:      0x55AA5506
		7:      0x55AA5507
		8:      0x55AA5508
		9:      0x55AA5509
		10:     0x55AA550A
		11:     0x55AA550B
		12:     0x55AA550C
		13:     0x55AA550D
		14:     0x55AA550E
		15:     0x55AA550F
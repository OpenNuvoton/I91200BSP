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
    (2) Configure SPI1 as SPI Master mode and demonstrate how SPI works in Master mode.
    (3) This sample code will transmit 16 32-bit data, and receive 16 32-bit data at the same time.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connect Pins: 
	     SPI1_Master      < - > Slave
         PB0(Master) MOSI < - > MOSI
         PB1(Master) SCLK < - > CLK
         PB2(Master) SSB  < - > SS
         PB3(Master) MISO < - > MISO
    (2) Run the slave demo first to wait for master transmission 
    (3) Compiled to execute.
    (4) Program test procedure -
		1. Press 'SWB4' button key to start
		2. We can see master receive data from slave
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample code needs to work with SPI1_Slave or SPI0_Slave on another chip.
	(2) The received data will be the following:
		0:      0xAA55AA00
		1:      0xAA55AA01
		2:      0xAA55AA02
		3:      0xAA55AA03
		4:      0xAA55AA04
		5:      0xAA55AA05
		6:      0xAA55AA06
		7:      0xAA55AA07
		8:      0xAA55AA08
		9:      0xAA55AA09
		10:     0xAA55AA0A
		11:     0xAA55AA0B
		12:     0xAA55AA0C
		13:     0xAA55AA0D
		14:     0xAA55AA0E
		15:     0xAA55AA0F
/*******************************************************************
 *
 * Copyright(c) Nuvoton Technology Corp. All rights reserved.
 *
 *******************************************************************/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
   Demonstrate I2S data transfer.
   (1) Implement how I2S works in Slave mode.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connect Pins: 
         Master GPA0(FS) < - > Slave GPA0(FS)
         Master GPA1(BCLK)   < - > Slave GPA1(BCLK)
         Master GPA2(DI)   < - > Slave GPA3(DO)
         Master GPA3(DO)    < - > Slave GPA2(DI)
    (2) Run the slave demo first to wait for master transmission
    (3) Compiled to execute
    (4) Program test procedure -
         1. Press 'SWB0' button key to start
         2. The slave will transmit data to master when it receive data from master.
         3. Show success after the verification of data receive, you can also see LED DA12 is light on
         4. I2S demo end
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    
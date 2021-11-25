/*******************************************************************
 *
 * Copyright(c) Nuvoton Technology Corp. All rights reserved.
 *
 *******************************************************************/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate Watchdog Timer(WDT).
    (1) Using Watch Dog Timer interrupt
    
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to com port to send out demo message (TX=GPA4, RX=GPA5).
    (2) Compiled to execute, and not into debug mode.
    (3) Program test procedure -
	1.program will configure WDT. start WDT and flash LED(DA13).
	2.If SWB0 is pressing, quit this loop and stop about 20th time.
        3.prepare to reset MCU. 

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
	(1) Please do not use Nu-Link Pro to try this demo code.

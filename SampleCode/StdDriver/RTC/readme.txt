/*******************************************************************
 *
 * Copyright(c) Nuvoton Technology Corp. All rights reserved.
 *
 *******************************************************************/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
   Demonstrate the usage of rtc driver, includes
    (1) Real time clock.
    (2) Interrupt handler(trigger irq when tick(1 second occur))
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Compiled to execute 
    (2) Program test procedure -
         1. User can select two option.
            a. Demo main loop pooling to count 10 seconds and display current time.
            b. Demo interrupt trigger every second for next 15 seconds.
         2.if choose 1, show timmer and count 10 seconds. 
         3.if choose 2, then trigger 15 times.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    
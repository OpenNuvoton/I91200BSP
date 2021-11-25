/*******************************************************************
 *
 * Copyright(c) Nuvoton Technology Corp. All rights reserved.
 *
 *******************************************************************/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate Power Driver sample 
     1. This code is for standby/ deep power down test, need to run without 
        ICE (Nu-Link dongle). GPA13 are used to indicate the program running
        status. 
    
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to com port to send out demo message (TX=GPA4, RX=GPA5).
    (2) Compiled to execute, and not into debug mode.
    (3) Program test procedure -
        1.Choose 'A', Enter Deep Power Down! Please Prees POR to wake up.
        2.choose 'B', Enter Deep Power Down! After 6400ms timer out wake up.
        3.Choose 'C', Enter Deep Power Down! Please Prees Wakeup PIN to wake up.
        4.Choose 'D', Enter Deep Power Down!Please Prees Wakeup PIN  or wait v to wake up.
        5.Choose 'E', Standby Power Down (SPD) wake up from RTC tick, CPU will wake up at each tick, tick unit: 1 second
        6.Choose 'F', Enter to power down mode, then wake up status after 5 second. 
        7.Choose 'G', Enter SPD Mode! Please Prees WAKEUP pin to wake up.
        8.Choose 'H', Enable Stop mode and turn off flash ROM when in stop mode. then wake up from RTC tick, 1 sec.
        9.Choose 'I', Enable Stop mode and turn off flash ROM when in stop mode. then wake up from RTC tick, 5 sec. 
        10.Choose 'J', Enter Stop Mode! Please Prees WAKEUP pin to wake up. 
        11.Choose 'K', Enable RTC clock in deep sleep mode, then wake up from RTC tick. 
        12.Choose 'L', Sleep , wake up from RTC tick. 
        13.Choose 'Q', exit.
    
        
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    
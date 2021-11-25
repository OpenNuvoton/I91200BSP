/*******************************************************************
 *
 * Copyright(c) Nuvoton Technology Corp. All rights reserved.
 *
 *******************************************************************/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate programming flow of LDORM, APROM and Data Flash
    
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Open ICP tool
        1.connect I91200 target board
        2.click setting option
        3.check in 'Data Flash' box, and setup Base address :0x1F000, then click 'OK'.
        4.in the 'Programming', check in box  for Data Flash.
        5.into the options, check in box for verify, please.
        6. click 'Refresh'   
    (2) Compiled to execute
    (3) Program test procedure -
	1.if setup fail, the log show LDROM is disabled!...
        2.if setup successful, log show Update data successfully. 

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
	(1) Programmer uses ICP tool to config LDORM, APROM and Data Flash 
            partition before run sample

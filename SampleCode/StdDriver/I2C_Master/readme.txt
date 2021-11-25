/*******************************************************************
 *
 * Copyright(c) Nuvoton Technology Corp. All rights reserved.
 *
 *******************************************************************/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
  Demonstrate the usage of I2C driver

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connect GPA6(Master) < -- > GPA6(Slave), GPA7(Master)< -- > GPA7(Slave)
    (2) Compiled to execute
    (3) Program test procedure -
	1. Before master start to send date, make sure slave is started for listening
        2. Press 'SWB0' button to start send & receive data
        3. Master demo end after the comparison of data send & receive, you can also see LED DA12 is light on

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This is a I2C_master mode demo and need to be tested with I2C_Slave demo.
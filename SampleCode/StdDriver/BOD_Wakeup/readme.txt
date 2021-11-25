/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to configure ¡§Brown-Out detection¡¨ wakeup function.
	(1)	System will wake up from SPD, STOP, or normal mode if brown-out event occurred.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to show demo message (TX=PA4, RX=PA5).
	(2)	Connect the EVB power¡¦s pins VCC and GND to a variable voltage source, like a DC power supply, and set the voltage level at 3.3V.
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	The program will first configure BOD level and interrupt function.
		2.	The program will write data to SBRAM and then enter specific mode according to different options.
		3.	User can lower the voltage source of VCC to trigger BOD.
		4.	The BOD event will trigger and wake up the system.
		5.	User can read SBRAM's data after device wakeup. 
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------

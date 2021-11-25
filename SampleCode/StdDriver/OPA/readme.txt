/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate OPA and Comparator.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Compiled to execute.
    (2) Program test procedure
	(3) Connect Pins: 
	case 0: 
			Connect PB1(A0N) to an analog source, you will meassure the following results.
			PB0's voltage > PB1  PB2(A0E) = high
			PB0's voltage < PB1  PB2(A0E) = low
	case 1:
			Connect PB0(A0P) to an analog source, you can meassure PB2(A0E) = PB0's voltage level
	case 2:
			Connect PB4(A1N)to GND and PB0(A0P) to an analog source(if 0.2V), you can meassure PB5(A1X) about 0.2V*8.
	case 3:
			Connect PB6(CNP) and PB7(C1N) to analog source respectively, you can meassure PB10(CMP1) with different trigger conditions.
	case 4:
			Connect PB0(A0P) to an analog source, you can meassure PB11(CMP2) with different trigger conditions.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
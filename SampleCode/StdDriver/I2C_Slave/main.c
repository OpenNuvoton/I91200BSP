/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/04/25 2:52p $
 * @brief    I2C Driver Sample Code
 *           This is a I2C_slave mode demo and need to be tested with I2C_Master demo.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "VI2C/VI2C.h"

// Pre-declare function.
void System_Initiate(void);

// I2C Slave API's function.
BOOL I2C_Slave_Setup(void);
void I2C_Slave_StartListen(void);
void I2C_Slave_StopListen(void);
void I2C_Slave_IRQProcess(void);

// Show message.
void Show_DemoLabel(void);
void Show_SlaveSetup(BOOL bSuccess);
void Show_DemoHwPrepare(void);
void Show_SlaveStartListen(void);
void Show_SlaveGetRxRequest(void);
void Show_SlaveRxDone(void);
void Show_SlaveGetTxRequest(void);
void Show_SlaveTxDone(void);
void Show_SlaveStopListen(void);
void Show_DemoEnd(void);

// Global variable
UINT8  g_au8DataBuf[64];

int main(void)
{
	// Initiate system clock
	System_Initiate();
	
	// Message : Demo label.
	Show_DemoLabel();
	
    // Setup VI2C0 to be a I2C slave. 
	// Message : I2C slave setup success/fail.
	if( I2C_Slave_Setup() == TRUE )
		Show_SlaveSetup(TRUE);
	else
	{
		Show_SlaveSetup(FALSE);
		while(1);
	}
	
	// Message : Hardware demo prepare(include GPIO pin connect.); When prepare done, press 'Enter' 
	Show_DemoHwPrepare();

	// Message : Slave start listen data & command from master.
	Show_SlaveStartListen();
	// Start VI2C_Slave to listen from I2C_Master.
	I2C_Slave_StartListen();
	
	// Message : Slave stop listen data & command from master.(Press 'Enter')
	Show_SlaveStopListen();	
	// Stop VI2C0 to listen master.
	I2C_Slave_StopListen();	
	
	// Message : Slave end to listen data from master.
	Show_DemoEnd();
}

// I2C0's IRQ handler.
void I2C0_IRQHandler(void)
{
	I2C_Slave_IRQProcess();
}

// VI2C provide callback API when get request of send data from master.
// User could add some action here.
// Note. If VI2C_CFG_INTEN is set, this callback function was in I2C's IRQ.
void VI2C_SendRequest(UINT8 u8VI2CNo)
{
	// Message : Slave get Tx request from master.
	// Suggest "not" to use 'printf' in the real case.
	Show_SlaveGetTxRequest();
	// Send data to master still data count(64) equal zero.
	// When data count is zero, request callback function will be happened again.
	VI2C_SetTxData(0,g_au8DataBuf,64);
}

// VI2C provide callback API when get request of receive data from master.
// User could add some action here.
// Note. If VI2C_CFG_INTEN is set, this callback function was in I2C's IRQ.
void VI2C_ReceiveRequest(UINT8 u8VI2CNo)
{
	// Message : Slave get Rx request from master.
	// Suggest "not" to use 'printf' in the real case.
	Show_SlaveGetRxRequest();
	// Receive data to master still data count(64) equal zero.
	// When data count is zero, request callback function will be happened again.
	VI2C_SetRxData(0,g_au8DataBuf,64);
}

// VI2C provide callback API when data send complete.
// User could add some action here.
// Note. If VI2C_CFG_INTEN is set, this callback function was in I2C's IRQ.
void VI2C_SendComplete(UINT8 u8VI2CNo)
{
	// Message : Slave finish Tx request.
	// Suggest "not" to use 'printf' in the real case.
	Show_SlaveTxDone();
}

// VI2C provide callback API when data receive complete.
// User could add some action here.
// Note. If VI2C_CFG_INTEN is set, this callback function was in I2C's IRQ.
void VI2C_ReceiveComplete(UINT8 u8VI2CNo)
{
	// Message : Slave finish Rx request.
	// Suggest "not" to use 'printf' in the real case.
	Show_SlaveRxDone();
}

// VI2C Slave ==========================================================================
#define I2C0_SLAVE_PINS_MSK    (SYS_GPA_MFP_PA6MFP_Msk|SYS_GPA_MFP_PA7MFP_Msk)
#define I2C0_SLAVE_PINS        (SYS_GPA_MFP_PA6MFP_I2C0_SDA|SYS_GPA_MFP_PA7MFP_I2C0_SCL)	

BOOL I2C_Slave_Setup(void)
{	
    // Open I2C0 and set clock to 100k 
    if( VI2C_Open(0, 100000, E_VI2C_CLK_DEFAULT,TRUE) )
	{
		// Init I/O multi-function ; I2C0: GPA6=SDA, GPA7= SCL
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~I2C0_SLAVE_PINS_MSK) ) | I2C0_SLAVE_PINS;
		// Config VI2C0 to be a slave.
		VI2C_Config(0,VI2C_CFG_SLAVE|VI2C_CFG_INTEN|VI2C_CFG_TODIS);
		// Config VI2C0 slave address(I91200 provide 4 slave address.)
		VI2C_ConfigSlaveAddress(0,0,0x15,0x00);
		VI2C_ConfigSlaveAddress(0,1,0x35,0x00);
		VI2C_ConfigSlaveAddress(0,2,0x55,0x00);
		VI2C_ConfigSlaveAddress(0,3,0x75,0x00);
		return TRUE;
	}
	return FALSE;
}
void I2C_Slave_StartListen(void)
{
	// Start VI2C to listen from master. 
	VI2C_Start(0);
}
void I2C_Slave_StopListen(void)
{
	// Stop VI2C to listen from master. 
	VI2C_Stop(0);
}
void I2C_Slave_IRQProcess(void)
{
	VI2C_Process(0);
}

// =====================================================================================
void System_Initiate(void)
{
	// Unlock protected registers
	SYS_UnlockReg();
	// Enable clock source 
	CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
	// Switch HCLK clock source to HIRC
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	// Enable LDO 3.3V
	CLK_EnableLDO(CLK_LDOSEL_3_3V);
	// Update System Core Clock
	SystemCoreClockUpdate();
	
	// Config gpio pin multi-functon.
	SYS->GPB_MFP = ( SYS->GPB_MFP & ~SYS_GPB_MFP_PB0MFP_Msk ) | SYS_GPB_MFP_PB0MFP_GPIO;
  // Config gpio debounce clock source & debounce time.
  GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_IRC10K, GPIO_DBCTL_DBCLKSEL_512);
	// Enable debounce pin.
	GPIO_ENABLE_DEBOUNCE( PB, BIT0);
  // Config gpio mode.
  GPIO_SetMode( PB, BIT0, GPIO_MODE_QUASI);
	// Light off DA13
	GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)|(BIT13));
	// Lock protected registers
	SYS_LockReg();	
}
void Show_DemoLabel(void)
{
	printf("\r\n+------------------------------------------------------------------------+\r\n");
	printf("|                       I2C Driver Sample Code                           |\r\n");
	printf("+------------------------------------------------------------------------+\r\n");			
}
void Show_SlaveSetup(BOOL bSuccess)
{
	printf("(1) Setup VI2C0 to be a I2C slave.\r\n");
	printf("    1. Open VI2C0 interface(frequency = 100000).\r\n");
	printf("    2. Config I2C0's gpio multi-function.\r\n");
	(bSuccess)?printf("    VI2C setup success.\r\n"):printf("    VI2C setup fail.\r\n");	
}
void Show_DemoHwPrepare(void)
{
	printf("(2) Please make sure GPA6 & GPA7 connect I2C_Master.\r\n");	
	printf("    (Press 'SWB0' button to start listen master, the LED DA13 will be light on.)\r\n");		
	// Wait user press key on EVB board.
	while( (GPIO_GET_IN_DATA(PB)&BIT0)==BIT0 );
	// Light on DA13
	GPIO_SET_OUT_DATA( PA, GPIO_GET_OUT_DATA(PA)&(~BIT13));
}
void Show_SlaveStartListen(void)
{
	printf("(3) Start to listen from I2C_Master.\r\n");
	printf("    1. Start VI2C0..\r\n");
	printf("    2. Send/receive request callback function will be happened via VI2C interface.\r\n");
}
void Show_SlaveGetRxRequest(void)
{
	printf("(4) Get receive data request from I2C_Master.\r\n");
	printf("    1. Callback function was provied by VI2C interface.\r\n");
	printf("    2. Set g_au8DataBuf for receive data from master.\r\n");
	printf("    3. After receive,VI2C0 will be trigger 'VI2C_ReceiveComplete' callback function.\r\n");	
}
void Show_SlaveRxDone(void)
{
	printf("    VI2C0 receive done.\r\n");
}
void Show_SlaveGetTxRequest(void)
{
	printf("(5) Get send data request from I2C_Master.\r\n");
	printf("    1. Callback function was provied by VI2C interface.\r\n");
	printf("    2. Set g_au8DataBuf for send data from master.\r\n");
	printf("    3. After send,VI2C0 will be trigger 'VI2C_SendComplete' callback function.\r\n");	
}
void Show_SlaveTxDone(void)
{
	printf("    VI2C0 send done.\r\n");
}
void Show_SlaveStopListen(void)
{
	printf("(6) After demo, if you want to stop listen master, press 'SWB0' again please.\r\n");
	// Wait user press key on EVB board.
	while( (GPIO_GET_IN_DATA(PB)&BIT0)==BIT0 );
}
void Show_DemoEnd(void)
{
	printf("(7) VI2C_Slave demo end.\r\n"); 
	while(1);
}

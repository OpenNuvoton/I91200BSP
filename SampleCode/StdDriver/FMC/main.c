/****************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/06/28 10:50am $
 * @brief    Show programming flow of LDORM, APROM and Data Flash 
 *
 * @note	 Programmer uses ICP tool to config LDORM, APROM and Data Flash 
 *			 partition before run sample
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "VFMC/VFMC.h"

// Define flash partition for this sample

//***********************----APROM base address (0x00000000)
//*											*
//*		  		APROM						*
//*				(124k bytes)				*
//*											*
//***********************----DATAFLASH_WRITE_ADDR (0x0001F000)
//*											*
//*				Data flash					*
//*				(4k bytes)					*
//*											*
//***********************----LDROM base address (0x00100000)
//*											*
//*				LDROM 						*
//*				(4k bytes)					*
//*											*
//*******************************************

// Global variable
UINT8 g_au8DataBuff[FMC_FLASH_PAGE_SIZE];

// Pre-declare function.
void System_Initiate(void);

// FMC API's function.
void Program_LDROM(void);
void Program_APROM(void);
void Program_DataFlash(void);
void Program_PartialData(void);

// Show message.
void Show_DemoLabel(void);

int main()
{
	// Initiate system clock.
	System_Initiate();
	// Message: Demo label.
	Show_DemoLabel();
	// Unlock write-protection for FMC operation
	SYS_UnlockReg();
	
	printf("Start to program LDROM....\r\n");
	Program_LDROM();
	printf("Program LDROM successfully.\r\n");
	
	printf("Start to program APROM....\r\n");
	Program_APROM();
	printf("Program APROM successfully.\r\n");
	
	printf("Start to program Data Flash....\r\n");
	Program_DataFlash();
	printf("Program Data Flash successfully.\r\n");
	
	printf("Start to update partital data in Data Flash....\r\n");
	Program_PartialData();
	printf("Update data successfully.\r\n");
	
	while (1);
}

// FMC =========================================================================
#define FLASH_TOTAL_SIZE			(0x20000)	//128k bytes

#define LDROM_TOTAL_SIZE			(0x1000)	//4k bytes
#define DATAFLASH_CONFIG_SIZE		(0x1000)	//config 4k bytes
#define APROM_CONFIG_SIZE			(FLASH_TOTAL_SIZE - DATAFLASH_CONFIG_SIZE)	// APROM real size (124k bytes) in this sample
#define APROM_PROGRAM_SIZE			(0x1000)	//demo program 4k bytes

#define LDROM_WRITE_ADDR			(FMC_LDROM_BASE)
#define DATAFLASH_WRITE_ADDR		(0x00000000 /*APROM base address*/ + APROM_CONFIG_SIZE)
#define APROM_WRITE_ADDR			(DATAFLASH_WRITE_ADDR - APROM_PROGRAM_SIZE)	

void Program_LDROM(void)
{
	UINT32 u32DataSize, u32LDROMAddr = LDROM_WRITE_ADDR;
	UINT16 u16Cnt, u16BuffSize;
	
	u16BuffSize = sizeof(g_au8DataBuff);
	u32DataSize = 0x400;
	
	// 1. Enable Virtual FMC IP.
	VFMC_Open();
	
	// 2. Check LDROM enabled or not
	if ( !VFMC_CheckUserConfig(VFMC_USER_CONFIG_LDROMEN))
	{
		printf("LDROM is disabled! Please uses ICP tool to config LDORM, APROM and Data Flash.\r\n");
		while(1);
	}
	
	// 3. Config LDROM updated option
	VFMC_Config(VFMC_CFG_LDUEN);

	while(u32DataSize)
	{
		// 4. Read data to buffer by different application. Maybe the data is from external flash or communcation path
		for (u16Cnt = 0; u16Cnt < u16BuffSize; u16Cnt++)
		{
			g_au8DataBuff[u16Cnt] = u16Cnt&0xff;
		}
		
		// 5. Erase LDROM 
		VFMC_Erase(u32LDROMAddr);	
		
		// 6. Write data to LDROM
		VFMC_WriteAlign(u32LDROMAddr, g_au8DataBuff, u16BuffSize);
		
		// 7. Verify data (optional)
		memset(g_au8DataBuff, 0, u16BuffSize);
		VFMC_ReadAlign(u32LDROMAddr, g_au8DataBuff, u16BuffSize);
			
		for (u16Cnt = 0; u16Cnt < u16BuffSize; u16Cnt++)
		{
			if (g_au8DataBuff[u16Cnt] != (u16Cnt&0xff))
			{
				printf("Verfiy fail!\r\n");	
				while(1);
			}
		}
		
		u32DataSize-=u16BuffSize;
		u32LDROMAddr+=u16BuffSize;
	}
	
	// 8. Disable Virtual FMC IP.
	VFMC_Close();
}

void Program_APROM(void)
{
	UINT32 u32DataSize, u32APROMAddr = APROM_WRITE_ADDR;
	UINT16 u16Cnt, u16BuffSize;
	
	u16BuffSize = sizeof(g_au8DataBuff);
	u32DataSize = 0x400;
	
	// 1. Enable Virtual FMC IP.
	VFMC_Open();

	// 2. Config APROM updated option
	VFMC_Config(VFMC_CFG_APUWEN);

	while(u32DataSize)
	{
		// 3. Read data to buffer by different application. Maybe the data is from external flash or communcation path
		for (u16Cnt = 0; u16Cnt < u16BuffSize; u16Cnt++)
		{
			g_au8DataBuff[u16Cnt] = u16Cnt&0xff;
		}
		
		// 4. Erase APROM 
		VFMC_Erase(u32APROMAddr);	
		
		// 5. Write data to APROM
		VFMC_WriteAlign(u32APROMAddr, g_au8DataBuff, u16BuffSize);
		
		// 6. Verify data (optional)
		memset(g_au8DataBuff, 0, u16BuffSize);
		VFMC_ReadAlign(u32APROMAddr, g_au8DataBuff, u16BuffSize);
		
		for (u16Cnt = 0; u16Cnt < u16BuffSize; u16Cnt++)
		{
			if (g_au8DataBuff[u16Cnt] != (u16Cnt&0xff))
			{
				printf("Verfiy fail!\r\n");	
				while(1);
			}
		}
		
		u32DataSize-=u16BuffSize;
		u32APROMAddr+=u16BuffSize;
	}
	
	// 7. Disable Virtual FMC IP.
	VFMC_Close();
}

void Program_DataFlash(void)
{
	UINT32 u32DataSize, u32DataflashAddr = DATAFLASH_WRITE_ADDR;
	UINT16 u16Cnt, u16BuffSize;
	
	u16BuffSize = sizeof(g_au8DataBuff);
	u32DataSize = 0x400;
	
	// 1. Enable Virtual FMC IP.
	VFMC_Open();
	
	// 2. Check data flash enabled or not
	if ( !VFMC_CheckUserConfig(VFMC_USER_CONFIG_DFEN))
	{
		printf("Data Flash is disabled! Please uses ICP tool to config LDORM, APROM and Data Flash.\r\n");
		while(1);
	}
	
	while(u32DataSize)
	{
		// 3. Read data to buffer by different application. Maybe the data is from external flash or communcation path
		for (u16Cnt = 0; u16Cnt < u16BuffSize; u16Cnt++)
		{
			g_au8DataBuff[u16Cnt] = u16Cnt&0xff;
		}
		
		// 4. Erase data flash 
		VFMC_Erase(u32DataflashAddr);	
		
		// 5. Write data to data flash
		VFMC_WriteAlign(u32DataflashAddr, g_au8DataBuff, u16BuffSize);
		
		// 6. Verify data (optional)
		memset(g_au8DataBuff, 0, u16BuffSize);
		VFMC_ReadAlign(u32DataflashAddr, g_au8DataBuff, u16BuffSize);
			
		for (u16Cnt = 0; u16Cnt < u16BuffSize; u16Cnt++)
		{
			if (g_au8DataBuff[u16Cnt] != (u16Cnt&0xff))
			{
				printf("Verfiy fail!\r\n");	
				while(1);
			}
		}
		
		u32DataSize-=u16BuffSize;
		u32DataflashAddr+=u16BuffSize;
	}
	
	// 7. Disable Virtual FMC IP.
	VFMC_Close();
}

void Program_PartialData(void)
{
	UINT32 u32AlignedAddr, u32UpdateAddr = DATAFLASH_WRITE_ADDR + 0x123;
	UINT16 u16Cnt, u16Offset;
	UINT8 u8UpdateSize = 7;
	
	// 1. Enable Virtual FMC IP.
	VFMC_Open();
		
	// 2. Check data flash enabled or not
	if ( !VFMC_CheckUserConfig(VFMC_USER_CONFIG_DFEN))
	{
		printf("Data Flash is disabled! Please uses ICP tool to config LDORM, APROM and Data Flash.\r\n");
		while(1);
	}
	
	// 3. Read data from u32UpdateAddr aligned at 512
	u32AlignedAddr = u32UpdateAddr&VFMC_ERASE_ALIGN_MASK;
	u16Offset = u32UpdateAddr&~VFMC_ERASE_ALIGN_MASK;
	VFMC_ReadSector(u32AlignedAddr, g_au8DataBuff);
	
	// 4. Write data to buffer
	for (u16Cnt = 0; u16Cnt < u8UpdateSize; u16Cnt++)
	{
		g_au8DataBuff[u16Cnt+u16Offset] = 0xA5;
	}
		
	// 5. Erase aligned u32AlignedAddr
	VFMC_Erase(u32AlignedAddr);	
		
	// 6. Write data to data flash
	VFMC_WriteSector(u32AlignedAddr, g_au8DataBuff);
		
	// 7. Verify data (optional)
	memset(g_au8DataBuff, 0, u8UpdateSize);
	VFMC_Read(u32UpdateAddr, g_au8DataBuff, u8UpdateSize);
			
	for (u16Cnt = 0; u16Cnt < u8UpdateSize; u16Cnt++)
	{
		if (g_au8DataBuff[u16Cnt] != 0xA5)
		{
			printf("Verfiy fail!\r\n");	
			while(1);
		}
	}
		
	// 8. Disable Virtual FMC IP.
	VFMC_Close();
}

// =====================================================================================
void System_Initiate(void)
{
	// Unlock protected registers
	SYS_UnlockReg();
	// Enable clock source 
	CLK_EnableXtalRC(CLK_PWRCTL_HIRC_EN|CLK_PWRCTL_LIRC_EN|CLK_PWRCTL_LXT_EN|CLK_PWRCTL_HXT_EN);
	// Switch HCLK clock source to HIRC
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	// Enable LDO 3.3V
	CLK_EnableLDO(CLK_LDOSEL_3_3V);
	// Update System Core Clock
	SystemCoreClockUpdate();
	// Lock protected registers
	SYS_LockReg();	
}

// =====================================================================================
void Show_DemoLabel(void)
{
	printf("+------------------------------------------------------------------------+\r\n");
	printf("|                      FMC Driver Sample Code                            |\r\n");
	printf("+------------------------------------------------------------------------+\r\n");
}
/*** (C) COPYRIGHT Nuvoton Technology Corp. ***/

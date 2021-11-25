/*******************************************************************************
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "VFMC.h"

// u32Frequency : Bus clock(Unit:Hz)
// eClockSource : Clock source(eVTIMER_Clock)
// bReset       : Reset hardware module(TRUE/FALSE)
void VFMC_Open(void)
{
	//uint8_t u8Lock = SYS_Unlock();
	
	FMC_ENABLE_ISP();
	
	//Flash Access Wait State Configuration
	//0x02: One wait state. HCLK <= 50MHz.
	//FMC->ISPCTL = (FMC->ISPCTL&~FMC_ISPCTL_WAITCFG_Msk )| (0x02 << FMC_ISPCTL_WAITCFG_Pos);
	
	//SYS_Lock(u8Lock);
}

void VFMC_Close(void)
{
	//uint8_t u8Lock = SYS_Unlock();
	
	FMC_DISABLE_ISP();
	
	//SYS_Lock(u8Lock);
}

// u32Configuration : Configuration Selection(Bitwise,ex: VTIMER_CFG_ONESHOT|VTIMER_CFG_INTEN etc.)
void VFMC_Config(UINT32 u32Configuration)
{
	UINT32 u32Tmp = 0;
	
	if( (u32Tmp = u32Configuration&(VFMC_CFG_LDUEN|VFMC_CFG_CFGUEN|VFMC_CFG_APUWEN)) > 0 )
	{
		FMC->ISPCTL |= u32Tmp;
		if ((u32Configuration & VFMC_CFG_LDUDIS) == VFMC_CFG_LDUDIS)
			FMC_DISABLE_LD_UPDATE();
		if ((u32Configuration & VFMC_CFG_CFGUDIS) == VFMC_CFG_CFGUDIS)
			FMC_DISABLE_CFG_UPDATE();
		if ((u32Configuration & VFMC_CFG_APUWDIS) == VFMC_CFG_APUWDIS)
			FMC_DISABLE_APROM_WRITABLE();
	}
}

void VFMC_Write(UINT32 u32DstAddr, PUINT8 pu8SrcBuff, UINT32 u32DataLen)
{
	UINT32 u32Addr, u32ProcessByte, u32WriteCount, u32WriteData = 0xffffffff;
	PUINT8 pu8Temp;
	
	//uint8_t u8Lock = SYS_Unlock();
	
	pu8Temp = (UINT8 *)&u32WriteData;
	u32WriteCount = u32DataLen;
	u32Addr = u32DstAddr&0x3;
	
	if (u32Addr)// Check u32DstAddr aligned with 4-byte
	{
		u32ProcessByte = 4 - u32Addr;
		if ( u32ProcessByte > u32DataLen )
			u32ProcessByte = u32DataLen;
		pu8Temp[u32Addr] = *pu8SrcBuff++;
		if ( u32ProcessByte >= 2 )
			pu8Temp[u32Addr+1] = *pu8SrcBuff++;
		if ( u32ProcessByte == 3 )
			pu8Temp[u32Addr+2] = *pu8SrcBuff++;
		FMC_Write(u32DstAddr&0xFFFFFFFC, u32WriteData);
		u32WriteCount -=  u32ProcessByte;
		u32DstAddr+=u32ProcessByte;
	}
	
	//SYS_Lock(u8Lock);
	
	VFMC_WriteAlign(u32DstAddr, pu8SrcBuff, u32WriteCount);	
}

// u32AlignedAddr	: Starting address.
// pu8SrcBuff			: Data source buffer.
// u32DataLen			: Length of data to write to memory.
void VFMC_WriteAlign(UINT32 u32AlignedAddr, PUINT8 pu8SrcBuff, UINT32 u32DataLen)
{
	UINT32 u32WriteCount, u32WriteData;
	PUINT8 pu8Temp;

	//uint8_t u8Lock = SYS_Unlock();
		
	pu8Temp = (UINT8 *)&u32WriteData;
	u32WriteCount = u32DataLen;

	while(u32WriteCount >= 4)
	{
		pu8Temp[0] = *pu8SrcBuff++;
		pu8Temp[1] = *pu8SrcBuff++;
		pu8Temp[2] = *pu8SrcBuff++;
		pu8Temp[3] = *pu8SrcBuff++;
		FMC_Write(u32AlignedAddr, u32WriteData);
		u32AlignedAddr+=4;
		u32WriteCount-=4;
	}
	
	if (u32WriteCount)
	{
		u32WriteData = 0xffffffff;
		pu8Temp[0] = *pu8SrcBuff++;
		if ( u32WriteCount >= 2 )
			pu8Temp[1] = *pu8SrcBuff++;
		if ( u32WriteCount == 3 )
			pu8Temp[2] = *pu8SrcBuff++;
		FMC_Write(u32AlignedAddr, u32WriteData);
	}

	//SYS_Lock(u8Lock);
}

// u32AlignedAddr	: Starting address.
// pu8SrcBuff			: Data source buffer.
void VFMC_WriteSector(UINT32 u32SectorAddr, PUINT8 pu8SrcBuff)
{
	UINT32 u32WriteCount, u32WriteData;
	PUINT8 pu8Temp;

	//uint8_t u8Lock = SYS_Unlock();
		
	pu8Temp = (UINT8 *)&u32WriteData;
	u32WriteCount = FMC_FLASH_PAGE_SIZE;

	do 
	{
		pu8Temp[0] = *pu8SrcBuff++;
		pu8Temp[1] = *pu8SrcBuff++;
		pu8Temp[2] = *pu8SrcBuff++;
		pu8Temp[3] = *pu8SrcBuff++;
		FMC_Write(u32SectorAddr, u32WriteData);
		u32SectorAddr+=4;
		u32WriteCount-=4;
	}while(u32WriteCount);
	
	//SYS_Lock(u8Lock);
}

void VFMC_Read(UINT32 u32SrcAddr, PUINT8 pu8DstBuff, UINT32 u32DataLen)
{
	UINT32 u32Addr, u32ProcessByte, u32ReadCount, u32ReadData;
	PUINT8 pu8Temp;
	
	//uint8_t u8Lock = SYS_Unlock();
	
	pu8Temp = (UINT8 *)&u32ReadData;
	u32ReadCount = u32DataLen;
	u32Addr = u32SrcAddr&0x3;
	
	if (u32Addr)
	{
		u32ReadData = FMC_Read(u32SrcAddr&0xFFFFFFFC);
		
		u32ProcessByte = 4 - u32Addr;
		if ( u32ProcessByte > u32DataLen )
			u32ProcessByte = u32DataLen;
		*pu8DstBuff++ = pu8Temp[u32Addr];
		if ( u32ProcessByte >= 2 )
			*pu8DstBuff++ = pu8Temp[u32Addr+1];
		if ( u32ProcessByte == 3 )
			*pu8DstBuff++ = pu8Temp[u32Addr+2];
		u32ReadCount -=  u32ProcessByte;
		u32SrcAddr+=u32ProcessByte;
	}
	
	//SYS_Lock(u8Lock);
	
	VFMC_ReadAlign(u32SrcAddr, pu8DstBuff, u32ReadCount);
}

void VFMC_ReadAlign(UINT32 u32AlignedAddr, PUINT8 pu8DstBuff, UINT32 u32DataLen)
{
	UINT32 u32ReadCount, u32ReadData;
	PUINT8 pu8Temp;

	//uint8_t u8Lock = SYS_Unlock();
	
	pu8Temp = (UINT8 *)&u32ReadData;
	u32ReadCount = u32DataLen;
	
	while(u32ReadCount >= 4)
	{
		u32ReadData = FMC_Read(u32AlignedAddr);
		*pu8DstBuff++ = pu8Temp[0];
		*pu8DstBuff++ = pu8Temp[1];
		*pu8DstBuff++ = pu8Temp[2];
		*pu8DstBuff++ = pu8Temp[3];
		u32AlignedAddr+=4;
		u32ReadCount-=4;
	}
	
	if (u32ReadCount)
	{
		u32ReadData = FMC_Read(u32AlignedAddr);
		*pu8DstBuff++ = pu8Temp[0];
		if ( u32ReadCount >= 2 )
			*pu8DstBuff++ = pu8Temp[1];
		if ( u32ReadCount == 3 )
			*pu8DstBuff++ = pu8Temp[2];
	}
	
	//SYS_Lock(u8Lock);
}

void VFMC_ReadSector(UINT8 u32SectorAddr, PUINT8 pu8DstBuff)
{
	UINT32 u32ReadCount, u32ReadData;
	PUINT8 pu8Temp;

	//uint8_t u8Lock = SYS_Unlock();
	
	pu8Temp = (UINT8 *)&u32ReadData;
	u32ReadCount = FMC_FLASH_PAGE_SIZE;
	
	do{
		u32ReadData = FMC_Read(u32SectorAddr);
		*pu8DstBuff++ = pu8Temp[0];
		*pu8DstBuff++ = pu8Temp[1];
		*pu8DstBuff++ = pu8Temp[2];
		*pu8DstBuff++ = pu8Temp[3];
		u32SectorAddr+=4;
		u32ReadCount-=4;
	}while(u32ReadCount);
	
	//SYS_Lock(u8Lock);
}

void VFMC_Erase(UINT32 u32Address)
{
	if ((u32Address&~VFMC_ERASE_ALIGN_MASK) == 0)
		FMC_Erase(u32Address);
}

BOOL VFMC_CheckUserConfig(UINT32 u32UserConfig)
{
	UINT32 au32Config[VFMC_LENGTH_USERCONFIG];
	
	FMC_ReadConfig(au32Config, VFMC_LENGTH_USERCONFIG);
	
	if ((au32Config[0]&u32UserConfig) == 0)
	{
		if(u32UserConfig == VFMC_USER_CONFIG_LDROMEN)
			return FALSE;
		else
			return TRUE;
	}
	else
	{	
		if(u32UserConfig == VFMC_USER_CONFIG_LDROMEN)
			return TRUE;
		else
			return FALSE;
	}
			
	/*if( u32UserConfig == VFMC_USER_CONFIG_LDROMEN )
	{
		if ((au32Config[0]&u32UserConfig) == 0)
			return FALSE;
		else
			return TRUE;
	}
	else
	{
		if ((au32Config[0]&u32UserConfig) == 0)
			return TRUE;
		else
			return FALSE;
	}*/
	
	/*switch(u32UserConfig)
	{
		case VFMC_USER_CONFIG_DFEN:
			if ((au32Config[0]&u32UserConfig) == 0)
				return TRUE;
			else
				return FALSE;
		case VFMC_USER_CONFIG_LOCK:
			if ((au32Config[0]&u32UserConfig) == 0)
				return TRUE;
			else
				return FALSE;
		case VFMC_USER_CONFIG_LDBOOT:
			if ((au32Config[0]&u32UserConfig) == 0)
				return TRUE;
			else
				return FALSE;
		case VFMC_USER_CONFIG_CBODEN:
			if ((au32Config[0]&u32UserConfig) == 0)
				return TRUE;
			else
				return FALSE;
		case VFMC_USER_CONFIG_LDROMEN:
			if ((au32Config[0]&u32UserConfig) == 0)
				return FALSE;
			else
				return TRUE;
		default:
			return FALSE;
	}*/
}

void VFMC_Reboot(UINT8 u8BootOption)
{
	FMC_SetBootSource((UINT32)u8BootOption);
	SYS_ResetCPU();
}	

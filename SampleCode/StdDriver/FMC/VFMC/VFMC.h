
#ifndef _VFMC_H_
#define _VFMC_H_
// ----------------------------------------------------------------------------------------------------------
// 	- FMC call flow:
//		main loop: VTIMER_Open()->VTIMER_Config()
//          +----->VTIMER_Start()->
//          +------VTIMER_Stop()
//
// -----------------------------------------------------------------------------------------------------------

#include "VCommon.h"

// Configuration Selection (Bitwise) ==================
#define VFMC_CFG_LDUEN          		(FMC_ISPCTL_LDUEN_Msk)         	// LDROM Update Enable
#define VFMC_CFG_LDUDIS          		(BIT31|FMC_ISPCTL_LDUEN_Msk)   	// LDROM Update Disable
#define VFMC_CFG_CFGUEN         		(FMC_ISPCTL_CFGUEN_Msk)         // CONFIG Update Enable
#define VFMC_CFG_CFGUDIS          	(BIT30|FMC_ISPCTL_CFGUEN_Msk)   // CONFIG Update Disable
#define VFMC_CFG_APUWEN         		(FMC_ISPCTL_APUWEN_Msk)  				// APROM Write Enable (write itself)
#define VFMC_CFG_APUWDIS         		(BIT29|FMC_ISPCTL_APUWEN_Msk) 	// APROM Write Disable 

// Erasable sector size is 512 Byte, sector address is aligned at 512 boundary
#define VFMC_ERASE_ALIGN_MASK				(0xFFFFFE00)

// Boot selection =====================================
#define VFMC_BOOT_FROM_APROM				(IS_BOOT_FROM_APROM)
#define VFMC_BOOT_FROM_LDROM				(IS_BOOT_FROM_LDROM)

// User configuration length
#define VFMC_LENGTH_USERCONFIG			(2)															// Length of User Configuration 

// User configuration mask
#define VFMC_USER_CONFIG_DFEN				(BIT0)													// Data Flash Enable
#define VFMC_USER_CONFIG_LOCK				(BIT1)													// Security Lock
#define VFMC_USER_CONFIG_LDROMEN 		(BIT2)													// LDROM Enable
#define VFMC_USER_CONFIG_LDBOOT  		(BIT7)													// Configuration Boot Selection
#define VFMC_USER_CONFIG_CBODEN 		(BIT23)													// Brown Out Detection

// Virtual FMC Common Function ========================
void VFMC_Open(void);

void VFMC_Close(void);

void VFMC_Config(UINT32 u32Configuration);

// Virtual PDMA Special Function =======================
BOOL VFMC_CheckUserConfig(UINT32 u32Configuration);

void VFMC_Write(UINT32 u32DstAddr, PUINT8 pu8SrcBuff, UINT32 u32DataLen);

void VFMC_WriteAlign(UINT32 u32AlignedAddr, PUINT8 pu8SrcBuff, UINT32 u32DataLen);

void VFMC_WriteSector(UINT32 u32SectorAddr, PUINT8 pu8SrcBuff);

void VFMC_Read(UINT32 u32SrcAddr, PUINT8 pu8DstBuff, UINT32 u32DataLen);

void VFMC_ReadAlign(UINT32 u32AlignedAddr, PUINT8 pu8DstBuff, UINT32 u32DataLen);

void VFMC_ReadSector(UINT8 u32SectorAddr, PUINT8 pu8DstBuff);

void VFMC_Erase(UINT32 u32Address);

void VFMC_Reboot(UINT8 u8BootOption);
#endif

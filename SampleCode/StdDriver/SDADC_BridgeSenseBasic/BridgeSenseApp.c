#include <stdio.h>
#include <string.h>
#include "Platform.h"

void SARADC_SetMode( uint32_t mode )
{
	SARADC->CTL = (SARADC->CTL & ~SARADC_CTL_OPMODE_Msk)| (mode<< SARADC_CTL_OPMODE_Pos);
}

void SARADC_SetInputType( uint32_t inputType)
{
	SARADC->ACTL=(SARADC->ACTL & ~SARADC_ACTL_SAR_SE_MODE_Msk)|inputType;
}

#define SARADC_ACTL_SAR_VCMsel_Pos       (16)                                              /*!< SARADC_T::ACTL: SAR_VCMsel Position    */
#define SARADC_ACTL_SAR_VCMsel_Msk       (0x1ul << SARADC_ACTL_SAR_VCMsel_Pos)             /*!< SARADC_T::ACTL: SAR_VCMsel Mask        */
#define SARADC_CHEN_SAR_Vref_sel_Pos     (16)                                              /*!< SARADC_T::CHEN: SAR_Vref_sel Position  */
#define SARADC_CHEN_SAR_Vref_sel_Msk     (0x1ul << SARADC_CHEN_SAR_Vref_sel_Pos)           /*!< SARADC_T::CHEN: SAR_Vref_sel Mask      */
#define SARADC_ACTL_SAR_cur_Pos          (17)                                              /*!< SARADC_T::ACTL: SAR_cur Position       */
#define SARADC_ACTL_SAR_cur_Msk          (0x1ul << SARADC_ACTL_SAR_cur_Pos)                /*!< SARADC_T::ACTL: SAR_cur Mask           */

void SARADC_SelectSingleEndedChannels( uint32_t channelMask)
{
	if(channelMask==0x8000) SARADC->CHEN |= SARADC_CHEN_SAR_Vref_sel_Msk;  // open band gap
	SARADC->CHEN = (SARADC->CHEN& ~SARADC_CHEN_CHEN_Msk)|  channelMask;
	
}


int32_t SARADC_Open1(uint32_t samplingRate, uint32_t opMode, uint32_t inputType, uint16_t channelMask)
{
	/* reset module */
	//SYS->IPRSTC2.SARADC_RST = 1;
	CLK->APBCLK0|= CLK_APBCLK0_SARADCKEN_Msk;
	SYS->IPRST1|= SYS_IPRST1_SARADCRST_Msk;
	
		
	/* Set module clock for proper sampling rate */
	if (SARADC_SetSampleRate(samplingRate)==0)
	{
		return -1;
	}
	
	/* enable clock to the module */
	//SYSCLK->APBCLK.SARADC_EN = 1;
	CLK->APBCLK0|= CLK_APBCLK0_SARADCKEN_Msk;
	//SYS->IPRSTC2.SARADC_RST = 0;
	SYS->IPRST1 &= ~SYS_IPRST1_SARADCRST_Msk;
	
	SARADC_SetMode(opMode);
	SARADC_SetInputType(inputType);
	SARADC_SelectSingleEndedChannels(channelMask);	
	SARADC->CTL |= SARADC_CTL_ADCEN_Msk;
	return 1;
	
}

float GetVdd(void)
{
	uint32_t uiTmp,u32SARADCDATA;
	float Vdd;
	
		SARADC_Open1(10000,0,1,BIT15/*BIT13*/); 	 // BIT13 - main LDO		
	  //CLK->CLKDIV0=0x08001010;
		SARADC->ACTL=(SARADC->ACTL & ~(SARADC_ACTL_SAR_VCMsel_Msk|SARADC_ACTL_SAR_cur_Msk|(1<<18)));

		SARADC_SelectSingleEndedChannels(1<<15);
		uiTmp=1000;
	  while(uiTmp>1){uiTmp--;__NOP();};
		
		SARADC->CTL |= SARADC_CTL_SWTRG_Msk;
		do{
					for (uiTmp = 0; (uiTmp < 0xFFFFF) && ((SARADC->STATUS& SARADC_STATUS_ADEF_Msk)==0); ++uiTmp);
		}while((SARADC->STATUS& SARADC_STATUS_ADEF_Msk)==0);
				
		SARADC->STATUS|= SARADC_STATUS_ADEF_Msk;
		u32SARADCDATA=SARADC->RESERVE0[3];
		u32SARADCDATA &=0xffff;

		SYS->IPRST1 |= SYS_IPRST1_SARADCRST_Msk;	
		SYS->IPRST1 &= ~SYS_IPRST1_SARADCRST_Msk;	
		Vdd= (float) u32SARADCDATA;
		if(Vdd<1)  	{Vdd=0.0;	return Vdd;}
		Vdd=(1.2* 4096.0)/Vdd;
		return Vdd;
}

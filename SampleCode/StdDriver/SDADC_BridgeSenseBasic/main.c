/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/01/23 2:52p $
 * @brief    Demo Aanlog or Digital Mic input to SDADC
 *
 * @note
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "BridgeSenseApp.h"
//#define SAMPLE_RATE			(16000)
extern int IsDebugFifoEmpty(void);
#define DC_Value 0
#define BS_Read_Value0 1
#define BS_Read_Value1 2
#define BS_Read_Value2 3
#define BS_Read_Value3 4
#define TrimPointnumber 4 // 128
#define BS_PDMA_LENGTH 0x16
#define BS_SAMPLE_NUMBER 32
//#define BS_SAMPLE_NUMBER 8
 
#define SAMPLE_RATE			(1000)
//#define TARGET_VOL			(eVOLCTRL_6_DB)
#define SDADC_INT_SAMPLES	(4)			//There are 4 words data in FIFO buffer, SDADC threshold will generate a interrupt

#define BSPDMACHANNEL 0
#define  ABS(a) (((a) < 0) ? -(a) : (a))

volatile uint32_t g_u32SampleCnt, u32SampleRate;
//volatile uint32_t g_u32CurrentVol = eVOLCTRL_NEG30_DB;
int8_t Optionitem;
int32_t dataarraycnt,PDMA_INT_CNT;
uint32_t u32Count;
int32_t u32ADCdata[2*BS_PDMA_LENGTH*4];
int32_t DataOutput[2*BS_SAMPLE_NUMBER],DataOut;
uint32_t Moveavg,fMoveavg ;
static int32_t BsDataOutput,BsDataN,BsDataP,ENOB_GetData;

#define SDADC_SET_CHOPPER_FREQ(x) 		{ SDADC->SDCHOP= (SDADC->SDCHOP & ~(3<<23))|(x<<23);}
#define SDADC_SET_CHOPPER_DIETHER(x) 	{SDADC->SDCHOP= (SDADC->SDCHOP & ~(1<<27))|(x<<27); }
#define SDADC_ENABLE_CHOPPERS(enable) 	{SDADC->SDCHOP= (SDADC->SDCHOP & ~((1<<29)|(1<<3)))|(enable<<29)|(enable<<3); } 			
                                   // {SDADC->SDCHOP.SDADC_CHOPEN = enable; SDADC->SDCHOP.SDADC_VREF = enable; }
#define SDADC_CHOPPER_FIXED_FREQ(enable)  	{SDADC->SDCHOP= (SDADC->SDCHOP & ~(1<<26))|(enable<<26); }//{SDADC->SDCHOP.SDADC_CHOPFIX = enable; }
#define BSADC_IA_ENABLE_CHOPPER(enable)		 {SDADC->BSINISTRAMP= (SDADC->BSINISTRAMP & ~(SDADC_BSINISTRAMP_CHOPEN_Msk))|(enable<<SDADC_BSINISTRAMP_CHOPEN_Pos);}// }SDADC_BSINISTRAMP_CHOPEN_Msk//{SDADC->INSTRU.BSIA_CHOPEN = enable;}
#define BSADC_BG_ENABLE_LFCHOPPER(enable)		{SDADC->BSBGLDO= (SDADC->BSBGLDO & ~(SDADC_BSBGLDO_BGCHOPLFEN_Msk))|(enable<<SDADC_BSBGLDO_BGCHOPLFEN_Pos);}//{SDADC->BANDGP.BSB_CHOPLFEN = enable;}
#define BSADC_BG_ENABLE_CHOPPER(enable)			{SDADC->BSBGLDO= (SDADC->BSBGLDO & ~(SDADC_BSBGLDO_BGCHOPEN_Msk))|(enable<<SDADC_BSBGLDO_BGCHOPEN_Pos);}//{SDADC->BANDGP.BSB_CHOPEN = enable;}
//#define BSADC_LDO_SHORT_DETECTED() 			(SDADC->BANDGP.BSLDO_SHRTDET)
 #define BSADC_SYSCHOP_TOGGLE()				{SDADC->BSINISTRAMP= (SDADC->BSINISTRAMP ^SDADC_BSINISTRAMP_SYSCHOPPH_Msk; )//	{SDADC->INSTRU.BSIA_SYSCHOPPH ^= 1;}

float BSLDOVOL[16]={1.5,	1.8,	2.1,	2.4,	2.7,	3.0,	3.3,	3.6,	3.9,	4.2,	4.5};
int BSLDOLEVEL;
	
/* Averaging Buffer Structure */
typedef struct
{
	 int 	BS_AVE_DC_Valuue;
	 int	BS_Read_Cnt_0;
	 int	BS_Read_Value_0;
	 int	BS_Read_Cnt_1;
	 int	BS_Read_Value_1;
	 int	BS_Read_Cnt_2;
	 int	BS_Read_Value_2;	
	 int	BS_Read_Cnt_3;
	 int	BS_Read_Value_3;		
}AVG_BUFF_T;

static AVG_BUFF_T BS_para;
static uint32_t Read_DSADCOnly;
extern float GetVdd(void);
void SDADC_IRQHandler(void)
{
	uint8_t u8i = SDADC_INT_SAMPLES;
	uint32_t u32i;
	
		if (SDADC_GetIntFlag(SDADC_FIFO_INT))
		{
		do
			{
				while(DPWM_IS_FIFOFULL(DPWM)); // check DPWM FIFO whether full
					u32i=SDADC->DAT;//SDADC_GET_FIFODATA(SDADC);
				u32ADCdata[u32Count++] = u32i;
			}while(--u8i>0);
		}
		if(u32Count>=dataarraycnt)
		{
			if(fMoveavg==1)
			{
				u32Count=0;
				Moveavg=1;
				return ;
			}
			u32i=SDADC->DAT;
			SDADC_STOP_CONV(SDADC);
		}
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:     PDMA_IRQHandler     		                                                               */
/*                                                                                                         */
/* Parameter:        																					   */	
/*				 None 												                                       */
/* Returns:                                                                                                */
/*               None                                                                                      */
/* Description:                                                                                            */
/*               The function is used to  PDMA ISR                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void PDMA_IRQHandler(void)
{
		int u32i,u32j,u32k,u32z,u32x;
		SDADC_STOP_CONV(SDADC);		
		if(PDMAC->GINTSTS & (1<<BSPDMACHANNEL))										 /* Check INT Channel */
		{			 							
			if((PDMA0->INTSTS & PDMA_INTSTS_ABTIF_Msk)==PDMA_INTSTS_ABTIF_Msk)  /* Check Target Abort INT and clear */
			{
				PDMA0->INTSTS |= PDMA_INTSTS_ABTIF_Msk;
				PDMA_Close();
				SDADC_START_CONV(SDADC);	
				printf("PDMA and SDADC abort !!, system halt..\r\n");
				while(1);
			}			 							
			else if((PDMA0->INTSTS & PDMA_INTSTS_TXIF_Msk)==PDMA_INTSTS_TXIF_Msk)
			{
				PDMA0->INTSTS |= PDMA_INTSTS_TXIF_Msk;
				if(Read_DSADCOnly!=1)
				{
					u32i=u32j=u32k=u32z=0;
					u32i=BS_PDMA_LENGTH-6;
					while(u32i>0) { u32j+=(u32ADCdata[u32i+3]);u32i--;}
					//while(u32i>0) {u32j+=u32ADCdata[u32i+3];u32i--;}
					u32j/=(BS_PDMA_LENGTH-6);
					// skip difference face and difference to big.
//					u32x=512;
//					while(1)
//					{
//					u32i=BS_PDMA_LENGTH-6;
//					while(u32i>0) {
//						u32i--; 
//						if((u32ADCdata[u32i+4]^u32j)&BIT31  ) 	continue;
//						if((ABS(u32ADCdata[u32i+4]-u32j)) >u32x) 	continue;
//						u32k+=u32ADCdata[u32i+4];
//						u32z++;
//						}
//						if(u32z=!0)  break;
//							u32x=u32x<<1;
//						
//					}
//					u32k/=u32z;

					
				if((SDADC->BSINISTRAMP &SDADC_BSINISTRAMP_SYSCHOPPH_Msk)==SDADC_BSINISTRAMP_SYSCHOPPH_Msk)
				{
					//BsDataN=(u32ADCdata[BS_PDMA_LENGTH-2]+u32ADCdata[BS_PDMA_LENGTH-1])/2;
					BsDataN=u32j;
					BsDataOutput=(BsDataN-BsDataP)/4;
					DataOutput[DataOut++]=BsDataOutput;
					DataOut&=0x7f;
					BsDataP=BsDataN=0;
					ENOB_GetData=1;
				}
				else
				{
					//BsDataP=(u32ADCdata[BS_PDMA_LENGTH-2]+u32ADCdata[BS_PDMA_LENGTH-1])/2;
					BsDataP=u32j;
				}
				SDADC->BSINISTRAMP^=SDADC_BSINISTRAMP_SYSCHOPPH_Msk;
				}
				else
				{
					ENOB_GetData=1;
				}
				PDMA_INT_CNT--;
				if(PDMA_INT_CNT>0){ SDADC_START_CONV(SDADC);		 PDMA_Trigger(0);	}	else  DataOut=0;
				
		}
	}
}
void Delay(int nCount)
{
	while(nCount>0)  nCount--;
}

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
    // Lock protected registers
    SYS_LockReg();	
}

void ADC_Init()
{
	// Enable SDADC module clock. 
//	int ucCoefNum;
	CLK_EnableModuleClock(SDADC_MODULE);
	SYS_ResetModule(SDADC_RST);
	u32Count=0;

		//g_u32CurrentVol = TARGET_VOL;
		// Enable analog block clock. 
		CLK_EnableModuleClock(ANA_MODULE);
		SYS_ResetModule(ANA_RST);
		Delay(10000);
		// set the VMID Reference for analog block
//		CLK_ENABLE_VMID(CLK, CLK_VMID_HIRES_DISCONNECT, CLK_VMID_LORES_CONNECT);
		CLK_ENABLE_VMID(CLK,0,0);
		SDADC->BSBGLDO |= (1<<7);
		SDADC->BSBGLDO |= SDADC_BSBGLDO_BGEN_Msk;
		SDADC->BSBGLDO |= SDADC_BSBGLDO_BGFLTCHRG_Msk;
		Delay(100000);
		//CLK_ENABLE_VMID(CLK, CLK_VMID_HIRES_DISCONNECT, CLK_VMID_LORES_CONNECT);
		CLK_ENABLE_VMID(CLK, CLK_VMID_HIRES_CONNECT, CLK_VMID_LORES_DISCONNECT);
		Delay(100000);
		SDADC->BSBGLDO |=SDADC_BSBGLDO_LDODIVEN_Msk;
		SDADC->BSBGLDO &= ~SDADC_BSBGLDO_BGFLTCHRG_Msk;
		SDADC->BSINISTRAMP  &= ~SDADC_BSINISTRAMP_CHOPEN_Msk;
	
		/* Power up the modulator */
		SDADC->SDCHOP &= ~SDADC_SDCHOP_PD_Msk;
	
		/* power up the Instrumentation Amplifer  */
		SDADC->BSINISTRAMP  |=SDADC_BSINISTRAMP_EN_Msk;
		
		SDADC->SDCHOP =(SDADC->SDCHOP  & ~SDADC_SDCHOP_AUDIOPATHSEL_Msk) |(0x2<<SDADC_SDCHOP_AUDIOPATHSEL_Pos);
		SDADC->SDCHOP = (SDADC->SDCHOP & ~SDADC_BSINISTRAMP_SYSCHOPF_Msk) |(0x2<<SDADC_BSINISTRAMP_SYSCHOPF_Pos); //SDADC->SDCHOP |= 1<<29;
		SDADC->SDCHOP = (SDADC->SDCHOP & ~ SDADC_BSINISTRAMP_CHOPF_Msk);// SDADC->SDCHOP &= ~(1<<26);
		SDADC->SDCHOP = (SDADC->SDCHOP & ~( SDADC_BSINISTRAMP_CHOPNPH_Msk|SDADC_BSINISTRAMP_CHOPPPH_Msk));//  SDADC->SDCHOP &= ~(3<<23);
		SDADC->SDCHOP =(SDADC->SDCHOP  & ~SDADC_BSINISTRAMP_GAIN_Msk) |(0x4<<SDADC_BSINISTRAMP_GAIN_Pos); // SDADC->SDCHOP |=(1<<3);
		SDADC->SDCHOP &= ~SDADC_SDCHOP_BIAS_Msk;

		BIQ->CTL |=BIQ_CTL_DLCOEFF_Msk; //RSTn
		BIQ->CTL |=BIQ_CTL_BIQEN_Msk; //
		BIQ->CTL &=~BIQ_CTL_PATHSEL_Msk;
		BIQ->CTL |=BIQ_CTL_STAGE_Msk; // stage 5/ 10th order
		BIQ->CTL &=~BIQ_CTL_HPFON_Msk;
		BIQ->CTL = (BIQ->CTL  & ~BIQ_CTL_DPWMPUSR_Msk) |(1<<BIQ_CTL_DPWMPUSR_Pos);
	
		SDADC->CLKDIV =4;	
		SDADC->CTL |= SDADC_CTL_BSRATE_Msk;
		SDADC->CTL |=SDADC_CTL_RATESEL_Msk;

		SDADC->BSBGLDO  = (SDADC->BSBGLDO& ~SDADC_BSBGLDO_CLKDIV_Msk)|(0x1e<<SDADC_BSBGLDO_CLKDIV_Pos);
		SDADC->BSBGLDO  |= SDADC_BSBGLDO_CLKEN_Msk;
		
		SDADC_SET_FIFODATABITS(SDADC, SDADC_FIFODATA_24BITS);
		
		
		SDADC->EN &= ~SDADC_EN_DINEDGE_Msk;
		
		
		SDADC_DisableInt(SDADC_FIFO_INT);
		
		// Determines the 4 words data in FIFO will generate a interrupt.
		SDADC_SET_FIFOINTLEVEL(SDADC, SDADC_INT_SAMPLES);		
		
		BSADC_IA_ENABLE_CHOPPER(TRUE);
		BSADC_BG_ENABLE_LFCHOPPER(TRUE);
		BSADC_BG_ENABLE_CHOPPER(TRUE);		
		
		
		SDADC->BSINISTRAMP= (SDADC->BSINISTRAMP & ~(SDADC_BSINISTRAMP_CHOPF_Msk))|(0<<SDADC_BSINISTRAMP_CHOPF_Pos);
		SDADC->BSINISTRAMP|=SDADC_BSINISTRAMP_CHOPEN_Msk;
		SDADC->BSBGLDO  |= SDADC_BSBGLDO_BGCHOPEN_Msk;
		
		SDADC->BSBGLDO  |= SDADC_BSBGLDO_BGCHOPLFEN_Msk;
		SDADC->BSBGLDO  &= ~SDADC_BSBGLDO_BGLFCHOPFRQSEL_Msk;
		
		
		BIQ->CTL |=BIQ_CTL_DLCOEFF_Msk; //RSTn
		BIQ->CTL |=BIQ_CTL_BIQEN_Msk; //
		BIQ->CTL &= ~BIQ_CTL_HPFON_Msk; //
		BIQ_SetCoeff((uint32_t *)BIQ_Coeffs[0]);
		
//		BIQ->CTL |= BIQ_CTL_PRGCOEFF_Msk;
//		for (ucCoefNum = 0; ucCoefNum < NUM_OF_BIQ_COEFFS; ++ucCoefNum)
//		{
//		  BIQ1->BIQ_COEFF[ucCoefNum] = BIQ_Coeffs[0][ucCoefNum];
//		}
//		
//		BIQ->CTL &= ~BIQ_CTL_PRGCOEFF_Msk;
		

		
		SDADC->BSINISTRAMP &= ~SDADC_BSINISTRAMP_GAIN_Msk;
		BSADC_IA_SetTrim(0,0);
		VOLCTRL->EN &= ~VOLCTRL_EN_SDADCVOLEN_Msk;
		
		SDADC->BSINISTRAMP |= SDADC_BSINISTRAMP_OFFSETTRIM_Msk;
		
		SDADC->BSINISTRAMP =(SDADC->BSINISTRAMP& 	~SDADC_BSINISTRAMP_GAIN_Msk)|(7<<SDADC_BSINISTRAMP_GAIN_Pos);
		SDADC->BSINISTRAMP |=SDADC_BSINISTRAMP_CHOPPDIS_Msk;
		SDADC->BSINISTRAMP |= (SDADC_BSINISTRAMP_OFFSETNEN_Msk|SDADC_BSINISTRAMP_OFFSETPEN_Msk);

		// Enable the Microphone Bias
		//SDADC_EnableMICBias(SDADC_MICBSEL_1P8V);
		// Enable the Microphone us BS LDO
		
		Delay(100000);
		CLK->APBCLK0|= CLK_APBCLK0_BIQALCKEN_Msk;
		CLK->CLKDIV0= 0x00011010;
		// SDADC power on
		
		SDADC_ANALOG_POWERON(SDADC);
		// Enable PGA and set 12dB gain value
		SDADC_ENABLE_PGA(SDADC, SDADC_PGACTL_GAIN_12DB);
		// PGA signal mute off
		SDADC_MUTEOFF_PGA(SDADC);
		SDADC->BSBGLDO |= SDADC_BSBGLDO_BGEN_Msk;
		SDADC->BSBGLDO |= SDADC_BSBGLDO_LDOEN_Msk;
		SDADC->BSBGLDO |= SDADC_BSBGLDO_BGFLTCHRG_Msk;
		SDADC->BSBGLDO |=SDADC_BSBGLDO_CLKEN_Msk;
		SDADC->BSBGLDO = (SDADC->BSBGLDO & ~SDADC_BSBGLDO_LDOSET_Msk)|(BSLDOLEVEL<<SDADC_BSBGLDO_LDOSET_Pos)|SDADC_BSBGLDO_BGFLTCHRG_Msk;
		SDADC->BSINISTRAMP |= SDADC_BSINISTRAMP_EN_Msk;
		SDADC->BSINISTRAMP &= ~(SDADC_BSINISTRAMP_SYSCHOPEN_Msk|SDADC_BSINISTRAMP_CHOPEN_Msk);
		
		SDADC->CTL =(SDADC->CTL & ~SDADC_CTL_FIFOBITS_Msk ) | SDADC_FIFODATA_24BITS;

	
	// Set 16-bit data selection for FIFO buffer 
	
	// Determines the 4 words data in FIFO will generate a interrupt.
	SDADC_SET_FIFOINTLEVEL(SDADC, SDADC_INT_SAMPLES);
	// Enable FIFO Threshold interrupt
	SDADC_EnableInt(SDADC_FIFO_INT);
	
	// Enable NVIC 
	NVIC_ClearPendingIRQ(SDADC_IRQn);
	NVIC_EnableIRQ(SDADC_IRQn);
}

void UART0_Init(void)
{
    // Enable SARADC module clock. 
	CLK_EnableModuleClock(UART0_MODULE);
	
	/* Reset UART module */
    SYS_ResetModule(UART0_RST);
	
	/* Peripheral clock source */
    CLK_SetModuleClock(UART0_MODULE, MODULE_NoMsk, CLK_CLKDIV0_UART0(1));
	    
    // Init I/O multi-function ; UART0: GPA4=TX, GPA5= RX
	SYS->GPA_MFP  = (SYS->GPA_MFP & ~(SYS_GPA_MFP_PA4MFP_Msk|SYS_GPA_MFP_PA5MFP_Msk) ) |  (SYS_GPA_MFP_PA4MFP_UART0_TX|SYS_GPA_MFP_PA5MFP_UART0_RX)	;

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}

int32_t BSADC_IA_CalibrateTrim()
{
	int32_t u32i,u32k,u32ArrayData,MinOFFSETN,MinOFFSETP,u32Temp,u32Temp1,MinADCValaueP,MinADCValaueN;
	int32_t u32ArrayDataBAK[2][64];
  SYS_UnlockReg();

	CLK->APBCLK0 |=CLK_APBCLK0_BIQALCKEN_Msk;
	BIQ->CTL=BIQ_CTL_STAGE_Msk |BIQ_CTL_DPWMPUSR_Msk|(1<<BIQ_CTL_SDADCWNSR_Pos)|BIQ_CTL_DLCOEFF_Msk; //BIQ->CTL= 0x00000918;
	MinADCValaueP=0x7fffffff;
	dataarraycnt=TrimPointnumber;
	fMoveavg=1;

	SDADC->BSBGLDO=0x9e00400d;
	SDADC->BSINISTRAMP =0x001490b1;
	
	SDADC->BSINISTRAMP = (SDADC->BSINISTRAMP & ~(SDADC_BSINISTRAMP_OFFSETNEN_Msk|SDADC_BSINISTRAMP_OFFSETPEN_Msk))|SDADC_BSINISTRAMP_OFFSETPEN_Msk;
	SDADC->BSINISTRAMP = (SDADC->BSINISTRAMP &  ~(SDADC_BSINISTRAMP_CHOPNDIS_Msk|SDADC_BSINISTRAMP_CHOPPDIS_Msk) )|SDADC_BSINISTRAMP_CHOPPDIS_Msk;
	SDADC->BSINISTRAMP = (SDADC->BSINISTRAMP  & ~SDADC_BSINISTRAMP_GAIN_Msk)| SDADC_BSINISTRAMP_GAIN_Msk;
	SDADC_START_CONV(SDADC);	

	for(u32i=63;u32i>=0;u32i--)  // Trim code 0x20 doesn't exist
	{
		if(u32i==32) continue;
		{
			{
				u32Temp1=u32i;
				if(u32Temp1<32) u32Temp1 ^=0x1f;  // Note CalibrateTrim value difference 
			SDADC->BSINISTRAMP=(SDADC->BSINISTRAMP & ~(SDADC_BSINISTRAMP_OFFSETN_Msk|SDADC_BSINISTRAMP_OFFSETP_Msk))  | (u32Temp1<<SDADC_BSINISTRAMP_OFFSETP_Pos) ;
			u32Count=0;
			u32k=dataarraycnt;
			u32ArrayData=0;
				
			while(Moveavg==0);
			Moveavg=0;				
			while(u32k>(TrimPointnumber/2))
			{
				u32Temp= (int) u32ADCdata[u32k-1];
				if(u32Temp&(1<<24)){ u32Temp|=0xff000000; u32Temp=-u32Temp;}
					
				u32ArrayData+= u32Temp;
				u32k--;
			}
			u32ArrayData=u32ArrayData/(dataarraycnt-(TrimPointnumber/2));			
			u32Count=0;
			u32ArrayDataBAK[0][u32Temp1]=u32ArrayData;
			if((MinADCValaueP>u32ArrayData))
				{
				MinADCValaueP = u32ArrayData;
				MinOFFSETP = u32Temp1;
				}
			}
		}
	}
	
	MinADCValaueN=0x7fffffff;
	SDADC->BSINISTRAMP = (SDADC->BSINISTRAMP & ~(SDADC_BSINISTRAMP_OFFSETNEN_Msk|SDADC_BSINISTRAMP_OFFSETPEN_Msk))|SDADC_BSINISTRAMP_OFFSETNEN_Msk;
	SDADC->BSINISTRAMP = (SDADC->BSINISTRAMP &  ~(SDADC_BSINISTRAMP_CHOPNDIS_Msk|SDADC_BSINISTRAMP_CHOPPDIS_Msk) )|SDADC_BSINISTRAMP_CHOPNDIS_Msk;
	fMoveavg=1;
	for(u32i=63;u32i>=0;u32i--)
	{
		if(u32i==32) continue; // Trim code 0x20 doesn't exist
		{
			{
			u32Temp1=u32i;
			if(u32Temp1<32) u32Temp1 ^=0x1f;  // Note CalibrateTrim value difference 
				
			SDADC->BSINISTRAMP=(SDADC->BSINISTRAMP & ~(SDADC_BSINISTRAMP_OFFSETN_Msk|SDADC_BSINISTRAMP_OFFSETP_Msk))  | (u32Temp1<<SDADC_BSINISTRAMP_OFFSETN_Pos) ;
			u32Count=0;
			u32k=dataarraycnt;
			u32ArrayData=0;
				
			while(Moveavg==0);
			Moveavg=0;				
			while(u32k>(TrimPointnumber/2))
			{
				u32Temp= (int) u32ADCdata[u32k-1];
				if(u32Temp&(1<<24)){ u32Temp|=0xff000000; u32Temp=-u32Temp;}
					
				u32ArrayData+= u32Temp;
				u32k--;
			}
			u32ArrayData=u32ArrayData/(dataarraycnt-(TrimPointnumber/2));			
			u32Count=0;
			u32ArrayDataBAK[1][u32Temp1]=u32ArrayData;
			
			if((MinADCValaueN>u32ArrayData))
				{
				MinADCValaueN = u32ArrayData;
				MinOFFSETN = u32Temp1;
				}
			}
		}
	}
	#if 1
	for(u32i=63;u32i>=0;u32i--)
	{
		if(u32i==32) continue; //
			u32Temp1=u32i;
			if(u32Temp1<32) u32Temp1 ^=0x1f;  // Note CalibrateTrim value difference 		
				printf("%.2d %d\r\n",63-u32i,u32ArrayDataBAK[0][63-u32i]);
	}
	printf("\r\n\r\n\r\n\r\n\r\n");
	for(u32i=63;u32i>=0;u32i--)
	{
		if(u32i==32) 		continue; //
			u32Temp1=u32i;
			if(u32Temp1<32) u32Temp1 ^=0x1f;  // Note CalibrateTrim value difference 		
				printf("%.2d %d\r\n",63-u32i ,u32ArrayDataBAK[1][63-u32i]);
	}
	
	#endif
	printf("MinOFFSETN =%.2x( MinADCValaue N=%.2x ) ,MinOFFSETP =%.2x( MinADCValaue P=%.2x )\r\n",MinOFFSETN,MinADCValaueN,MinOFFSETP,MinADCValaueP);
	printf("MinOFFSETN =%.2x( MinADCValaue N=%.2x ) ,MinOFFSETP =%.2x( MinADCValaue P=%.2x )\r\n",MinOFFSETN,MinADCValaueN,MinOFFSETP,MinADCValaueP);
	
	SDADC->BSINISTRAMP=(SDADC->BSINISTRAMP & ~(SDADC_BSINISTRAMP_OFFSETN_Msk|SDADC_BSINISTRAMP_OFFSETP_Msk))  | (MinOFFSETP<<SDADC_BSINISTRAMP_OFFSETP_Pos) | (MinOFFSETN<<SDADC_BSINISTRAMP_OFFSETN_Pos);
	SDADC->BSINISTRAMP = (SDADC->BSINISTRAMP &  ~(SDADC_BSINISTRAMP_CHOPNDIS_Msk|SDADC_BSINISTRAMP_CHOPPDIS_Msk) );
	dataarraycnt=8;
	fMoveavg=0;
	SDADC_STOP_CONV(SDADC);
	return ((MinOFFSETN<<SDADC_BSINISTRAMP_OFFSETN_Pos)| (MinOFFSETP<<SDADC_BSINISTRAMP_OFFSETP_Pos));
}

void GetDSADC()
{
	
		int i=0,k;
		int GetSample;
		GetSample =BS_PDMA_LENGTH;
		i= GetSample;
	  while(i>0) 	{	DataOutput[i-1]=0;i--;}
		SYS_UnlockReg();
		Read_DSADCOnly=1;
		SDADC->BSINISTRAMP=0x000494B1;
		CLK_EnableModuleClock(PDMA_MODULE);	
		PDMA_Close();
		SYS_ResetModule(PDMA_RST);
		NVIC_DisableIRQ(SDADC_IRQn);
		ENOB_GetData=0;
    /* Open Channel TEST_CH */
    PDMA_Open(1<<BSPDMACHANNEL );
    PDMA_SetTransferCnt(BSPDMACHANNEL, PDMA_WIDTH_32, GetSample+4);
    PDMA_SetTransferAddr(BSPDMACHANNEL, (uint32_t)&SDADC->DAT, PDMA_SAR_FIX, (uint32_t)u32ADCdata, PDMA_DAR_INC);
		PDMA_SetTransferDirection(BSPDMACHANNEL,PDMA_APB_SRAM);
    PDMA_EnableInt(BSPDMACHANNEL, PDMA_INTSTS_TXIF_Msk);			
		PDMA_SetTransferMode( BSPDMACHANNEL,PDMA_SDADC);
	  PDMA_INT_CNT=512;
    PDMA_Trigger(0);
		NVIC_EnableIRQ(PDMA_IRQn);			
		SDADC_ENABLE_PDMA(SDADC);	

#if 0		
		j=0;
		FMC_Open();
		FMC_ENABLE_APROM_WRITABLE();
		printf("Erase Flash....\r\n");
		while(IsDebugFifoEmpty()==0);
		for(j=0;j<32;j++)
		{
			FMC_Erase(0x10000+j*512);
			
		}
		printf("Erase Done.\r\nStart Convert....\r\n");
		while(IsDebugFifoEmpty()==0);
	
		j=0;
		SDADC_START_CONV(SDADC);		
		while(1)
		{
			k=0;
			if(ENOB_GetData==0) continue;
			ENOB_GetData=0;
			i++;
			if(i>128) break;
			for(k=4;k<GetSample+4;k++)
			{
			FMC_Write(0x10000+j*4,u32ADCdata[k]);
			j++;
			}
		}
#else
			SDADC_START_CONV(SDADC);		
				while(ENOB_GetData==0);
				ENOB_GetData=0;
		while(IsDebugFifoEmpty()==0);
			printf("offset %d\r\n",u32ADCdata[0]);	
		while(IsDebugFifoEmpty()==0);
			for(k=4;k<GetSample+4;k++)
			{
					printf("%d\r\n",u32ADCdata[k]-u32ADCdata[0]);		
			}
#endif	
		
		
				SDADC_STOP_CONV(SDADC);	
			SYS_ResetModule(PDMA_RST);
				PDMA_Close();
		Read_DSADCOnly=0;
}

void		ENOB_Measure(int GetSample,int BS_Read_Mode)
{
		int i=0,BS_AVE_DC_Valuue;
		i= GetSample;
	  while(i>0) 	{	DataOutput[i-1]=0;i--;}

		CLK_EnableModuleClock(PDMA_MODULE);	
		PDMA_Close();
		SYS_ResetModule(PDMA_RST);
		NVIC_DisableIRQ(SDADC_IRQn);
		ENOB_GetData=0;
    /* Open Channel TEST_CH */
    PDMA_Open(1<<BSPDMACHANNEL );
    PDMA_SetTransferCnt(BSPDMACHANNEL, PDMA_WIDTH_32, BS_PDMA_LENGTH);
    PDMA_SetTransferAddr(BSPDMACHANNEL, (uint32_t)&SDADC->DAT, PDMA_SAR_FIX, (uint32_t)u32ADCdata, PDMA_DAR_INC);
		PDMA_SetTransferDirection(BSPDMACHANNEL,PDMA_APB_SRAM);
    PDMA_EnableInt(BSPDMACHANNEL, PDMA_INTSTS_TXIF_Msk);			
		PDMA_SetTransferMode( BSPDMACHANNEL,PDMA_SDADC);
	  PDMA_INT_CNT=GetSample<<1;
    PDMA_Trigger(0);
		NVIC_EnableIRQ(PDMA_IRQn);			
		SDADC_ENABLE_PDMA(SDADC);	
		SDADC_START_CONV(SDADC);
	
	  BS_AVE_DC_Valuue=0;
		while(i<GetSample)
		{

			if(ENOB_GetData==1)
			{
				//printf("%x\r\n",BsDataOutput);
				
				if(i>4)
				{
					BS_AVE_DC_Valuue+= (int)BsDataOutput;
				}
				ENOB_GetData=0;
				i++;
				
			}
		}	
			SDADC_STOP_CONV(SDADC);	
			SYS_ResetModule(PDMA_RST);
			PDMA_Close();
				
								
		switch(BS_Read_Mode)
		{
			case DC_Value:
						BS_para.BS_AVE_DC_Valuue=BS_AVE_DC_Valuue/(GetSample-4);
						break;
			case BS_Read_Value0:
						BS_para.BS_Read_Value_0+=BS_AVE_DC_Valuue;
						BS_para.BS_Read_Cnt_0+=(GetSample-4);
						if((BS_para.BS_Read_Value_0>16777215)||(BS_para.BS_Read_Value_0< -16777215))
						{ BS_para.BS_Read_Value_0=BS_para.BS_Read_Value_0>>1;BS_para.BS_Read_Cnt_0=BS_para.BS_Read_Cnt_0>>1;}
			
						break;			
			case BS_Read_Value1:
						BS_para.BS_Read_Value_1+=BS_AVE_DC_Valuue;
						BS_para.BS_Read_Cnt_1+=(GetSample-4);
						if((BS_para.BS_Read_Value_1>16777215)||(BS_para.BS_Read_Value_1< -16777215))
						{ BS_para.BS_Read_Value_1=BS_para.BS_Read_Value_1>>1;BS_para.BS_Read_Cnt_1=BS_para.BS_Read_Cnt_1>>1;}
						break;		
			case BS_Read_Value2:
						BS_para.BS_Read_Value_2+=BS_AVE_DC_Valuue;
						BS_para.BS_Read_Cnt_2+=(GetSample-4);
						if((BS_para.BS_Read_Value_2>16777215)||(BS_para.BS_Read_Value_2< -16777215))
						{ BS_para.BS_Read_Value_2=BS_para.BS_Read_Value_2>>1;BS_para.BS_Read_Cnt_2=BS_para.BS_Read_Cnt_2>>1;}
			break;	
			case BS_Read_Value3:
						BS_para.BS_Read_Value_3+=BS_AVE_DC_Valuue;
						BS_para.BS_Read_Cnt_3+=(GetSample-4);
						if((BS_para.BS_Read_Value_3>16777215)||(BS_para.BS_Read_Value_3< -16777215))
						{ BS_para.BS_Read_Value_3=BS_para.BS_Read_Value_3>>1;BS_para.BS_Read_Cnt_3=BS_para.BS_Read_Cnt_3>>1;}
			break;	

		}
		
		
}

void Get_BS_DC()
{
	SDADC->BSINISTRAMP &= ~SDADC_BSINISTRAMP_OFFSETTRIM_Msk;
	ENOB_Measure(BS_SAMPLE_NUMBER,DC_Value);
}
void Get_BS_Value0()
{
	SDADC->BSINISTRAMP &= ~SDADC_BSINISTRAMP_OFFSETTRIM_Msk;
	ENOB_Measure(BS_SAMPLE_NUMBER,BS_Read_Value0);
}

void Get_BS_Value1()
{
	SDADC->BSINISTRAMP &= ~SDADC_BSINISTRAMP_OFFSETTRIM_Msk;
	ENOB_Measure(BS_SAMPLE_NUMBER,BS_Read_Value1);
}
void Get_BS_Value2()
{
	SDADC->BSINISTRAMP &= ~SDADC_BSINISTRAMP_OFFSETTRIM_Msk;
	ENOB_Measure(BS_SAMPLE_NUMBER,BS_Read_Value2);
}
void Get_BS_Value3()
{
	SDADC->BSINISTRAMP &= ~SDADC_BSINISTRAMP_OFFSETTRIM_Msk;
	ENOB_Measure(BS_SAMPLE_NUMBER,BS_Read_Value3);
}
// FMC =========================================================================
#define FLASH_TOTAL_SIZE			(0x20000)	//128k bytes
#define VFMC_LENGTH_USERCONFIG 4
void CheckCalibrateData(BOOL fTrim)
{
	UINT32 au32Config[VFMC_LENGTH_USERCONFIG];

	SYS_UnlockReg();
	FMC_Open();
	FMC_ENABLE_CFG_UPDATE();
	FMC_ReadConfig(au32Config, VFMC_LENGTH_USERCONFIG);
	if((au32Config[2]==0) ||(au32Config[2]==~0)||(fTrim==1))
	{
		// Start Calibrate
		dataarraycnt=8;
		DataOut=0;
		PDMA_Close();
		SYS_ResetModule(PDMA_RST);		
		ADC_Init();
		CLK_EnableModuleClock(BFAL_MODULE);
		SYS_ResetModule(BIQ_RST);
		BIQ_LoadDefaultCoeff();				
		BSADC_IA_CalibrateTrim();
		au32Config[2]= SDADC->BSBGLDO;
		au32Config[3]= SDADC->BSINISTRAMP;

		// Enable LDO and set 1.8V
		au32Config[2] |=SDADC_BSBGLDO_LDOEN_Msk;
		au32Config[2] = (au32Config[2] & ~SDADC_BSBGLDO_LDOSET_Msk)|(BSLDOLEVEL<<SDADC_BSBGLDO_LDOSET_Pos)|SDADC_BSBGLDO_BGFLTCHRG_Msk;
		SDADC->BSBGLDO =au32Config[2];
		FMC_Erase(FMC_CONFIG_BASE);
		FMC_WriteConfig(au32Config, VFMC_LENGTH_USERCONFIG);
	}
	else
	{
		// Enable LDO and set 1.8V
		au32Config[2] |=SDADC_BSBGLDO_LDOEN_Msk;
		au32Config[2] = (au32Config[2] & ~SDADC_BSBGLDO_LDOSET_Msk)|(BSLDOLEVEL<<SDADC_BSBGLDO_LDOSET_Pos)|SDADC_BSBGLDO_BGFLTCHRG_Msk;
		 SDADC->BSBGLDO=au32Config[2];
		 SDADC->BSINISTRAMP=au32Config[3];		
	}
	FMC_DISABLE_CFG_UPDATE();
	FMC_Close();
	SYS_LockReg();
}
/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/

int32_t main(void)
{                                                                                                                                         
	
	int32_t u32i,u32GetValueCnt;
	float scale,scale1,EmptyValue,ZeroOffset,Vdd;
	BsDataOutput=BsDataN=BsDataP=0;
	BSLDOLEVEL=0;
	System_Initiate();
	UART0_Init();
	dataarraycnt=8;
	DataOut=0;
	Read_DSADCOnly=0;
	ADC_Init();
																																																																					 
	CLK_EnableModuleClock(BFAL_MODULE);
	SYS_ResetModule(BIQ_RST);
	BIQ_LoadDefaultCoeff();																																																																					 
	
	CheckCalibrateData(0);
	while(1) {
	printf("\n\nCPU @ %d Hz,Vdd= %f\n", SystemCoreClock,GetVdd());
  printf("+-------------------------------------------------+\n");
  printf("|  Demo BS-Sense input to SDADC	                  |\n");
  printf("+-------------------------------------------------+\n\n");
	Moveavg=fMoveavg =0;
	printf("0: Load Parameter or Calibrate offset Trim.\n");
	printf("1: +BS-Sense to SDADC measure (NoLoad) .\n");
	printf("2: +BS-Sense to SDADC measure.\n");
	printf("3: Change BSLDO and Get SDADC measure.\n");
	Optionitem = getchar();
	Vdd=GetVdd();
	

	switch(Optionitem)
	{
		case '0':
			CheckCalibrateData(1);
			 break;
		case '1':
			Get_BS_DC();
			Get_BS_Value0();
			//Get_BS_Value1();
			Get_BS_Value2();
			Get_BS_Value3();		
			printf("AVG : %.8d  \r\n", BS_para.BS_AVE_DC_Valuue);
			printf("DC 0: %.8d  \r\n", BS_para.BS_Read_Value_0/BS_para.BS_Read_Cnt_0);
			printf("DC 2: %.8d  \r\n", BS_para.BS_Read_Value_2/BS_para.BS_Read_Cnt_2);
			printf("DC 3: %.8d  \r\n", BS_para.BS_Read_Value_3/BS_para.BS_Read_Cnt_3);
		  break;
		case '2':
			
			 u32i=((SDADC->BSBGLDO& SDADC_BSBGLDO_LDOSET_Msk)>>SDADC_BSBGLDO_LDOSET_Pos);
			
		   if((Vdd<(BSLDOVOL[u32i]+0.3)) ||(u32i!=BSLDOLEVEL))
			 {
				 BSLDOLEVEL =u32i;
				 if(BSLDOLEVEL!=0) BSLDOLEVEL--;
				 SDADC->BSBGLDO= (SDADC->BSBGLDO& ~SDADC_BSBGLDO_LDOSET_Msk)|(BSLDOLEVEL<<SDADC_BSBGLDO_LDOSET_Pos)|SDADC_BSBGLDO_BGFLTCHRG_Msk;
				 memset(&BS_para,0x0,sizeof(BS_para));
				Get_BS_DC();
				Get_BS_Value0();
			//Get_BS_Value1();
				Get_BS_Value2();
				Get_BS_Value3();		
				printf("AVG : %.8d  \r\n", BS_para.BS_AVE_DC_Valuue);
				printf("DC 0: %.8d  \r\n", BS_para.BS_Read_Value_0/BS_para.BS_Read_Cnt_0);
				printf("DC 2: %.8d  \r\n", BS_para.BS_Read_Value_2/BS_para.BS_Read_Cnt_2);
				printf("DC 3: %.8d  \r\n", BS_para.BS_Read_Value_3/BS_para.BS_Read_Cnt_3);				 
				 
			 }
			 scale1= BSLDOVOL[BSLDOLEVEL] / 1.8;
			 u32GetValueCnt=10;
			 u32i=0;
			while(u32GetValueCnt>0){
				u32GetValueCnt--;
				Get_BS_Value2();
				PDMA_Close();
				SYS_ResetModule(PDMA_RST);
				SDADC_STOP_CONV(SDADC);			
				//ZeroOffset=(float)((BS_para.BS_Read_Value_2/BS_para.BS_Read_Cnt_2 -BS_para.BS_Read_Value_1/BS_para.BS_Read_Cnt_1 -(BS_para.BS_Read_Value_3/BS_para.BS_Read_Cnt_3-BS_para.BS_Read_Value_1/BS_para.BS_Read_Cnt_1  )));
				ZeroOffset=(float)((BS_para.BS_Read_Value_2/BS_para.BS_Read_Cnt_2 -(BS_para.BS_Read_Value_3/BS_para.BS_Read_Cnt_3)));
			  if(ABS(ZeroOffset)<50) 				EmptyValue=(float)(ZeroOffset/2);
			  scale=ZeroOffset-EmptyValue;
				scale= (ZeroOffset/75);
				scale =scale/ scale1;
				scale =-scale;
#if 1				
				if(scale>220.0)  scale/=1.009; 
				else if(scale>50.0)  scale/=1.0098;
#endif	

#if 0				
				if(scale<-5)
				{
					u32i++;
					if(u32i>5) { u32i=0; 
						printf("Re-Initial...Fail..\r\n"); 
						break;
					}
						
					memset(&BS_para,0x0,sizeof(BS_para));
					Get_BS_DC();
					Get_BS_Value0();
					Get_BS_Value2();
					Get_BS_Value3();	
					printf("Re-Initial...\r\n"); continue;
				}
#endif				
				printf("%.8d  %.8d  %.8d  %.8d %.8d\r\n", BS_para.BS_Read_Cnt_2, BS_para.BS_Read_Value_2, BS_para.BS_Read_Value_3, (int)ZeroOffset,(int)scale);
				BS_para.BS_Read_Value_2=0;
				BS_para.BS_Read_Cnt_2=0;		
		
			}
			break;

		case '3':
				printf(" 0 = 1.5(1.2 )\r\n");
				printf(" 1 = 1.8(1.4 )\r\n");
				printf(" 2 = 2.1(1.65)\r\n");
				printf(" 3 = 2.4(1.9 )\r\n");
				printf(" 4 = 2.7(2.1 )\r\n");
				printf(" 5 = 3.0(2.4 )\r\n");
				printf(" 6 = 3.3(2.6 )\r\n");
				printf(" 7 = 3.6(2.8 )\r\n");
				printf(" 8 = 3.9(3.0 )\r\n");
				printf(" 9 = 4.2(3.3 )\r\n");
				printf(" A = 4.5(3.6 )\r\n");
				printf(" B = 4.5(4.9 )\r\n");  

				Optionitem = getchar();
				if((Optionitem=='B')||(Optionitem=='b')) Optionitem=11;
				else if((Optionitem=='A')||(Optionitem=='a')) Optionitem=11;
				else if((Optionitem>='0')&&(Optionitem<='9')) Optionitem=Optionitem-0x30;
				else { printf(" error \r\n"); break;}
				BSLDOLEVEL=Optionitem;
				SDADC->BSBGLDO= (SDADC->BSBGLDO& ~SDADC_BSBGLDO_LDOSET_Msk)|(BSLDOLEVEL<<SDADC_BSBGLDO_LDOSET_Pos)|SDADC_BSBGLDO_BGFLTCHRG_Msk;
				GetDSADC();
				break;

		}
	}
}

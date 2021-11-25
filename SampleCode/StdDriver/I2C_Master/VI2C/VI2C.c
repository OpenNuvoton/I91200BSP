//#include "CompilerOptionO2.h"
#include <string.h>
#include "VI2C.h"
#include "VCommon.h"

#define VI2C_COUNT                 (1)
#define VI2C_SLAVEMODE             (0x80)

// Register state
#define VI2C_REGSTA_M_START             (0x08)  // Start has been transmitted.
#define VI2C_REGSTA_M_SLAW_ACK          (0x18)  // SLA+W has been transmitted. ACK bit will be received.
#define VI2C_REGSTA_M_SLAW_NACK         (0x20)  // SLW+W has been transmitted. NACK bit will be received.
#define VI2C_REGSTA_M_TXDATA_ACK        (0x28)  // Data has been transmitted. ACK bit will be received.
#define VI2C_REGSTA_M_TXDATA_NACK       (0x30)  // Data has been transmitted. NACK bit will be received.
#define VI2C_REGSTA_M_SLAR_ACK          (0x40)  // SLA+R has been transmitted. ACK has been received.
#define VI2C_REGSTA_M_SLAR_NACK         (0x48)  // SLA+R has been transmitted. NACK has been received.
#define VI2C_REGSTA_M_RXDATA_ACK        (0x50)  // Data has been transmitted. ACK has been received.
#define VI2C_REGSTA_M_RXDATA_NACK       (0x58)  // Data has been transmitted. NACK has been received.
#define VI2C_REGSTA_S_SLAR_ACK          (0xA8)  // SLA+R has been received. ACK has been received.
#define VI2C_REGSTA_S_LOST_SLAR_ACK     (0xB0)  // Lost SLA+R/W. SLA+R has been received. ACK has been received.
#define VI2C_REGSTA_S_TXDATA_ACK        (0xB8)  // Data has been transmitted. ACK has been received.
#define VI2C_REGSTA_S_TXLDATA_ACK       (0xC8)  // Last data has been transmitted. ACK has been received.
#define VI2C_REGSTA_S_TXDATA_NACK       (0xC0)  // Data or last data has been transmitted. NACK has been received.
#define VI2C_REGSTA_S_SLAW_ACK          (0x60)  // SLA+W has been received. ACK has been received.
#define VI2C_REGSTA_S_LOST_SLAW_ACK     (0x68)  // Lost SLA+R/W. SLA+W has been received. ACK has been received.
#define VI2C_REGSTA_S_RXDATA_ACK        (0x80)  // Data has been received. ACK has been received.
#define VI2C_REGSTA_S_RXDATA_NACK       (0x88)  // Data has been received. NACK has been received.


// Structure for VI2C control handler.
typedef struct
{
    __IO UINT16  u16TxCount; 
    __IO UINT16  u16RxCount; 
    __IO PUINT8  pu8TxBuf;
    __IO PUINT8  pu8RxBuf;
} S_VI2C_DATABUF;

// Structure for VI2C control handler.
typedef struct
{
	__IO UINT8   u8DeviceAddr;  
} S_VI2C_CTRL;

// Declare hardware information about VI2C
const S_VHW_INFO g_I2CHwInfo[VI2C_COUNT] =                   
{                                                            
	{I2C0, I2C0_IRQn, I2C0_MODULE, I2C0_RST, NULL}
};

S_VI2C_DATABUF g_asVI2CDataBuf[VI2C_COUNT];
S_VI2C_CTRL    g_asVI2CCtrl[VI2C_COUNT];

UINT32 VI2C_Open(UINT8 u8VI2CNo, UINT32 u32Frequency, enum eVI2C_Clock eClockSource, BOOL bReset)
{
	CLK_EnableModuleClock(g_I2CHwInfo[u8VI2CNo].u32ModuleID);	
	
	if(bReset)
	{
		SYS_ResetModule(g_I2CHwInfo[u8VI2CNo].u32ResetID);
	}
	if( I2C_Open((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr, u32Frequency) )
	{
		VI2C_Config(u8VI2CNo,VI2C_CFG_MASTER|VI2C_CFG_INTEN|VI2C_CFG_TODIS);
		memset(g_asVI2CDataBuf,'\0',sizeof(S_VI2C_DATABUF));
		memset(g_asVI2CCtrl,'\0',sizeof(S_VI2C_CTRL));
		return TRUE;
	}
	return FALSE;
};

void VI2C_Close(UINT8 u8VI2CNo)
{
	I2C_DisableInt((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr);
	I2C_Close(g_I2CHwInfo[u8VI2CNo].pHwAddr);		
	CLK_DisableModuleClock(g_I2CHwInfo[u8VI2CNo].u32ModuleID);
	g_asVI2CDataBuf[u8VI2CNo].u16TxCount = g_asVI2CDataBuf[u8VI2CNo].u16RxCount = 0;	
	g_asVI2CCtrl[u8VI2CNo].u8DeviceAddr = 0;
}

void VI2C_Start(UINT8 u8VI2CNo)
{
	if( g_asVI2CCtrl[u8VI2CNo].u8DeviceAddr&VI2C_SLAVEMODE )
	{
		I2C_SET_CONTROL_REG((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr, I2C_SI|I2C_AA);	
	}
	else
	{
		I2C_SET_CONTROL_REG((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr, I2C_SI|I2C_STA);	
	}
}

void VI2C_Stop(UINT8 u8VI2CNo)
{
	I2C_STOP((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr);
	g_asVI2CDataBuf[u8VI2CNo].u16TxCount = g_asVI2CDataBuf[u8VI2CNo].u16RxCount = 0;
}

void VI2C_Config(UINT8 u8VI2CNo,UINT32 u32Configuration)
{
	UINT32 u32Tmp = 0;
	
	// Config type(VI2C_CFG_MASTER/VI2C_CFG_SLAVE)
	g_asVI2CCtrl[u8VI2CNo].u8DeviceAddr = (g_asVI2CCtrl[u8VI2CNo].u8DeviceAddr&~VI2C_SLAVEMODE)|((u32Configuration&VI2C_CFG_SLAVE)?VI2C_SLAVEMODE:0);
		
	// Configurate time out(VI2C_CFG_LTOEN/VI2C_CFG_TOEN/VI2C_CFG_TODIS)
	if( ( u32Tmp = u32Configuration&(VI2C_CFG_LTOEN|VI2C_CFG_TOEN) ) > 0 )
	{
		if( u32Tmp == VI2C_CFG_LTOEN || u32Tmp == VI2C_CFG_TOEN )
		{
			I2C_EnableTimeout((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr,(UINT8)(u32Configuration&VI2C_CFG_LTOEN));
		}
	}
	else
	{
		I2C_DisableTimeout((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr);
	}
	// Configurate interrupt(VI2C_CFG_INTEN/VI2C_CFG_INTDIS)
	if( u32Configuration&VI2C_CFG_INTDIS )
	{
		NVIC_DisableIRQ(g_I2CHwInfo[u8VI2CNo].eHwIRQn);
		I2C_DisableInt((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr);		
		NVIC_ClearPendingIRQ(g_I2CHwInfo[u8VI2CNo].eHwIRQn);
	}
	else
	{
		NVIC_ClearPendingIRQ(g_I2CHwInfo[u8VI2CNo].eHwIRQn);
		NVIC_EnableIRQ(g_I2CHwInfo[u8VI2CNo].eHwIRQn);
		I2C_EnableInt((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr);
	}
}

void VI2C_ConfigDeviceAddress(UINT8 u8VI2CNo,UINT8 u8Address)
{
	g_asVI2CCtrl[u8VI2CNo].u8DeviceAddr = (g_asVI2CCtrl[u8VI2CNo].u8DeviceAddr&VI2C_SLAVEMODE)|(u8Address&~VI2C_SLAVEMODE);
}

void VI2C_ConfigSlaveAddress(UINT8 u8VI2CNo, UINT8 u8SlaveNo,UINT8 u8Address,UINT8 u8AddressMask)
{
	I2C_SetSlaveAddr((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr,u8SlaveNo, u8Address,I2C_GCMODE_DISABLE);
	I2C_SetSlaveAddrMask((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr,u8SlaveNo,u8AddressMask);		
}

void VI2C_SetTxData(UINT8 u8VI2CNo,PUINT8 pu8Data,UINT32 u32DataCount)
{
	if( u32DataCount && pu8Data != NULL )
	{
		g_asVI2CDataBuf[u8VI2CNo].pu8TxBuf = pu8Data;
		g_asVI2CDataBuf[u8VI2CNo].u16TxCount = u32DataCount;
	}
}

void VI2C_SetRxData(UINT8 u8VI2CNo,PUINT8 pu8Data,UINT32 u32DataCount)
{
	if( u32DataCount && pu8Data != NULL )
	{
		g_asVI2CDataBuf[u8VI2CNo].pu8RxBuf = pu8Data;
		g_asVI2CDataBuf[u8VI2CNo].u16RxCount = u32DataCount;
	}
}

BOOL VI2C_IsBusy(UINT8 u8VI2CNo)
{
	if( g_asVI2CDataBuf[u8VI2CNo].u16TxCount || g_asVI2CDataBuf[u8VI2CNo].u16RxCount )
	{
		return TRUE;
	}
	return FALSE;
}

void VI2C_WaitComplete(UINT8 u8VI2CNo)
{
	while(VI2C_IsBusy(u8VI2CNo) == TRUE);
}

UINT32 VI2C_Process(UINT8 u8VI2CNo)
{
	S_VI2C_DATABUF *psVI2CDataBuf = &g_asVI2CDataBuf[u8VI2CNo];
	S_VI2C_CTRL *psVI2CCtrl = &g_asVI2CCtrl[u8VI2CNo];
	I2C_T* psVI2CHw = (I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr;
	UINT32 u32ProcessFlag = VI2C_STA_NONE;
        UINT8 u8Tmp = 0;
	
	if( (((I2C_T*)g_I2CHwInfo[u8VI2CNo].pHwAddr)->CTL&I2C_CTL_INTEN_Msk)==FALSE && VI2C_IsBusy(u8VI2CNo)==FALSE)
	{
		return VI2C_STA_NONE;
	}
	if( I2C_GET_TIMEOUT_FLAG(psVI2CHw) )
	{
		I2C_ClearTimeoutFlag(psVI2CHw);
		
		// Callback function when time out happened; otherwise, feedback VI2C_STA_TIMEOUT flag.
		#if( VI2C_PROCESSEVENT )
		u32ProcessFlag = VI2C_STA_TIMEOUT;
		#else
		VI2C_TimeOut(u8VI2CNo);
		#endif
	}
	else
	{
		switch(I2C_GET_STATUS(psVI2CHw))
		{
			// Slave mode status(Rx)
			// Own SLA+W has been receive; ACK has been return
			case VI2C_REGSTA_S_SLAW_ACK:
			case VI2C_REGSTA_S_LOST_SLAW_ACK:
				// Callback receive request for user to set input data buffer.
				if( psVI2CDataBuf->u16RxCount == 0 )
				{
					#if( VI2C_PROCESSEVENT )
					u32ProcessFlag = VI2C_STA_RXREQUEST;
					#else
					VI2C_ReceiveRequest(u8VI2CNo);
					#endif
				}
                                u8Tmp = (psVI2CDataBuf->u16RxCount)?I2C_AA:0;
				I2C_SET_CONTROL_REG(psVI2CHw, (I2C_SI|u8Tmp));
				break;
			
			// Slave mode status(Rx)
			// Previously address with own SLA address 
			// Data has been received; ACK has been returned
			case VI2C_REGSTA_S_RXDATA_ACK:
                                u8Tmp = I2C_GET_DATA(psVI2CHw);
				*(psVI2CDataBuf->pu8RxBuf)= u8Tmp;
				psVI2CDataBuf->pu8RxBuf++;
				if( (psVI2CDataBuf->u16RxCount-=1) == 0 )
				{
					#if( VI2C_PROCESSEVENT )
					u32ProcessFlag = VI2C_STA_RXCOMPLETE;
					#else
					VI2C_ReceiveComplete(u8VI2CNo);
					#endif
				}
                                u8Tmp = (psVI2CDataBuf->u16RxCount)?I2C_AA:0;
				I2C_SET_CONTROL_REG(psVI2CHw, (I2C_SI|u8Tmp));
				break;
				
			// Slave mode status(Rx)
			// Switch to not addressed SLV mode.
			// Own SLA will be recognized.
			case VI2C_REGSTA_S_RXDATA_NACK:
				I2C_SET_CONTROL_REG(psVI2CHw, I2C_SI | I2C_AA);
				break;
				
			// Slave mode status(Tx)
			// Own SLA+R has been receive; ACK has been returned.
			case VI2C_REGSTA_S_SLAR_ACK:
			// Own SLA+R has been receive; ACK has been returned.
			case VI2C_REGSTA_S_LOST_SLAR_ACK:
				// Data buffer did not set to transmit.
				if(psVI2CDataBuf->u16TxCount == 0)
				{
					#if( VI2C_PROCESSEVENT )
					u32ProcessFlag = VI2C_STA_TXREQUEST;
					#else
					VI2C_SendRequest(u8VI2CNo);
					#endif
				}
			// Data has been transmitted; ACK has been return
			case VI2C_REGSTA_S_TXDATA_ACK:
				// Data will be transmitted; ACK will be received.
				if( psVI2CDataBuf->u16TxCount )
				{
					I2C_SET_DATA(psVI2CHw, *(psVI2CDataBuf->pu8TxBuf));
					psVI2CDataBuf->pu8TxBuf++;
					psVI2CDataBuf->u16TxCount--;
				}
                                u8Tmp = (psVI2CDataBuf->u16TxCount)?I2C_AA:0;
				I2C_SET_CONTROL_REG(psVI2CHw, (I2C_SI|u8Tmp));
				break;
				
			// Slave mode status(Tx)
			// Data byte or last data byte in I2CDAT has been transmitted
			// Not ACK has been received
			case VI2C_REGSTA_S_TXDATA_NACK:
			// Last data byte has been transmitted.
			// Ack has been received.
			case VI2C_REGSTA_S_TXLDATA_ACK:
			// A STOP or repeated START has been received while still addressed as SLV/REC
			case 0xA0:
				psVI2CDataBuf->u16TxCount = 0;
				I2C_SET_CONTROL_REG(psVI2CHw, I2C_SI | I2C_AA);
				break;

			// Master mode status(Tx & Rx)
			// START has been transmitted 
			case VI2C_REGSTA_M_START:
				// SLA+W to Register I2CDAT
                                u8Tmp =(psVI2CDataBuf->u16TxCount)?0:1;
				I2C_SET_DATA(psVI2CHw, (psVI2CCtrl->u8DeviceAddr<<1)+u8Tmp);
				I2C_SET_CONTROL_REG(psVI2CHw, I2C_SI);
				break;
			
			// Master mode status(Tx)
			// SLA+W has been transmitted and ACK has been received
			case VI2C_REGSTA_M_SLAW_ACK:
			// DATA has been transmitted and ACK has been received
			case VI2C_REGSTA_M_TXDATA_ACK:				
				// Data byte will be transmitted and ACK will be received.
				if( psVI2CDataBuf->u16TxCount )
				{
					I2C_SET_DATA(psVI2CHw, (UINT8)(*(psVI2CDataBuf->pu8TxBuf)));
					psVI2CDataBuf->pu8TxBuf++;
					psVI2CDataBuf->u16TxCount--;					
					I2C_SET_CONTROL_REG(psVI2CHw, I2C_SI);	
				}
				else
				{
					#if( VI2C_PROCESSEVENT )
					u32ProcessFlag = VI2C_STA_TXCOMPLETE;
					#else
					VI2C_SendComplete(u8VI2CNo);
					#endif
                                        u8Tmp = (psVI2CDataBuf->u16RxCount||psVI2CDataBuf->u16TxCount)?I2C_STA:0;
					I2C_SET_CONTROL_REG(psVI2CHw, (I2C_SI|I2C_STO|u8Tmp));		
				}
				break;
				
			// Master mode status(Tx)
			// SLA+W has been transmitted and NACK has been received
			case VI2C_REGSTA_M_SLAW_NACK:
			// Master mode status(Rx)
			// SLA+R has been transmitted and NACK has been returned.
			case VI2C_REGSTA_M_SLAR_NACK:					
				// A STOP followed by a START will be transmitted.
				I2C_SET_CONTROL_REG(psVI2CHw, I2C_STA | I2C_STO | I2C_SI);
				break;

			// Master mode status(Tx)				
			// DATA has been transmitted and NACK has been received
			case VI2C_REGSTA_M_TXDATA_NACK:
				psVI2CDataBuf->u16TxCount = 0;
				#if( VI2C_PROCESSEVENT )
				u32ProcessFlag = VI2C_STA_TXCOMPLETE;
				#else
				VI2C_SendComplete(u8VI2CNo);
				#endif	
                                u8Tmp = (psVI2CDataBuf->u16RxCount||psVI2CDataBuf->u16TxCount)?I2C_STA:0;
				I2C_SET_CONTROL_REG(psVI2CHw, (I2C_SI|I2C_STO|u8Tmp));		
				break;

			// Master mode status(Rx)	
			// DATA has been received and ACK has been returned
			case VI2C_REGSTA_M_RXDATA_ACK:
				// Data will be received, and ACK will be returned or NACK will be returned for STOP
                                u8Tmp = I2C_GET_DATA(psVI2CHw);
				*(psVI2CDataBuf->pu8RxBuf)= u8Tmp;
				psVI2CDataBuf->pu8RxBuf++;
				psVI2CDataBuf->u16RxCount--;
			// SLA+R has been transmitted and ACK has been received
			case VI2C_REGSTA_M_SLAR_ACK:
                                u8Tmp = (psVI2CDataBuf->u16RxCount)?I2C_AA:0;
				I2C_SET_CONTROL_REG(psVI2CHw, (I2C_SI|u8Tmp));
				break;
				
			// DATA has been received and NACK has been returned
			case VI2C_REGSTA_M_RXDATA_NACK:
				// A STOP will be transmitted, STO flag will be reset
                                u8Tmp = I2C_GET_DATA(psVI2CHw);
				*(psVI2CDataBuf->pu8RxBuf)= u8Tmp;
				psVI2CDataBuf->u16RxCount = 0;
				#if( VI2C_PROCESSEVENT )
				u32ProcessFlag = VI2C_STA_RXCOMPLETE;
				#else
				VI2C_ReceiveComplete(u8VI2CNo);
				#endif
                                u8Tmp = (psVI2CDataBuf->u16RxCount||psVI2CDataBuf->u16TxCount)?I2C_STA:0;
				I2C_SET_CONTROL_REG(psVI2CHw, (I2C_SI|I2C_STO|u8Tmp));
				break;
		}
	}
	return u32ProcessFlag;
}

__weak void VI2C_TimeOut(UINT8 u8VI2CNo)
{
};

__weak void VI2C_SendComplete(UINT8 u8VI2CNo)
{
};

__weak void VI2C_ReceiveComplete(UINT8 u8VI2CNo)
{	
};

__weak void VI2C_SendRequest(UINT8 u8VI2CNo)
{
};

__weak void VI2C_ReceiveRequest(UINT8 u8VI2CNo)
{	
};

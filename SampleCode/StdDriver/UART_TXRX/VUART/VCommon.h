#ifndef _VCOMMON_H_
#define _VCOMMON_H_

#include "Platform.h"

/* Hardware information */
typedef struct
{
	void      *pHwAddr;
	IRQn_Type eHwIRQn;
	UINT32    u32ModuleID;
	UINT32    u32ResetID;
	UINT8     u8ClokcPos;
}S_VHW_INFO;

#endif


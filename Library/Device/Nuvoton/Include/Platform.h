/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef __I91200BS__
#include "..\I91200\Include\I91200BS.h"		        /* H/W SFR definition */
#else
#include "..\I91200\Include\I91200.h"	
#endif

#define	__NUVOTON__									/* Nuvoton platform */

#include "..\I91200\Include\NVTTypes.h"				/* Nuvoton type definition */

// Audio process unit pcm bits
#define APU_PCM_BITS  (16)
// Audio process unit pcm bits
#define ADC_PCM_BITS  (16)


#endif /* __PLATFORM_H__ */

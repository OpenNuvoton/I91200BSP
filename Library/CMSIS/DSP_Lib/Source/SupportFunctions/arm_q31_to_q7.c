/* ----------------------------------------------------------------------------    
* Copyright (C) 2010 ARM Limited. All rights reserved.    
*    
* $Date: 13/10/11 2:55p $Revision: 	V1.1.0  
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_q31_to_q7.c    
*    
* Description:	Converts the elements of the Q31 vector to Q7 vector.    
*    
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*  
* Version 1.1.0 2012/02/15 
*    Updated with more optimizations, bug fixes and minor API changes.  
*   
* Version 1.0.10 2011/7/15  
*    Big Endian support added and Merged M0 and M3/M4 Source code.   
*    
* Version 1.0.3 2010/11/29   
*    Re-organized the CMSIS folders and updated documentation.    
*     
* Version 1.0.2 2010/11/11    
*    Documentation updated.     
*    
* Version 1.0.1 2010/10/05     
*    Production release and review comments incorporated.    
*    
* Version 1.0.0 2010/09/20     
*    Production release and review comments incorporated.    
* ---------------------------------------------------------------------------- */

#include "arm_math.h"

/**    
 * @ingroup groupSupport    
 */

/**    
 * @addtogroup q31_to_x    
 * @{    
 */

/**    
 * @brief Converts the elements of the Q31 vector to Q7 vector.    
 * @param[in]       *pSrc points to the Q31 input vector    
 * @param[out]      *pDst points to the Q7 output vector   
 * @param[in]       blockSize length of the input vector    
 * @return none.    
 *    
 * \par Description:    
 *    
 * The equation used for the conversion process is:    
 *   
 * <pre>    
 * 	pDst[n] = (q7_t) pSrc[n] >> 24;   0 <= n < blockSize.     
 * </pre>    
 *   
 */


void arm_q31_to_q7(
  q31_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  q31_t *pIn = pSrc;                             /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#ifndef ARM_MATH_CM0

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t in1, in2, in3, in4;
  q7_t out1, out2, out3, out4;

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.    
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = (q7_t) A >> 24 */
    /* convert from q31 to q7 and then store the results in the destination buffer */
    in1 = *pIn++;
    in2 = *pIn++;
    in3 = *pIn++;
    in4 = *pIn++;

    out1 = (q7_t) (in1 >> 24);
    out2 = (q7_t) (in2 >> 24);
    out3 = (q7_t) (in3 >> 24);
    out4 = (q7_t) (in4 >> 24);

    *__SIMD32(pDst)++ = __PACKq7(out1, out2, out3, out4);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.    
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

#else

  /* Run the below code for Cortex-M0 */

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

#endif /* #ifndef ARM_MATH_CM0 */

  while(blkCnt > 0u)
  {
    /* C = (q7_t) A >> 24 */
    /* convert from q31 to q7 and then store the results in the destination buffer */
    *pDst++ = (q7_t) (*pIn++ >> 24);

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**    
 * @} end of q31_to_x group    
 */

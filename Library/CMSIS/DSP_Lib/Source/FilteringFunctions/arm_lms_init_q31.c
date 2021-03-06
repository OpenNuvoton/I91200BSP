/*-----------------------------------------------------------------------------    
* Copyright (C) 2010 ARM Limited. All rights reserved.    
*    
* $Date: 13/10/11 2:55p $Revision: 	V1.1.0  
*    
* Project: 	    CMSIS DSP Library    
* Title:        arm_lms_init_q31.c    
*    
* Description:  Q31 LMS filter initialization function.    
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
*    Production release and review comments incorporated    
*    
* Version 0.0.7  2010/06/10     
*    Misra-C changes done    
* ---------------------------------------------------------------------------*/

#include "arm_math.h"

/**    
 * @ingroup groupFilters    
 */

/**    
 * @addtogroup LMS    
 * @{    
 */

  /**    
   * @brief Initialization function for Q31 LMS filter.    
   * @param[in] *S points to an instance of the Q31 LMS filter structure.    
   * @param[in] numTaps  number of filter coefficients.    
   * @param[in] *pCoeffs points to coefficient buffer.    
   * @param[in] *pState points to state buffer.    
   * @param[in] mu step size that controls filter coefficient updates.    
   * @param[in] blockSize number of samples to process.    
   * @param[in] postShift bit shift applied to coefficients.    
   * @return none.    
 *    
 * \par Description:    
 * <code>pCoeffs</code> points to the array of filter coefficients stored in time reversed order:    
 * <pre>    
 *    {b[numTaps-1], b[numTaps-2], b[N-2], ..., b[1], b[0]}    
 * </pre>    
 * The initial filter coefficients serve as a starting point for the adaptive filter.    
 * <code>pState</code> points to an array of length <code>numTaps+blockSize-1</code> samples,    
 * where <code>blockSize</code> is the number of input samples processed by each call to    
 * <code>arm_lms_q31()</code>.    
 */

void arm_lms_init_q31(
  arm_lms_instance_q31 * S,
  uint16_t numTaps,
  q31_t * pCoeffs,
  q31_t * pState,
  q31_t mu,
  uint32_t blockSize,
  uint32_t postShift)
{
  /* Assign filter taps */
  S->numTaps = numTaps;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and size is always blockSize + numTaps - 1 */
  memset(pState, 0, ((uint32_t) numTaps + (blockSize - 1u)) * sizeof(q31_t));

  /* Assign state pointer */
  S->pState = pState;

  /* Assign Step size value */
  S->mu = mu;

  /* Assign postShift value to be applied */
  S->postShift = postShift;

}

/**    
 * @} end of LMS group    
 */

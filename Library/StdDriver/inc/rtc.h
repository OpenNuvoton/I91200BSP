/**************************************************************************//**
 * @file     rtc.h
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 16/08/01 5:06p $
 * @brief    I91200 RTC driver header file
 *
 * @note
 * Copyright (C) Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __RTC_H
#define __RTC_H

#ifdef  __cplusplus
extern "C"
{
#endif


/** @addtogroup I91200_Device_Driver I91200 Device Driver
  @{
*/

/** @addtogroup I91200_RTC_Driver RTC Driver
  @{
*/


/** @addtogroup I91200_RTC_EXPORTED_CONSTANTS RTC Exported Constants
  @{
*/


#define RTC_INIT_KEY         0xA5EB1357UL     /*!< RTC Access Key   \hideinitializer */
#define RTC_WRITE_KEY        0xA965           /*!< RTC Access Key  \hideinitializer */

#define RTC_WAIT_COUNT       0xFFFFFFFF       /*!< Initial Time Out Value  \hideinitializer */

#define RTC_YEAR2000         2000             /*!< RTC Reference \hideinitializer */
#define RTC_FCR_REFERENCE    32761            /*!< RTC Reference \hideinitializer */

#define RTC_CLOCK_12         0                /*!< RTC 12 Hour \hideinitializer */
#define RTC_CLOCK_24         1                /*!< RTC 24 Hour \hideinitializer */

#define RTC_AM               1                /*!< RTC AM \hideinitializer */
#define RTC_PM               2                /*!< RTC PM \hideinitializer */

#define RTC_TICK_1_SEC       ((uint32_t) 0x00000000)   /*!< Time tick is 1 second \hideinitializer */
#define RTC_TICK_1_2_SEC     ((uint32_t) 0x00000001)   /*!< Time tick is 1/2 second \hideinitializer */
#define RTC_TICK_1_4_SEC     ((uint32_t) 0x00000002)   /*!< Time tick is 1/4 second \hideinitializer */
#define RTC_TICK_1_8_SEC     ((uint32_t) 0x00000003)   /*!< Time tick is 1/8 second \hideinitializer */
#define RTC_TICK_1_16_SEC    ((uint32_t) 0x00000004)   /*!< Time tick is 1/16 second \hideinitializer */
#define RTC_TICK_1_32_SEC    ((uint32_t) 0x00000005)   /*!< Time tick is 1/32 second \hideinitializer */
#define RTC_TICK_1_64_SEC    ((uint32_t) 0x00000006)   /*!< Time tick is 1/64 second \hideinitializer */
#define RTC_TICK_1_128_SEC   ((uint32_t) 0x00000007)   /*!< Time tick is 1/128 second \hideinitializer */

#define RTC_SUNDAY           ((uint32_t) 0x00000000)
#define RTC_MONDAY           ((uint32_t) 0x00000001)
#define RTC_TUESDAY          ((uint32_t) 0x00000002)
#define RTC_WEDNESDAY        ((uint32_t) 0x00000003)
#define RTC_THURSDAY         ((uint32_t) 0x00000004)
#define RTC_FRIDAY           ((uint32_t) 0x00000005)
#define RTC_SATURDAY         ((uint32_t) 0x00000006)

/*@}*/ /* end of group I91200_RTC_EXPORTED_CONSTANTS */


/** @addtogroup I91200_RTC_EXPORTED_STRUCTS RTC Exported Structs
  @{
*/

/**
  * @brief  RTC define Time Data Struct
  */
typedef struct {
    uint32_t u32Year;          /*!<  Year value */
    uint32_t u32Month;         /*!<  Month value */
    uint32_t u32Day;           /*!<  Day value */
    uint32_t u32DayOfWeek;     /*!<  Day of week value */
    uint32_t u32Hour;          /*!<  Hour value */
    uint32_t u32Minute;        /*!<  Minute value */
    uint32_t u32Second;        /*!<  Second value */
    uint32_t u32TimeScale;     /*!<  12-Hour, 24-Hour */
    uint32_t u32AmPm;          /*!<  Only Time Scale select 12-hr used */
} S_RTC_TIME_DATA_T;

/*@}*/ /* end of group I91200_RTC_EXPORTED_STRUCTS */

/** @addtogroup I91200_RTC_EXPORTED_FUNCTIONS RTC Exported Functions
  @{
*/

/**
 *  @brief    Get RTC Active Status.
 *
 *  @param    None
 *
 *  @return   RTC Active Status.
 * \hideinitializer 
 */
#define RTC_GET_ACTIVE_STATE    (RTC->INIT & RTC_INIT_ATVSTS_Msk)

/**
 *  @brief    According to current time, return this year is leap year or not
 *
 *  @param    None
 *
 *  @return   0 = This year is not a leap year. \n
 *            1 = This year is a leap year.
 * \hideinitializer 
 */
#define RTC_IS_LEAP_YEAR    ((RTC->LEAPYEAR & (RTC_LEAPYEAR_LEAPYEAR_Msk))?1:0)

/**
 *  @brief    Clear alarm interrupt status.
 *
 *  @param    None
 *
 *  @return   None
 * \hideinitializer 
 */
#define RTC_CLEAR_ALARM_INT_FLAG    (RTC->INTSTS = RTC_INTSTS_ALMIF_Msk)

/**
 *  @brief    Clear tick interrupt status.
 *
 *  @param    None
 *
 *  @return    None
 * \hideinitializer 
 */
#define RTC_CLEAR_TICK_INT_FLAG    (RTC->INTSTS = RTC_INTSTS_TICKIF_Msk)

/**
 *  @brief    Get alarm interrupt status.
 *
 *  @param    None
 *
 *  @return   Alarm interrupt status
 * \hideinitializer 
 */
#define RTC_GET_ALARM_INT_FLAG    ((RTC->INTSTS & RTC_INTSTS_ALMIF_Msk) >> RTC_INTSTS_ALMIF_Pos)

/**
 *  @brief    Get tick interrupt status.
 *
 *  @param    None
 *
 *  @return   tick interrupt status
 * \hideinitializer 
 */
#define RTC_GET_TICK_INT_FLAG    ((RTC->INTSTS & RTC_INTSTS_TICKIF_Msk) >> RTC_INTSTS_TICKIF_Pos)

/**
 *  @brief    Enable tick wakeup function.
 *
 *  @param    None
 *
 *  @return   None
 * \hideinitializer 
 */
#define RTC_ENABLE_TICK_WAKEUP    ((RTC->TICK) |= RTC_TICK_TWKEN_Msk)

/**
 *  @brief    Disable tick wakeup function.
 *
 *  @param    None
 *
 *  @return   None
 * \hideinitializer 
 */
#define RTC_DISABLE_TICK_WAKEUP    ((RTC->TICK) &= ~RTC_TICK_TWKEN_Msk)

void RTC_Open(S_RTC_TIME_DATA_T *sPt);
void RTC_Close(void);

void RTC_32KCalibration(int32_t i32FrequencyX100);
void RTC_SetTickPeriod(uint32_t u32TickSelection);

void RTC_EnableInt(uint32_t u32IntFlagMask);
void RTC_DisableInt(uint32_t u32IntFlagMask);

void RTC_SetAlarmTime(uint32_t u32Hour, uint32_t u32Minute, uint32_t u32Second, uint32_t u32TimeMode, uint32_t u32AmPm);
void RTC_SetAlarmDate(uint32_t u32Year, uint32_t u32Month, uint32_t u32Day);
void RTC_SetTime(uint32_t u32Hour, uint32_t u32Minute, uint32_t u32Second, uint32_t u32TimeMode, uint32_t u32AmPm);
void RTC_SetDate(uint32_t u32Year, uint32_t u32Month, uint32_t u32Day, uint32_t u32DayOfWeek);
void RTC_SetAlarmDateAndTime(S_RTC_TIME_DATA_T *sPt);
void RTC_SetDateAndTime(S_RTC_TIME_DATA_T *sPt);

uint32_t RTC_GetDayOfWeek(void);
void RTC_GetAlarmDateAndTime(S_RTC_TIME_DATA_T *sPt);
void RTC_GetDateAndTime(S_RTC_TIME_DATA_T *sPt);

void RTC_EnableWakeUp(void);
void RTC_DisableWakeUp(void);

/*@}*/ /* end of group I91200_RTC_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group I91200_RTC_Driver */

/*@}*/ /* end of group I91200_Device_Driver */


#ifdef  __cplusplus
}
#endif

#endif /* __RTC_H */


/*** (C) COPYRIGHT Nuvoton Technology Corp. ***/

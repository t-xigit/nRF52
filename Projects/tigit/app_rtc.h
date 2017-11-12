/**@file app_rtc.h
 *
 * @brief RTC MODULE variables and funcions
 *
 * @addtogroup group_rtc
 *  this will also be in the RTC module documentaion.
 *  @{
 */

#ifndef SDK_M_RTC_H__
#define SDK_M_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include "sdk_errors.h"

extern time_t unix_time;
ret_code_t rtc_init_task(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // SDK_COMMON_H__


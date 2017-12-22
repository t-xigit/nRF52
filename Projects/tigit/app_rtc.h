/**
 *
 * @addtogroup group_rtc RTC Module
 * @ingroup group_app
 * @brief RTC APIs.
 * @details The RTC Module is for following porpuses:
 * - Running a high accuracy LOW Power Clock
 *
 *
 *
 * @defgroup group_rtc RTC driver
 * @{
 * @ingroup group_rtc
 * @brief APIs for using the RRTC Module
 */ 

#ifndef SDK_M_RTC_H__
#define SDK_M_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include "sdk_errors.h"

extern time_t unix_time;
extern SemaphoreHandle_t m_rtc_semaphore;
ret_code_t rtc_init_task(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // SDK_COMMON_H__


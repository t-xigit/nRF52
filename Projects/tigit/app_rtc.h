/**
 *
 * @addtogroup group_rtc RTC Module
 * @ingroup group_app
 * @brief RTC APIs.
 * @details The clock HAL provides basic APIs for accessing the registers of the clock.
 * The clock driver provides APIs on a higher level.
 *
 * @defgroup group_rtc RTC driver
 * @{
 * @ingroup group_rtc
 * @brief Driver for managing the low-frequency clock (LFCLK) and the high-frequency clock (HFCLK).
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


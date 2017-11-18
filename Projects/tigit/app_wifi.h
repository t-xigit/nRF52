/**
 *
 * @addtogroup group_wifi WIFI Module
 * @ingroup group_app
 * @brief WIFI APIs.
 * @details The clock HAL provides basic APIs for accessing the registers of the clock.
 * The clock driver provides APIs on a higher level.
 *
 * @defgroup app_wifi_drv WIFI driver
 * @{
 * @ingroup group_wifi
 * @brief Driver for managing the low-frequency clock (LFCLK) and the high-frequency clock (HFCLK).
 */
 
 

#ifndef SDK_APP_WIFI_H__
#define SDK_APP_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_errors.h"
#include "m2m_wifi.h"

#define wifi_req_curr_rssi()   m2m_wifi_req_curr_rssi()

ret_code_t wifi_start_task(void);


/** @} */


#ifdef __cplusplus
}
#endif

#endif // SDK_COMMON_H__


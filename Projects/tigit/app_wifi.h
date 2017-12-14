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

extern struct sockaddr_in resolved_addr;
//Semaphores
extern SemaphoreHandle_t app_wifi_sys_t_Sema;
extern SemaphoreHandle_t app_dns_Sema;
//Queues
extern QueueHandle_t socket_snd_Q;
extern QueueHandle_t socket_rx_Q;

#define wifi_req_curr_rssi()   m2m_wifi_req_curr_rssi()

ret_code_t wifi_start_task(void);
void wifi_turn_off(void);


/** @} */


#ifdef __cplusplus
}
#endif

#endif // SDK_COMMON_H__


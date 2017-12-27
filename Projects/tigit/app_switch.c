/** @file app_switch.c
 *
 * @brief Push Button MODULE FILE
 *
 * This	file contains the source code for the rtc module
 *
 * @addtogroup group_switch
 *  Documentation of the Button Module.
 *	Explanataion of the assigned button functions.
 *  @{
 */

/* Standard includes. */
#include <stdbool.h>
#include <stdint.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

/* Application includes. */
#include "app_config.h"
#include "app_mqtt.h"
#include "app_wifi.h"

#define NRF_LOG_MODULE_NAME app_switch

#if APP_SWITCH_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL APP_SWITCH_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR APP_SWITCH_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR APP_SWITCH_CONFIG_DEBUG_COLOR

#else  //APP_SWITCH_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif  //APP_SWITCH_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();


/**
 *@}
 **/
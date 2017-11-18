/** @file app_mqtt.c
 *  
 * @brief MQTT MODULE FILE
 *
 * This	file contains the source code for the rtc module
 *
 * @addtogroup group_mqtt
 *  this source will also be in the RTC module documentaion.
 *  @{
 */

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "app_config.h"
#include "app_mqtt.h"

#define NRF_LOG_MODULE_NAME app_mqtt

#if APP_MQTT_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       APP_RTC_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  APP_RTC_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR APP_RTC_CONFIG_DEBUG_COLOR

#else //APP_RTC_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif //APP_RTC_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define	TASK_DELAY 600	      /**< Task	delay. Delays a	LED0 task for 200 ms */
#define	TIMER_PERIOD 500      /**< Timer period. LED1 timer will expire	after 1000 ms */
#define	RTC_PERIOD 1000000ULL /**< Timer period. LED1 timer will expire	after 1000 ms */

/**
 * @brief RTC instance number used for blinking
 *
 */
#define	BLINK_RTC 2


/**@brief RTC task handle function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
void mqtt_test(void){
   
 }

/**
 *@}
 **/
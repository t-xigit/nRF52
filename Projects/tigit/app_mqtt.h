/**
 *
 * @addtogroup group_mqtt MQTT Module
 * @ingroup group_app
 * @brief MQTT APIs.
 * @details The clock HAL provides basic APIs for accessing the registers of the clock.
 * The clock driver provides APIs on a higher level.
 *
 * @defgroup group_mqtt MQTT stuff
 * @{
 * @ingroup group_mqtt
 * @brief Driver for managing the low-frequency clock (LFCLK) and the high-frequency clock (HFCLK).
 */ 

#ifndef __APP_MQTT_
#define __APP_MQTT_

#ifdef __cplusplus
extern "C" {
#endif

int mqtt_start_task(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // SDK_COMMON_H__
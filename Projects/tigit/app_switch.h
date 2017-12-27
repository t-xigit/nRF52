/**
 *
 * @addtogroup group_mqtt MQTT Module
 * @ingroup group_app
 * @brief Push Button APIs.
 * @details APIs for accessing the Push Button Module.
 *
 * @defgroup group_switch Push Button Functions
 * @{
 * @ingroup group_switch
 * @brief Driver for managing the Push Buttons.
 */

#ifndef __APP_SWITCH_
#define __APP_SWITCH_

#ifdef __cplusplus
extern "C" {
#endif



extern TaskHandle_t button_task_handle; /**< Reference to button task. */


/**@brief Button task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
void button_task_function(void* pvParameter);


/** @} */

#ifdef __cplusplus
}
#endif

#endif  // SDK_COMMON_H__
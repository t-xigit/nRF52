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

/* Nordic Includes */
#include "app_error.h"
#include "boards.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"

/* Nordic Driver Includes */
#include "nrf_drv_gpiote.h"

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

TaskHandle_t		button_task_handle;
SemaphoreHandle_t	m_switch_semaphore;

#ifdef BSP_BUTTON_0
    #define PIN_IN BSP_BUTTON_0
#endif
#ifndef PIN_IN
    #error "Please indicate input pin"
#endif

#ifdef BSP_LED_0
    #define PIN_OUT BSP_LED_0
#endif
#ifndef PIN_OUT
    #error "Please indicate output pin"
#endif


/**
 * @brief Semaphore set	in Button event
 */
static void switch_int_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	BaseType_t yield_req = pdFALSE;
	ret_code_t err_code;

	NRF_LOG_DEBUG("BUTTON INTERUPT");
	/* The returned value may be safely	ignored, if error is returned it only means that
	* the semaphore is already given (raised). */
	UNUSED_VARIABLE(xSemaphoreGiveFromISR(m_switch_semaphore, &yield_req));
	portYIELD_FROM_ISR(yield_req);
}

void button_task_function(void* pvParameter) {
#define BUTTON_TASK_DELAY 100  // 100ms
	UNUSED_PARAMETER(pvParameter);

    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(PIN_IN, &in_config, switch_int_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_IN, true);

	uint32_t button_1_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_0);
	uint32_t button_2_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_1);
	uint32_t button_3_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_2);
	uint32_t button_4_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_3);

   //ret_code_t err = nrf_drv_gpiote_in_init( button_1_idx, &gpiote_config, switch_int_handle());

    m_switch_semaphore = xSemaphoreCreateBinary();
	ASSERT(NULL != m_switch_semaphore);

	/* initializing buttons*/
	bsp_board_buttons_init();

	while (true) {
		/* Block to wait for next poll*/
		UNUSED_RETURN_VALUE(xSemaphoreTake(m_switch_semaphore, portMAX_DELAY));

		if (bsp_board_button_state_get(button_1_idx)) {
			NRF_LOG_INFO("BUTTON 1\n\r");
			NRF_LOG_INFO("wifi_req_curr_rssi\n\r");
			//wifi_req_curr_rssi();
			/* Block to debounce*/
			vTaskDelay(BUTTON_TASK_DELAY * 2);
		}

		if (bsp_board_button_state_get(button_2_idx)) {
			NRF_LOG_INFO("BUTTON 2\n\r");
			//wifi_turn_off();
			/* Block to debounce*/
			vTaskDelay(BUTTON_TASK_DELAY * 2);
		}

		if (bsp_board_button_state_get(button_3_idx)) {
			NRF_LOG_INFO("BUTTON 3\n\r");
			/* Block to debounce*/
			vTaskDelay(BUTTON_TASK_DELAY * 2);
		}

		if (bsp_board_button_state_get(button_4_idx)) {
			NRF_LOG_INFO("BUTTON 4");
			/* Block to debounce*/
            //xSemaphoreGive(app_button4_Sema);
			vTaskDelay(BUTTON_TASK_DELAY * 2);
		}
	}
}






/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT pin for output,
 * and configures GPIOTE to give an interrupt on pin change.
 */
static void gpio_init(void)
{

}

/**
 *@}
 **/
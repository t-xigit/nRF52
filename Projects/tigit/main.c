/** @file main.c
 *
 * @brief WINC1500 FreeRTOS Example Application main file.
 *
 * This file contains the source code for a sample application using FreeRTOS to do some wifi stuff.
 *
 * @mainpage Start Page
 *	Porpose is having a start point for Projects using the nRF52 SoC. With following tools:
 *  - Segger Embedded Studio
 *  - FreeRTOS v9
 *  - WINC1500
 *  - PAHO MQTT C-Embedded Client
 *
 * @defgroup group_app Application Modules
 *  Modules used by the Application
 */

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "app_error.h"
#include "boards.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "FreeRTOSConfig.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "SEGGER_SYSVIEW.h"

#include "app_rtc.h"
#include "app_wifi.h"
#include "app_mqtt.h"
#include "app_switch.h"

#if LEDS_NUMBER <= 2
#error "Board is not equipped with enough amount of LEDs"
#endif

#define TASK_DELAY 600   /**< Task delay. Delays a LED0 task for 200 ms */
#define TIMER_PERIOD 500 /**< Timer period. LED1 timer will expire after 1000 ms */

/**@brief Function for initializing the nrf log module. */
static void log_init(void) {
	ret_code_t err_code = NRF_LOG_INIT(NULL);
	APP_ERROR_CHECK(err_code);
	NRF_LOG_DEFAULT_BACKENDS_INIT();
	NRF_LOG_INFO("log_init()\n\r");

	NRF_LOG_INFO("SEGGER_SYSVIEW_Conf()\n\r");
	SEGGER_SYSVIEW_Conf(); /* Configure and initialize SystemView */
}

TaskHandle_t timer_task_handle; /**< Reference to LED0 toggling FreeRTOS task. */

/**@brief TimerTask task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void timer_task_function(void* pvParameter) {
	UNUSED_PARAMETER(pvParameter);
	while (true) {
		/* Block to wait for timer to notify this task. */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		bsp_board_led_invert(BSP_BOARD_LED_1);
		//NRF_LOG_DEBUG("TIMER TASK\n\r");
	}
}

TimerHandle_t led_toggle_timer_handle; /**< Reference to LED1 toggling FreeRTOS timer. */
/**@brief The function to call when the LED1 FreeRTOS timer expires.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the timer.
 */
static void led_toggle_timer_callback(void* pvParameter) {
	UNUSED_PARAMETER(pvParameter);
	/* Send a notification to prvTask1(), bringing it out of the Blocked state. */
	xTaskNotifyGive(timer_task_handle);
}

#if 1
int main(void) {
	ret_code_t err_code;

	log_init();

	/* Initialize clock driver for better time accuracy in FREERTOS */
	err_code = nrf_drv_clock_init();
	APP_ERROR_CHECK(err_code);

	/* Configure LED-pins as outputs */
	bsp_board_leds_init();

	err_code = rtc_init_task();
	APP_ERROR_CHECK(err_code);
#ifdef APP_CONF_ENABLE_WINC
    err_code = wifi_start_task();
	APP_ERROR_CHECK(err_code);
#endif

#ifdef APP_CONF_ENABLE_MQTT
	err_code = mqtt_start_task();
	APP_ERROR_CHECK(err_code);
#endif

	/* Create task for timer with priority set to 2 */
	UNUSED_VARIABLE(xTaskCreate(timer_task_function, "TIM", configMINIMAL_STACK_SIZE * 2, NULL, 2, &timer_task_handle));

	/* Create task for button handling with priority set to 2 */
	UNUSED_VARIABLE(xTaskCreate(button_task_function, "BUT", configMINIMAL_STACK_SIZE * 2, NULL, 2, &button_task_handle));

	led_toggle_timer_handle = xTimerCreate("LED1", TIMER_PERIOD, pdTRUE, NULL, led_toggle_timer_callback);
	/* Start timer for LED1 blinking */
	UNUSED_VARIABLE(xTimerStart(led_toggle_timer_handle, 0));

	/* Activate deep sleep mode */
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	/* Start FreeRTOS scheduler. */
	vTaskStartScheduler();

	while (true) {
		ASSERT(false);
		/* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
         * in vTaskStartScheduler function. */
	}
}
#endif

#if 0

int main(void) {
	ret_code_t err_code;

	log_init();

	/* Initialize clock driver for better time accuracy in FREERTOS */
	err_code = nrf_drv_clock_init();
	APP_ERROR_CHECK(err_code);

	/* Configure LED-pins as outputs */
	bsp_board_leds_init();

	/* Create task for button handling with priority set to 2 */
	UNUSED_VARIABLE(xTaskCreate(button_task_function, "BUT", configMINIMAL_STACK_SIZE * 2, NULL, 2, &button_task_handle));


	/* Activate deep sleep mode */
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	/* Start FreeRTOS scheduler. */
	vTaskStartScheduler();

	while (true) {
		ASSERT(false);
	}
}
#endif

/**
 *@}
 **/
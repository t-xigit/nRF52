/** @file
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example_freertos
 *
 * @brief Blinky FreeRTOS Example Application main file.
 *
 * This file contains the source code for a sample application using FreeRTOS to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "bsp.h"
#include "nordic_common.h"
#include "boards.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "SEGGER_SYSVIEW.h"

#include "nrf_drv_rtc.h"

#if LEDS_NUMBER <= 2
#error "Board is not equipped with enough amount of LEDs"
#endif

#define TASK_DELAY        800           /**< Task delay. Delays a LED0 task for 200 ms */
#define TIMER_PERIOD      500           /**< Timer period. LED1 timer will expire after 1000 ms */
#define RTC_PERIOD        1000000ULL    /**< Timer period. LED1 timer will expire after 1000 ms */
/**
 * @brief RTC instance number used for blinking
 *
 */
#define BLINK_RTC 2

/**
 * @brief RTC compare channel used
 *
 */
#define BLINK_RTC_CC 0

/**
 * @brief Number of RTC ticks between interrupts
 */
#define BLINK_RTC_TICKS   (RTC_US_TO_TICKS(RTC_PERIOD, RTC_DEFAULT_CONFIG_FREQUENCY))

/**@brief Function for initializing the nrf log module. */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("log_init()\n\r");

    NRF_LOG_INFO("SEGGER_SYSVIEW_Conf()\n\r");
    SEGGER_SYSVIEW_Conf(); /* Configure and initialize SystemView */

}

TaskHandle_t  timer_task_handle;   /**< Reference to LED0 toggling FreeRTOS task. */
/**@brief TimerTask task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void timer_task_function (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    while (true)
    {
        /* Block to wait for timer to notify this task. */
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

        bsp_board_led_invert(BSP_BOARD_LED_1);
        NRF_LOG_INFO("TIMER TASK\n\r");
    }
}

TaskHandle_t  button_task_handle;   /**< Reference to button task. */
/**@brief Button task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void button_task_function (void * pvParameter)
{
    #define BUTTON_TASK_DELAY 100 // 100ms
    UNUSED_PARAMETER(pvParameter);

    uint32_t button_1_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_0);
    uint32_t button_2_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_1);
    uint32_t button_3_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_2);
    uint32_t button_4_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_3);

    /* initializing buttons*/
    bsp_board_buttons_init();

    while (true)
    {
        /* Block to wait for next poll*/
        vTaskDelay(BUTTON_TASK_DELAY);

        if(bsp_board_button_state_get(button_1_idx))
        {
              NRF_LOG_INFO("BUTTON 1\n\r");
              /* Block to debounce*/
              vTaskDelay(BUTTON_TASK_DELAY*2);
        }
               
        if(bsp_board_button_state_get(button_2_idx))
        {
              NRF_LOG_INFO("BUTTON 2\n\r");
              /* Block to debounce*/
              vTaskDelay(BUTTON_TASK_DELAY*2);
        }

        if(bsp_board_button_state_get(button_3_idx))
        {
              NRF_LOG_INFO("BUTTON 3\n\r");
              /* Block to debounce*/
              vTaskDelay(BUTTON_TASK_DELAY*2);
        }

        if(bsp_board_button_state_get(button_4_idx))
        {
              NRF_LOG_INFO("BUTTON 4\n\r");
              /* Block to debounce*/
              vTaskDelay(BUTTON_TASK_DELAY*2);
        }
    }
}

TimerHandle_t led_toggle_timer_handle;  /**< Reference to LED1 toggling FreeRTOS timer. */
/**@brief The function to call when the LED1 FreeRTOS timer expires.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the timer.
 */
static void led_toggle_timer_callback (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    /* Send a notification to prvTask1(), bringing it out of the Blocked state. */
    xTaskNotifyGive( timer_task_handle );
    
}

/**
 * @brief Semaphore set in RTC event
 */
static SemaphoreHandle_t m_led_semaphore;

/**
 * @brief Function to convert Unix time into string and print it
 */
void print_time(time_t *unix_time_ms)
{
	#define PRINT_BUFFER_SIZE 80
	//const time_t timezone_delta = 60 * 60 * 2;
	struct tm asctime;
	time_t time;
	char print_buffer[PRINT_BUFFER_SIZE];

	uint64_t temp_time = *unix_time_ms;

	time = (time_t)(temp_time);
	//time = time + timezone_delta;

	asctime = *localtime(&time);

	strftime(print_buffer, sizeof(print_buffer), "%H:%M:%S %Y-%m-%d", &asctime);
        NRF_LOG_INFO("%s\n", print_buffer);
}

/**
 * @brief RTC configuration & instance
 */
static nrf_drv_rtc_config_t const m_rtc_config = NRF_DRV_RTC_DEFAULT_CONFIG;
static nrf_drv_rtc_t const m_rtc = NRF_DRV_RTC_INSTANCE(BLINK_RTC);

static void rtc_int_handler(nrf_drv_rtc_int_type_t int_type)
{
    BaseType_t yield_req = pdFALSE;
    ret_code_t err_code;
    bsp_board_led_invert(BSP_BOARD_LED_3);
    err_code = nrf_drv_rtc_cc_set(
        &m_rtc,
        BLINK_RTC_CC,
        (nrf_rtc_cc_get(m_rtc.p_reg, BLINK_RTC_CC) + BLINK_RTC_TICKS) & RTC_COUNTER_COUNTER_Msk,
        true);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("RTC INTERUPT\n\r");
   /* The returned value may be safely ignored, if error is returned it only means that
    * the semaphore is already given (raised). */
   UNUSED_VARIABLE(xSemaphoreGiveFromISR(m_led_semaphore, &yield_req));
   portYIELD_FROM_ISR(yield_req);
}

TaskHandle_t rtc_task_handle; /**< Reference to LED0 toggling FreeRTOS task. */
/**@brief LED0 task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void rtc_task_function (void * pvParameter)
{
    ret_code_t err_code;
    time_t unix_time = 0;
    UNUSED_PARAMETER(pvParameter);

    err_code = nrf_drv_rtc_init(&m_rtc, &m_rtc_config, rtc_int_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_rtc_cc_set(&m_rtc, BLINK_RTC_CC, BLINK_RTC_TICKS, true);
    APP_ERROR_CHECK(err_code);
    nrf_drv_rtc_enable(&m_rtc);

    m_led_semaphore = xSemaphoreCreateBinary();
    ASSERT(NULL != m_led_semaphore);

    while (true)
    {
        bsp_board_led_invert(BSP_BOARD_LED_0);
        NRF_LOG_INFO("RTC TASK\n\r");
        print_time(&unix_time);
        unix_time++;

        /* Delay a task for a given number of ticks */
        UNUSED_RETURN_VALUE(xSemaphoreTake(m_led_semaphore, portMAX_DELAY));
    }
}

int main(void)
{
    ret_code_t err_code;

    log_init();

    /* Initialize clock driver for better time accuracy in FREERTOS */
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    /* Configure LED-pins as outputs */
    bsp_board_leds_init();

    /* Create task for LED0 blinking with priority set to 2 */
    UNUSED_VARIABLE(xTaskCreate(rtc_task_function, "RTC", configMINIMAL_STACK_SIZE + 200, NULL, 2, &rtc_task_handle));

    /* Create task for timer with priority set to 2 */
    UNUSED_VARIABLE(xTaskCreate(timer_task_function, "TIM", configMINIMAL_STACK_SIZE + 200, NULL, 2, &timer_task_handle));

    /* Create task for button handling with priority set to 2 */
    UNUSED_VARIABLE(xTaskCreate(button_task_function, "BUT", configMINIMAL_STACK_SIZE + 200, NULL, 2, &button_task_handle));

    led_toggle_timer_handle = xTimerCreate( "LED1", TIMER_PERIOD, pdTRUE, NULL, led_toggle_timer_callback);
    /* Start timer for LED1 blinking */
    UNUSED_VARIABLE(xTimerStart(led_toggle_timer_handle, 0));

    /* Activate deep sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    while (true)
    {
       ASSERT(false);
        /* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
         * in vTaskStartScheduler function. */
    }
}

/**
 *@}
 **/
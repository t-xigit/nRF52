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

#include "bsp.h"
#include "nordic_common.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_rtc.h"

#define TASK_DELAY        600           /**< Task delay. Delays a LED0 task for 200 ms */
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

time_t unix_time = 0;

/**
 * @brief RTC configuration & instance
 */
static nrf_drv_rtc_config_t const m_rtc_config = NRF_DRV_RTC_DEFAULT_CONFIG;
static nrf_drv_rtc_t const m_rtc = NRF_DRV_RTC_INSTANCE(BLINK_RTC);

/**
 * @brief Semaphore set in RTC event
 */
static SemaphoreHandle_t m_rtc_semaphore;


/**
 * @brief Function to convert Unix time into string and print it
 */
void print_time(time_t *unix_time)
{
	#define PRINT_BUFFER_SIZE 80
	//const time_t timezone_delta = 60 * 60 * 2;
	struct tm asctime;
	time_t time;
	char print_buffer[PRINT_BUFFER_SIZE];

	uint64_t temp_time = *unix_time;

	time = (time_t)(temp_time);
	//time = time + timezone_delta;

	asctime = *gmtime(&time);

	strftime(print_buffer, sizeof(print_buffer), "%H:%M:%S %Y-%m-%d", &asctime);
        NRF_LOG_INFO("%s\n", print_buffer);
}

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
   UNUSED_VARIABLE(xSemaphoreGiveFromISR(m_rtc_semaphore, &yield_req));
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
    UNUSED_PARAMETER(pvParameter);

    err_code = nrf_drv_rtc_init(&m_rtc, &m_rtc_config, rtc_int_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_rtc_cc_set(&m_rtc, BLINK_RTC_CC, BLINK_RTC_TICKS, true);
    APP_ERROR_CHECK(err_code);
    nrf_drv_rtc_enable(&m_rtc);

    m_rtc_semaphore = xSemaphoreCreateBinary();
    ASSERT(NULL != m_rtc_semaphore);

    while (true)
    {
        bsp_board_led_invert(BSP_BOARD_LED_0);
        NRF_LOG_INFO("RTC TASK\n\r");
        if(unix_time)
        {
          print_time(&unix_time);
          unix_time++;          
        }                

        /* Delay a task for a given number of ticks */
        UNUSED_RETURN_VALUE(xSemaphoreTake(m_rtc_semaphore, portMAX_DELAY));
    }
}

ret_code_t rtc_init_task(void)
{
    ret_code_t err_code = NRF_SUCCESS;    

    /* Create task for LED0 blinking with priority set to 2 */
    err_code = (ret_code_t) xTaskCreate(rtc_task_function, "RTC", configMINIMAL_STACK_SIZE + 200, NULL, 2, &rtc_task_handle);
    if(err_code == pdPASS){
      NRF_LOG_INFO("RTC TASK CREATED");
      err_code = NRF_SUCCESS;
    }else{
      NRF_LOG_ERROR("RTC TASK CREATE ERROR");
      err_code = NRF_ERROR_NO_MEM;     
    }
	
    return err_code;
}

/**
 *@}
 **/
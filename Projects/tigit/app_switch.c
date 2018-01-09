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

TaskHandle_t button_task_handle;
SemaphoreHandle_t m_switch_semaphore;
QueueHandle_t button_Q; /**< Queue for messages to be published */

BaseType_t xHigherPriorityTaskWoken;

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

#if 0

static bsp_indication_t actual_state =  BSP_INDICATE_FIRST;         /**< Currently indicated state. */

static const char * indications_list[] = BSP_INDICATIONS_LIST;

/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    uint32_t err_code;
    switch (evt)
    {
        case BSP_EVENT_KEY_0:
            if (actual_state != BSP_INDICATE_FIRST)
                actual_state--;
            else
                actual_state = BSP_INDICATE_LAST;
            break;

        case BSP_EVENT_KEY_1:

            if (actual_state != BSP_INDICATE_LAST)
                actual_state++;
            else
                actual_state = BSP_INDICATE_FIRST;
            break;

        default:
            return; // no implementation needed
    }
    err_code = bsp_indication_set(actual_state);
    NRF_LOG_INFO("%s", (uint32_t)indications_list[actual_state]);
    APP_ERROR_CHECK(err_code);
}
#endif

#if 0
void vBufferISR( void )
{
char cIn;
BaseType_t xHigherPriorityTaskWoken;

    /* We have not woken a task at the start of the ISR. */
    xHigherPriorityTaskWoken = pdFALSE;

    /* Loop until the buffer is empty. */
    do
    {
        /* Obtain a byte from the buffer. */
        cIn = portINPUT_BYTE( RX_REGISTER_ADDRESS );

        /* Post the byte. */
        xQueueSendFromISR( xRxQueue, &cIn, &xHigherPriorityTaskWoken );

    } while( portINPUT_BYTE( BUFFER_COUNT ) );

    /* Now the buffer is empty we can switch context if necessary. */
    if( xHigherPriorityTaskWoken )
    {
        /* Actual macro used here is port specific. */
        taskYIELD_FROM_ISR ();
    }
}
#endif

/**
 * @brief Semaphore set	in Button event
 */
static void switch_int_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    BaseType_t xReturn;

    nrf_drv_gpiote_pin_t int_pin = pin;
    nrf_drv_gpiote_in_event_disable(int_pin);

    xReturn = xQueueSendFromISR(button_Q, &int_pin, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void button_task_function(void* pvParameter) {
#define BUTTON_TASK_DELAY 100  // 100ms
    UNUSED_PARAMETER(pvParameter);

    ret_code_t err_code;
    BaseType_t xReturn;
    nrf_drv_gpiote_pin_t int_pin;

    uint32_t count = 0;
    xHigherPriorityTaskWoken = pdFALSE;

    if (!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    in_config.hi_accuracy = true;
    in_config.is_watcher = true;

    err_code = nrf_drv_gpiote_in_init(BSP_BUTTON_0, &in_config, switch_int_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_gpiote_in_init(BSP_BUTTON_1, &in_config, switch_int_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_gpiote_in_init(BSP_BUTTON_2, &in_config, switch_int_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_gpiote_in_init(BSP_BUTTON_3, &in_config, switch_int_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(BSP_BUTTON_0, true);
    nrf_drv_gpiote_in_event_enable(BSP_BUTTON_1, true);
    nrf_drv_gpiote_in_event_enable(BSP_BUTTON_2, true);
    nrf_drv_gpiote_in_event_enable(BSP_BUTTON_3, true);

    uint32_t button_1_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_0);
    uint32_t button_2_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_1);
    uint32_t button_3_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_2);
    uint32_t button_4_idx = bsp_board_pin_to_button_idx(BSP_BUTTON_3);

    button_Q = xQueueCreate(5, sizeof(uint32_t));
    if (mqtt_publish_Q == NULL) {
        NRF_LOG_ERROR("xQueueCreate >>> ERROR >>> button_Q");
    }

    /* initializing buttons*/
    bsp_board_buttons_init();

    while (true) {
        int_pin = 0;
        /* Block to wait for next poll*/
        xReturn = xQueueReceive(button_Q, &int_pin, (TickType_t)portMAX_DELAY);
        if (xReturn == pdTRUE) {
            NRF_LOG_DEBUG("Button xQueueReceiveFromISR: %d", int_pin);
        } else
            NRF_LOG_ERROR("xQueueReceiveFromISR");

        switch (int_pin) {
            case BSP_BUTTON_0: {
                NRF_LOG_INFO("BUTTON 0");
                //NRF_LOG_INFO("wifi_req_curr_rssi\n\r");
                //wifi_req_curr_rssi();
                vTaskDelay(BUTTON_TASK_DELAY * 2);
                count++;
                NRF_LOG_DEBUG("COUNT BT 1: %d", count);

                break;
            }

            case BSP_BUTTON_1: {
                NRF_LOG_INFO("BUTTON 1");
                vTaskDelay(BUTTON_TASK_DELAY * 2);
                break;
            }

            case BSP_BUTTON_2: {
                NRF_LOG_INFO("BUTTON 2");
                vTaskDelay(BUTTON_TASK_DELAY * 2);
                app_MQTTPublishSendQueue(white, 1);
		 
                break;
            }

            case BSP_BUTTON_3: {
                NRF_LOG_INFO("BUTTON 3");
                //wifi_turn_off();
                vTaskDelay(BUTTON_TASK_DELAY * 2);
                app_MQTTPublishSendQueue(black, 1);
                break;
            }
        }

        nrf_drv_gpiote_in_event_enable(int_pin, true);
    }
}

#if 0
/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT pin for output,
 * and configures GPIOTE to give an interrupt on pin change.
 */
static void gpio_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

    err_code = nrf_drv_gpiote_out_init(PIN_OUT, &out_config);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(PIN_IN, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_IN, true);
}
#endif

/**
 *@}
 **/
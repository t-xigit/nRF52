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

 //http://www.atmel.com/Images/Atmel-42388-ATWINC1500-Xplained-Pro_UserGuide.pdf
 // page 7

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

#include "m2m_wifi.h"
#include "socket.h"

#if LEDS_NUMBER <= 2
#error "Board is not equipped with enough amount of LEDs"
#endif

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


// WIFI Stuff

/** Wi-Fi Settings */
#define MAIN_WLAN_SSID         "Schluesseldienst"
#define MAIN_WLAN_AUTH          M2M_WIFI_SEC_WPA_PSK
#define MAIN_WLAN_PSK          "4YGp7XL8BDEbkUwM"

/** PowerSave mode Settings */
#define MAIN_PS_SLEEP_MODE          M2M_PS_DEEP_AUTOMATIC /* M2M_NO_PS / M2M_PS_DEEP_AUTOMATIC / M2M_PS_MANUAL */

/** Using NTP server information */
#define MAIN_WORLDWIDE_NTP_POOL_HOSTNAME        "pool.ntp.org"
#define MAIN_ASIA_NTP_POOL_HOSTNAME             "asia.pool.ntp.org"
#define MAIN_EUROPE_NTP_POOL_HOSTNAME           "europe.pool.ntp.org"
#define MAIN_NAMERICA_NTP_POOL_HOSTNAME         "north-america.pool.ntp.org"
#define MAIN_OCEANIA_NTP_POOL_HOSTNAME          "oceania.pool.ntp.org"
#define MAIN_SAMERICA_NTP_POOL_HOSTNAME         "south-america.pool.ntp.org"
#define MAIN_SERVER_PORT_FOR_UDP                (123)
#define MAIN_DEFAULT_ADDRESS                    0xFFFFFFFF /* "255.255.255.255" */
#define MAIN_DEFAULT_PORT                       (6666)
#define MAIN_WIFI_M2M_BUFFER_SIZE               1460

/** Wi-Fi status variable. */
static bool gbConnectedWifi = false;
/** Host name placeholder. */
static char dns_server_address[HOSTNAME_MAX_SIZE];

/** UDP socket handlers. */
static SOCKET udp_socket = -1;

/** Receive buffer definition. */
static uint8_t gau8SocketBuffer[MAIN_WIFI_M2M_BUFFER_SIZE];

/** Wi-Fi Sleep status. */
static uint8 gu8SleepStatus;


tstrWifiInitParam param;
int8_t ret;

tstrSystemTime  *sys_time;
// WIFI Stuff

time_t unix_time = 0;

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
        if(unix_time)
        {
          print_time(&unix_time);
          unix_time++;
        }                

        /* Delay a task for a given number of ticks */
        UNUSED_RETURN_VALUE(xSemaphoreTake(m_led_semaphore, portMAX_DELAY));
    }
}

/**
 * \brief Callback to get the Wi-Fi status update.
 *
 * \param[in] u8MsgType Type of Wi-Fi notification.
 * \param[in] pvMsg A pointer to a buffer containing the notification parameters.
 */
static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
	switch (u8MsgType) {
	case M2M_WIFI_RESP_CON_STATE_CHANGED:
	{
		tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
		if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
			printf("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED\r\n");
			m2m_wifi_request_dhcp_client();
		} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
			printf("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED\r\n");
			gbConnectedWifi = false;
			m2m_wifi_connect((char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID),
					MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
		}

		break;
	}

	case M2M_WIFI_REQ_DHCP_CONF:
	{
		uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
		/* Turn LED0 on to declare that IP address received. */
		printf("wifi_cb: M2M_WIFI_REQ_DHCP_CONF: IP is %u.%u.%u.%u\r\n",
				pu8IPAddress[0], pu8IPAddress[1], pu8IPAddress[2], pu8IPAddress[3]);
		gbConnectedWifi = true;
		memcpy(dns_server_address, (uint8_t *)MAIN_WORLDWIDE_NTP_POOL_HOSTNAME, strlen(MAIN_WORLDWIDE_NTP_POOL_HOSTNAME));
		/* Obtain the IP Address by network name */
		gethostbyname((uint8_t *)dns_server_address);
		break;
	}

        case M2M_WIFI_RESP_GET_SYS_TIME:

		NRF_LOG_INFO("M2M_WIFI_RESP_GET_SYS_TIME: ");

                struct tm time_struct;                
		sys_time = (tstrSystemTime *)pvMsg;

                time_struct.tm_sec = (int)sys_time->u8Second;
                time_struct.tm_min = (int)sys_time->u8Minute;
                time_struct.tm_hour = (int)sys_time->u8Hour;
                time_struct.tm_mday = (int)sys_time->u8Day;
                time_struct.tm_mon = ((int)sys_time->u8Month) -1;              
                time_struct.tm_year = ((int)sys_time->u16Year)-1900;         
               
                unix_time = mktime(&time_struct);
             
                NRF_LOG_INFO("%s\n\r",ctime(&unix_time));

	break;

	default:
	{
		break;
	}
	}
}

/**
 * \brief Callback to get the ServerIP from DNS lookup.
 *
 * \param[in] pu8DomainName Domain name.
 * \param[in] u32ServerIP Server IP.
 */
static void resolve_cb(uint8_t *pu8DomainName, uint32_t u32ServerIP)
{
	struct sockaddr_in addr;
	int8_t cDataBuf[48];
	int16_t ret;

	memset(cDataBuf, 0, sizeof(cDataBuf));
	cDataBuf[0] = '\x1b'; /* time query */

	printf("resolve_cb: DomainName %s\r\n", pu8DomainName);

	if (udp_socket >= 0) {
		/* Set NTP server socket address structure. */
		addr.sin_family = AF_INET;
		addr.sin_port = _htons(MAIN_SERVER_PORT_FOR_UDP);
		addr.sin_addr.s_addr = u32ServerIP;

		/*Send an NTP time query to the NTP server*/
		ret = sendto(udp_socket, (int8_t *)&cDataBuf, sizeof(cDataBuf), 0, (struct sockaddr *)&addr, sizeof(addr));
		if (ret != M2M_SUCCESS) {
			printf("resolve_cb: failed to send  error!\r\n");
			return;
		}
	}

        m2m_wifi_get_sytem_time();
}

/**
 * \brief Callback to get the Data from socket.
 *
 * \param[in] sock socket handler.
 * \param[in] u8Msg Type of Socket notification.
 * \param[in] pvMsg A structure contains notification informations.
 */
static void  socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
{
	/* Check for socket event on socket. */
	int16_t ret;

	switch (u8Msg) {
	case SOCKET_MSG_BIND:
	{
		/* printf("socket_cb: socket_msg_bind!\r\n"); */
		tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg *)pvMsg;
		if (pstrBind && pstrBind->status == 0) {
			ret = recvfrom(sock, gau8SocketBuffer, MAIN_WIFI_M2M_BUFFER_SIZE, 0);
			if (ret != SOCK_ERR_NO_ERROR) {
				printf("socket_cb: recv error!\r\n");
			}
		} else {
			printf("socket_cb: bind error!\r\n");
		}

		break;
	}

	case SOCKET_MSG_RECVFROM:
	{
		/* printf("socket_cb: socket_msg_recvfrom!\r\n"); */
		tstrSocketRecvMsg *pstrRx = (tstrSocketRecvMsg *)pvMsg;
		if (pstrRx->pu8Buffer && pstrRx->s16BufferSize) {
			uint8_t packetBuffer[48];
			memcpy(&packetBuffer, pstrRx->pu8Buffer, sizeof(packetBuffer));

			if ((packetBuffer[0] & 0x7) != 4) {                   /* expect only server response */
				printf("socket_cb: Expecting response from Server Only!\r\n");
				return;                    /* MODE is not server, abort */
			} else {
				uint32_t secsSince1900 = packetBuffer[40] << 24 |
						packetBuffer[41] << 16 |
						packetBuffer[42] << 8 |
						packetBuffer[43];

				/* Now convert NTP time into everyday time.
				 * Unix time starts on Jan 1 1970. In seconds, that's 2208988800.
				 * Subtract seventy years.
				 */
				const uint32_t seventyYears = 2208988800UL;
				uint32_t epoch = secsSince1900 - seventyYears;

				/* Print the hour, minute and second.
				 * GMT is the time at Greenwich Meridian.
				 */
				printf("socket_cb: The GMT time is %lu:%02lu:%02lu\r\n",
						(epoch  % 86400L) / 3600,           /* hour (86400 equals secs per day) */
						(epoch  % 3600) / 60,               /* minute (3600 equals secs per minute) */
						epoch % 60);                        /* second */

				ret = close(sock);
				if (ret == SOCK_ERR_NO_ERROR) {
					udp_socket = -1;
				}
			}
		}
	}
	break;

	default:
		break;
	}
}

TaskHandle_t wifi_task_handle; /**< Reference to LED0 toggling FreeRTOS task. */
SemaphoreHandle_t m_winc_int_semaphore; /**< Semaphore set in RTC event */
/**@brief LED0 task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void wifi_task_function (void * pvParameter)
{    
    UNUSED_PARAMETER(pvParameter);

    m_winc_int_semaphore = xSemaphoreCreateBinary();
    ASSERT(NULL != m_winc_int_semaphore);

    /* Initialize the BSP. */
    nm_bsp_init();

    /* Initialize Wi-Fi parameters structure. */
    memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

    /* Initialize Wi-Fi driver with data and status callbacks. */
    param.pfAppWifiCb = wifi_cb;
    ret = m2m_wifi_init(&param);
    if (M2M_SUCCESS != ret) {
            printf("main: m2m_wifi_init call error!(%d)\r\n", ret);
            while (1) {
            }
    }

    m2m_wifi_enable_firmware_logs(1);

    /* Initialize socket interface. */
    socketInit();
    registerSocketCallback((tpfAppSocketCb) socket_cb, (tpfAppResolveCb) resolve_cb);

    /* Set defined sleep mode */
    if (MAIN_PS_SLEEP_MODE == M2M_PS_MANUAL) {
            printf("M2M_PS_MANUAL\r\n");
            m2m_wifi_set_sleep_mode(MAIN_PS_SLEEP_MODE, 1);
    } else if (MAIN_PS_SLEEP_MODE == M2M_PS_DEEP_AUTOMATIC) {
            printf("M2M_PS_DEEP_AUTOMATIC\r\n");
            tstrM2mLsnInt strM2mLsnInt;
            m2m_wifi_set_sleep_mode(M2M_PS_DEEP_AUTOMATIC, 1);
            strM2mLsnInt.u16LsnInt = M2M_LISTEN_INTERVAL;
            m2m_wifi_set_lsn_int(&strM2mLsnInt);
    } else {
            printf("M2M_PS_NO\r\n");
    }

    /* Connect to defined AP. */
    m2m_wifi_connect((char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (void *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);

    while (true)
    {
        bsp_board_led_invert(BSP_BOARD_LED_3);
        NRF_LOG_INFO("WIFI TASK\n\r");

         while (1)
         {           
            while (m2m_wifi_handle_events(NULL) != M2M_SUCCESS) 
            {
            }
            /* Delay a task for a given number of ticks */
            UNUSED_RETURN_VALUE(xSemaphoreTake(m_winc_int_semaphore, portMAX_DELAY));
        }

       
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

    /* Create task for LED0 blinking with priority set to 2 */
    UNUSED_VARIABLE(xTaskCreate(wifi_task_function, "LAN", configMINIMAL_STACK_SIZE + 800, NULL, 2, &wifi_task_handle));

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
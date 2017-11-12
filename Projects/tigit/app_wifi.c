/** @file
 *
 * @brief WIFI MODULE FILE
 *
 * This file contains the source code for the wifi module
 *
 */

//http://www.atmel.com/Images/Atmel-42388-ATWINC1500-Xplained-Pro_UserGuide.pdf
// page 7

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "FreeRTOS.h"
#include "app_error.h"
#include "bsp.h"
#include "nordic_common.h"
#include "sdk_errors.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "m2m_wifi.h"
#include "socket.h"

#include "app_rtc.h"


// WIFI Stuff

#if 0
/** Wi-Fi Settings */
#define MAIN_WLAN_SSID "Schluesseldienst"
#define MAIN_WLAN_AUTH M2M_WIFI_SEC_WPA_PSK
#define MAIN_WLAN_PSK "4YGp7XL8BDEbkUwM"
#endif
#if 1
/** Wi-Fi Settings */
#define MAIN_WLAN_SSID "Tech_D0042715"
#define MAIN_WLAN_AUTH M2M_WIFI_SEC_WPA_PSK
#define MAIN_WLAN_PSK "GMZPVGYY"
#endif

/** PowerSave mode Settings */
#define MAIN_PS_SLEEP_MODE M2M_PS_DEEP_AUTOMATIC /* M2M_NO_PS / M2M_PS_DEEP_AUTOMATIC / M2M_PS_MANUAL */

/** Using NTP server information */
#define MAIN_WORLDWIDE_NTP_POOL_HOSTNAME "pool.ntp.org"
#define MAIN_ASIA_NTP_POOL_HOSTNAME "asia.pool.ntp.org"
#define MAIN_EUROPE_NTP_POOL_HOSTNAME "europe.pool.ntp.org"
#define MAIN_NAMERICA_NTP_POOL_HOSTNAME "north-america.pool.ntp.org"
#define MAIN_OCEANIA_NTP_POOL_HOSTNAME "oceania.pool.ntp.org"
#define MAIN_SAMERICA_NTP_POOL_HOSTNAME "south-america.pool.ntp.org"
#define MAIN_SERVER_PORT_FOR_UDP (123)
#define MAIN_DEFAULT_ADDRESS 0xFFFFFFFF /* "255.255.255.255" */
#define MAIN_DEFAULT_PORT (6666)
#define MAIN_WIFI_M2M_BUFFER_SIZE 1460

/** Wi-Fi status variable. */
static bool gbConnectedWifi = false;
/** Host name placeholder. */
static char dns_server_address[HOSTNAME_MAX_SIZE];

/** UDP socket handlers. */
static SOCKET udp_socket = -1;

/** Receive buffer definition. */
static uint8_t gau8SocketBuffer[MAIN_WIFI_M2M_BUFFER_SIZE];

static uint8 gu8SleepStatus;		  /**< Wi-Fi Sleep status. */

tstrWifiInitParam param;

tstrSystemTime* sys_time;

TaskHandle_t wifi_task_handle;		/**< Reference to LED0 toggling FreeRTOS task. */
SemaphoreHandle_t m_winc_int_semaphore; /**< Semaphore set in RTC event */

// WIFI Stuff

/**
 * \brief Callback to get the Wi-Fi status update.
 *
 * \param[in] u8MsgType Type of Wi-Fi notification.
 * \param[in] pvMsg A pointer to a buffer containing the notification parameters.
 */
static void wifi_cb(uint8_t u8MsgType, void* pvMsg) {
	switch (u8MsgType) {
		case M2M_WIFI_RESP_CON_STATE_CHANGED: {
			tstrM2mWifiStateChanged* pstrWifiState = (tstrM2mWifiStateChanged*)pvMsg;
			if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
				printf("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED\r\n");
				m2m_wifi_request_dhcp_client();
			} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
				printf("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED\r\n");
				gbConnectedWifi = false;
				m2m_wifi_connect((char*)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID),
					MAIN_WLAN_AUTH, (char*)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
			}

			break;
		}

		case M2M_WIFI_REQ_DHCP_CONF: {

			uint8_t* pu8IPAddress = (uint8_t*)pvMsg;
			printf("wifi_cb: M2M_WIFI_REQ_DHCP_CONF: IP is %u.%u.%u.%u\r\n",
				pu8IPAddress[0], pu8IPAddress[1], pu8IPAddress[2], pu8IPAddress[3]);
			gbConnectedWifi = true;
			memcpy(dns_server_address, (uint8_t*)MAIN_WORLDWIDE_NTP_POOL_HOSTNAME, strlen(MAIN_WORLDWIDE_NTP_POOL_HOSTNAME));
			/* Obtain the IP Address by network name */
			gethostbyname((uint8_t*)dns_server_address);
			break;
		}

		case M2M_WIFI_RESP_GET_SYS_TIME:

			NRF_LOG_INFO("M2M_WIFI_RESP_GET_SYS_TIME: ");

			struct tm time_struct;
			sys_time = (tstrSystemTime*)pvMsg;

			time_struct.tm_sec = (int)sys_time->u8Second;
			time_struct.tm_min = (int)sys_time->u8Minute;
			time_struct.tm_hour = (int)sys_time->u8Hour;
			time_struct.tm_mday = (int)sys_time->u8Day;
			time_struct.tm_mon = ((int)sys_time->u8Month) - 1;
			time_struct.tm_year = ((int)sys_time->u16Year) - 1900;

			unix_time = mktime(&time_struct);

			NRF_LOG_INFO("%s\n\r", ctime(&unix_time));
			//m2m_wifi_disconnect();
			//m2m_wifi_deinit(NULL);

			break;

		case M2M_WIFI_RESP_CURRENT_RSSI:

			NRF_LOG_INFO("M2M_WIFI_RESP_CURRENT_RSSI: ");

			sint8* rssi = (sint8*)pvMsg;
			M2M_INFO("ch rssi %d\n", *rssi);
			break;

		default: {
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
static void resolve_cb(uint8_t* pu8DomainName, uint32_t u32ServerIP) {
	
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
		ret = sendto(udp_socket, (int8_t*)&cDataBuf, sizeof(cDataBuf), 0, (struct sockaddr*)&addr, sizeof(addr));
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
static void socket_cb(SOCKET sock, uint8_t u8Msg, void* pvMsg) {
	/* Check for socket event on socket. */
	int16_t ret;

	switch (u8Msg) {
		case SOCKET_MSG_BIND: {
			/* printf("socket_cb: socket_msg_bind!\r\n"); */
			tstrSocketBindMsg* pstrBind = (tstrSocketBindMsg*)pvMsg;
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

		case SOCKET_MSG_RECVFROM: {
			/* printf("socket_cb: socket_msg_recvfrom!\r\n"); */
			tstrSocketRecvMsg* pstrRx = (tstrSocketRecvMsg*)pvMsg;
			if (pstrRx->pu8Buffer && pstrRx->s16BufferSize) {
				uint8_t packetBuffer[48];
				memcpy(&packetBuffer, pstrRx->pu8Buffer, sizeof(packetBuffer));

				if ((packetBuffer[0] & 0x7) != 4) { /* expect only server response */
					printf("socket_cb: Expecting response from Server Only!\r\n");
					return; /* MODE is not server, abort */
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
						(epoch % 86400L) / 3600, /* hour (86400 equals secs per day) */
						(epoch % 3600) / 60,	 /* minute (3600 equals secs per minute) */
						epoch % 60);			 /* second */

					ret = close(sock);
					if (ret == SOCK_ERR_NO_ERROR) {
						udp_socket = -1;
					}
				}
			}
		} break;

		default:
			break;
	}
}

/**@brief WIFI TASK HANDLE
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void wifi_task_function(void* pvParameter) {
	UNUSED_PARAMETER(pvParameter);

	sint8 ret = M2M_SUCCESS;

	m_winc_int_semaphore = xSemaphoreCreateBinary();
	ASSERT(NULL != m_winc_int_semaphore);

	/* Initialize the BSP. */
	nm_bsp_init();

	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t*)&param, 0, sizeof(tstrWifiInitParam));

	/* Initialize Wi-Fi driver with data and status callbacks. */
	param.pfAppWifiCb = wifi_cb;
	ret = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret) {
		printf("main: m2m_wifi_init call error!(%d)\r\n", ret);
		while (1) {
		}
	}

	ret = m2m_wifi_enable_firmware_logs(1);
	if (M2M_SUCCESS != ret) {
		NRF_LOG_ERROR("m2m_wifi_enable_firmware_logs call error!(%d)\r\n", ret);
	}

	/* Initialize socket interface. */
	socketInit();
	registerSocketCallback((tpfAppSocketCb)socket_cb, (tpfAppResolveCb)resolve_cb);

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
	m2m_wifi_connect((char*)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (void*)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);

	while (true) {
		bsp_board_led_invert(BSP_BOARD_LED_3);
		NRF_LOG_INFO("WIFI TASK\n\r");

		while (m2m_wifi_handle_events(NULL) != M2M_SUCCESS) {
		}
		/* Delay a task for a given number of ticks */
		UNUSED_RETURN_VALUE(xSemaphoreTake(m_winc_int_semaphore, portMAX_DELAY));
	}
}

int start_wifi_task(void) {
	
	ret_code_t err_code;

	/* Create task for LED0 blinking with priority set to 2 */
	err_code = (ret_code_t)xTaskCreate(wifi_task_function, "LAN", configMINIMAL_STACK_SIZE + 800, NULL, 2, &wifi_task_handle);
	if (err_code == pdPASS) {
		NRF_LOG_INFO("RTC TASK CREATED");
		err_code = NRF_SUCCESS;
	} else {
		NRF_LOG_ERROR("RTC TASK	CREATE ERROR");
		err_code = NRF_ERROR_NO_MEM;
	}

	return err_code;
	
}

/**
 *@}
 **/
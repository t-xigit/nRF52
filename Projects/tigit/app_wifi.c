/** @file app_wifi.c
 *
 * @brief WIFI MODULE FILE
 *
 * This file source file contains the source code for the wifi module
 *
 *  @addtogroup group_wifi
 *  this will also be in the wifi module documentaion.
 *  @{
 *
 */

//http://www.atmel.com/Images/Atmel-42388-ATWINC1500-Xplained-Pro_UserGuide.pdf
// page 7

#include <stdbool.h>
#include <stdint.h>
#include <time.h>


#include "app_error.h"
#include "bsp.h"
#include "nordic_common.h"
#include "sdk_errors.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "FreeRTOSConfig.h"


#include "m2m_wifi.h"
#include "socket.h"

#include "app_rtc.h"
#include "app_config.h"

#define NRF_LOG_MODULE_NAME app_wifi

#if APP_WIFI_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       APP_WIFI_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  APP_WIFI_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR APP_WIFI_CONFIG_DEBUG_COLOR

#else //APP_WIFI_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif //APP_WIFI_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#if 1
/** Wi-Fi Settings */
#define MAIN_WLAN_SSID "Schluesseldienst"
#define MAIN_WLAN_AUTH M2M_WIFI_SEC_WPA_PSK
#define MAIN_WLAN_PSK "4YGp7XL8BDEbkUwM"
#endif
#if 0
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

static uint8_t gau8SocketBuffer[MAIN_WIFI_M2M_BUFFER_SIZE];	/** Receive buffer definition. */

static uint8 gu8SleepStatus;				/**< Wi-Fi Sleep status. */

tstrWifiInitParam param;

tstrSystemTime* sys_time;

TaskHandle_t wifi_task_handle;			/**< Reference to LED0 toggling FreeRTOS task. */
SemaphoreHandle_t m_winc_int_semaphore;		/**< Semaphore set in RTC event */

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
				NRF_LOG_INFO("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED");
				m2m_wifi_request_dhcp_client();
			} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
				NRF_LOG_INFO("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED");
				gbConnectedWifi = false;
				m2m_wifi_connect((char*)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID),
					MAIN_WLAN_AUTH, (char*)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
			}

			break;
		}

		case M2M_WIFI_REQ_DHCP_CONF: {

			uint8_t* pu8IPAddress = (uint8_t*)pvMsg;
			NRF_LOG_INFO("wifi_cb: M2M_WIFI_REQ_DHCP_CONF: IP is %u.%u.%u.%u",
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

	NRF_LOG_INFO("resolve_cb: DomainName %s", pu8DomainName);

	if (udp_socket >= 0) {
		/* Set NTP server socket address structure. */
		addr.sin_family = AF_INET;
		addr.sin_port = _htons(MAIN_SERVER_PORT_FOR_UDP);
		addr.sin_addr.s_addr = u32ServerIP;

		/*Send an NTP time query to the NTP server*/
		ret = sendto(udp_socket, (int8_t*)&cDataBuf, sizeof(cDataBuf), 0, (struct sockaddr*)&addr, sizeof(addr));
		if (ret != M2M_SUCCESS) {
			NRF_LOG_INFO("resolve_cb: failed to send  error!");
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
					NRF_LOG_INFO("socket_cb: recv error!");
				}
			} else {
				NRF_LOG_INFO("socket_cb: bind error!");
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
					NRF_LOG_INFO("socket_cb: Expecting response from Server Only!");
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
					NRF_LOG_INFO("socket_cb: The GMT time is %lu:%02lu:%02lu",
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

		case SOCKET_MSG_CONNECT: {
			// Connect Event Handler.

			tstrSocketConnectMsg* pstrConnect = (tstrSocketConnectMsg*)pvMsg;
			if (pstrConnect->s8Error == 0) {
				uint8 acBuffer[256];
				uint16 u16MsgSize;

				NRF_LOG_INFO("Connect success!");
			} else {
				NRF_LOG_ERROR("Connection Failed, Error: %d", pstrConnect->s8Error);
			}

		} break;

		default:
			break;
	}
}

/**@brief turns off deinitalises all resources
 *
 */
void wifi_turn_off(void) {
   
  sint8 ret = M2M_SUCCESS;
  NRF_LOG_INFO("wifi_turn_off()");

  ret = m2m_wifi_disconnect();
  if(ret == M2M_SUCCESS){
    NRF_LOG_INFO("m2m_wifi_disconnect >>> OK");
  }else NRF_LOG_ERROR("m2m_wifi_disconnect >>> ERROR");
  
  ret = m2m_wifi_deinit(NULL);
  if(ret == M2M_SUCCESS){
    NRF_LOG_INFO("m2m_wifi_deinit >>> OK");
  }else NRF_LOG_ERROR("m2m_wifi_deinit >>> ERROR");
  
  ret = nm_bsp_deinit();
  if(ret == M2M_SUCCESS){
    NRF_LOG_INFO("nm_bsp_deinit >>> OK");
  }else NRF_LOG_ERROR("nm_bsp_deinit >>> ERROR");
  
  nm_bsp_interrupt_ctrl(false);

#if 0
  //@BUG doesn't work, both calls cause crashes
  NRF_LOG_INFO("vTaskDelete(wifi_task_handle)");
  vSemaphoreDelete(m_winc_int_semaphore);
  vTaskDelete(wifi_task_handle);
#endif
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
		NRF_LOG_INFO("main: m2m_wifi_init call error!(%d)", ret);
		while (1) {
		}
	}

	ret = m2m_wifi_enable_firmware_logs(1);
	if (M2M_SUCCESS != ret) {
		NRF_LOG_ERROR("m2m_wifi_enable_firmware_logs call error!(%d)", ret);
	}

	/* Initialize socket interface. */
	socketInit();
	registerSocketCallback((tpfAppSocketCb)socket_cb, (tpfAppResolveCb)resolve_cb);

	/* Set defined sleep mode */
	if (MAIN_PS_SLEEP_MODE == M2M_PS_MANUAL) {
		NRF_LOG_INFO("M2M_PS_MANUAL");
		m2m_wifi_set_sleep_mode(MAIN_PS_SLEEP_MODE, 1);
	} else if (MAIN_PS_SLEEP_MODE == M2M_PS_DEEP_AUTOMATIC) {
		NRF_LOG_INFO("M2M_PS_DEEP_AUTOMATIC");
		tstrM2mLsnInt strM2mLsnInt;
		m2m_wifi_set_sleep_mode(M2M_PS_DEEP_AUTOMATIC, 1);
		strM2mLsnInt.u16LsnInt = M2M_LISTEN_INTERVAL;
		m2m_wifi_set_lsn_int(&strM2mLsnInt);
	} else {
		NRF_LOG_INFO("M2M_PS_NO");
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

int wifi_start_task(void) {
	
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
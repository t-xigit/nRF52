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
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "m2m_wifi.h"
#include "socket.h"

#include "FreeRTOS_WINC1500_socket_wrapper.h"

#include "app_config.h"
#include "app_mqtt.h"
#include "app_rtc.h"
#include "app_wifi.h"

#define NRF_LOG_MODULE_NAME app_wifi

#if APP_WIFI_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL APP_WIFI_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR APP_WIFI_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR APP_WIFI_CONFIG_DEBUG_COLOR

#else  //APP_WIFI_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif  //APP_WIFI_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#if 1
/** Wi-Fi Settings */
#define MAIN_WLAN_SSID "Schluessel"
#define MAIN_WLAN_AUTH M2M_WIFI_SEC_WPA_PSK
#define MAIN_WLAN_PSK "verygoodpw123"
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

/** UDP socket handlers. */
static SOCKET udp_socket = -1;

static uint8_t gau8SocketBuffer[MAIN_WIFI_M2M_BUFFER_SIZE]; /** Receive buffer definition. */

static uint8 gu8SleepStatus; /**< Wi-Fi Sleep status. */

struct sockaddr_in resolved_addr;

tstrWifiInitParam param;

tstrSystemTime* sys_time;

uint8_t rxBuffer[rxBufferSize];
//tstrSocketRecvMsg* pstrRecv;
tstrSocketRecvMsg RecvData;
tstrSocketRecvMsg* RecvDataptr;

size_t RecvDataSize = sizeof(tstrSocketRecvMsg);

TaskHandle_t wifi_task_handle;			/**< Reference to LED0 toggling FreeRTOS task. */
SemaphoreHandle_t m_winc_int_semaphore; /**< Semaphore set in WIFI ISR */
SemaphoreHandle_t app_wifi_sys_t_Sema;  /**< Semaphore for WIFI client */
SemaphoreHandle_t app_dns_Sema;			/**< Semaphore for dns reslove wait */

SemaphoreHandle_t socket_rx_sema;

QueueHandle_t socket_snd_Q; /**< Queue for returning the error code from the Socket CB */
QueueHandle_t socket_rx_Q;  /**< Queue for RX Data from the Socket CB */

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

			m2m_wifi_get_sytem_time();
			//memcpy(dns_server_address, (uint8_t*)MQTT_BROKER_HOSTNAME, strlen(MQTT_BROKER_HOSTNAME));
			/* Obtain the IP Address by network name */
			//gethostbyname((uint8_t*)dns_server_address);
			break;
		}

		case M2M_WIFI_RESP_GET_SYS_TIME:

			NRF_LOG_INFO("wifi_cb: M2M_WIFI_RESP_GET_SYS_TIME");
			//time at compilation
			const time_t ref_time = 1513981235;
			time_t temp_time = 0;

			struct tm time_struct;
			memset(&time_struct, 0, sizeof(struct tm));
			sys_time = (tstrSystemTime*)pvMsg;

			time_struct.tm_sec = (int)sys_time->u8Second;
			time_struct.tm_min = (int)sys_time->u8Minute;
			time_struct.tm_hour = (int)sys_time->u8Hour;
			time_struct.tm_mday = (int)sys_time->u8Day;
			time_struct.tm_mon = ((int)sys_time->u8Month) - 1;
			time_struct.tm_year = ((int)sys_time->u16Year) - 1900;

			temp_time = mktime(&time_struct);
			// if received time makes sense
			if (temp_time > ref_time) {
				unix_time = temp_time;
				NRF_LOG_INFO("%s", ctime(&unix_time));
				//unlock rtc task
				xSemaphoreGive(m_rtc_semaphore);
				xSemaphoreGive(app_wifi_sys_t_Sema);
			}

			break;

		case M2M_WIFI_RESP_CURRENT_RSSI:

			NRF_LOG_INFO("wifi_cb: M2M_WIFI_RESP_CURRENT_RSSI");

			sint8* rssi = (sint8*)pvMsg;
			M2M_INFO("ch rssi %d", *rssi);
			break;

		default: {
			NRF_LOG_ERROR("<<<<<<< wifi_cb >>>>>>");
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
	int16_t ret;
	char resloved_ip_hex[4];
	uint32_t temp_u32ServerIP = u32ServerIP;
	resolved_addr.sin_family = AF_INET;
	resolved_addr.sin_port = _htons(MAIN_SERVER_PORT_FOR_UDP);
	resolved_addr.sin_addr.s_addr = u32ServerIP;

	memset(resloved_ip_hex, 0, sizeof(resloved_ip_hex));

	resloved_ip_hex[3] = (uint8_t)((u32ServerIP >> 24) & 0xff);
	resloved_ip_hex[2] = (uint8_t)((u32ServerIP >> 16) & 0xff);
	resloved_ip_hex[1] = (uint8_t)((u32ServerIP >> 8) & 0xff);
	resloved_ip_hex[0] = (uint8_t)(u32ServerIP & 0xff);

	NRF_LOG_INFO("resolve_cb >>> DomainName: %s >>> IP : %d.%d.%d.%d",
		pu8DomainName,
		resloved_ip_hex[0],
		resloved_ip_hex[1],
		resloved_ip_hex[2],
		resloved_ip_hex[3]);

	xSemaphoreGive(app_dns_Sema);
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
	sint16 s16Rcvd = 0;

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

		case SOCKET_MSG_SEND: {
			sint16 s16Rcvd = 0;
			s16Rcvd = *(sint16*)pvMsg;
			NRF_LOG_DEBUG("socket_cb: SOCKET_MSG_SEND: %d", s16Rcvd);
			//calling receive because page 31 in software design guide
			ret = recv(0, rxBuffer, rxBufferSize, 0);
			//calling receive because page 31 in software design guide
			if (xQueueSend(socket_snd_Q, (void*)&s16Rcvd, (TickType_t)10) != pdPASS) {
				NRF_LOG_ERROR("xQueueSend >>> ERROR >>> socket_snd_Q");
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

		case SOCKET_MSG_RECV: {
			NRF_LOG_DEBUG("socket_cb: SOCKET_MSG_RECV");
			tstrSocketRecvMsg* pstrRecv = (tstrSocketRecvMsg*)pvMsg;
			pstrRecv->pu8Buffer = rxBuffer;

			memcpy(&RecvData, pstrRecv, sizeof(tstrSocketRecvMsg));

			if (sock == mqtt_client.ipstack->my_socket) {
				// This means an error occurred
				if (pstrRecv->s16BufferSize <= 0) {
					NRF_LOG_DEBUG("SOCK_ERR >>> CODE: %d >>> ", pstrRecv->s16BufferSize);
					if (pstrRecv->s16BufferSize == SOCK_ERR_CONN_ABORTED) {
						NRF_LOG_ERROR("SOCK_ERR_CONN_ABORTED\n\r");
					} else if (pstrRecv->s16BufferSize == SOCK_ERR_NO_ERROR) {
						NRF_LOG_ERROR("SOCKET CONNECTION IS TERMINATED\n\r");
					}
				}
				// This means there are valid data to be pulled
				else if (pstrRecv->s16BufferSize > 0) {
					NRF_LOG_DEBUG("SOCKET_MSG_RECV >>> MSG_OK : %d", pstrRecv->s16BufferSize);

					NRF_LOG_DEBUG("SOCKET_MSG_RECV >>> FreeRTOS_recv_copy");
					FreeRTOS_recv_copy(pstrRecv);

					if (xSemaphoreGive(socket_rx_sema) != pdPASS) {
						NRF_LOG_ERROR("xSemaphoreGive >>> ERROR >>> socket_rx_sema");
					}
				}
			}

		} break;

		case SOCKET_MSG_CONNECT: {
			// Connect Event Handler.
			NRF_LOG_DEBUG("socket_cb: SOCKET_MSG_CONNECT");

			tstrSocketConnectMsg* pstrConnect = (tstrSocketConnectMsg*)pvMsg;
			if (pstrConnect->s8Error == 0) {
				uint8 acBuffer[256];
				uint16 u16MsgSize;

				//calling receive because page 31 in software design guide
				//ret = (sint16)recv(mqtt_client.ipstack ->my_socket, rxBuffer, rxBufferSize, 0);

				NRF_LOG_INFO("socket_cb: Socket Connected");
				xSemaphoreGive(app_socket_Sema);

			} else if (pstrConnect->s8Error == SOCK_ERR_CONN_ABORTED) {
				NRF_LOG_ERROR("socket_cb: Socket Connection Failed >>> SOCK_ERR_CONN_ABORTED");
			} else {
				NRF_LOG_ERROR("socket_cb: Socket Connection Failed, Error: %d", pstrConnect->s8Error);
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
	if (ret == M2M_SUCCESS) {
		NRF_LOG_INFO("m2m_wifi_disconnect >>> OK");
	} else
		NRF_LOG_ERROR("m2m_wifi_disconnect >>> ERROR");

	ret = m2m_wifi_deinit(NULL);
	if (ret == M2M_SUCCESS) {
		NRF_LOG_INFO("m2m_wifi_deinit >>> OK");
	} else
		NRF_LOG_ERROR("m2m_wifi_deinit >>> ERROR");

	ret = nm_bsp_deinit();
	if (ret == M2M_SUCCESS) {
		NRF_LOG_INFO("nm_bsp_deinit >>> OK");
	} else
		NRF_LOG_ERROR("nm_bsp_deinit >>> ERROR");

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

	app_dns_Sema = xSemaphoreCreateBinary();
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

	ret = m2m_wifi_enable_firmware_logs(0);
	if (M2M_SUCCESS != ret) {
		NRF_LOG_ERROR("m2m_wifi_enable_firmware_logs call error!(%d)", ret);
	} else
		NRF_LOG_INFO("m2m_wifi_enable_firmware_logs >>> OFF");

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
		NRF_LOG_DEBUG("WIFI TASK");

		while (m2m_wifi_handle_events(NULL) != M2M_SUCCESS) {
		}
		/* Delay a task for a given number of ticks */
		UNUSED_RETURN_VALUE(xSemaphoreTake(m_winc_int_semaphore, portMAX_DELAY));
	}
}

int wifi_start_task(void) {
	ret_code_t err_code;

	/* Attempt to create a semaphore. */
	app_wifi_sys_t_Sema = xSemaphoreCreateBinary();

	if (app_wifi_sys_t_Sema == NULL) {
		/* There was insufficient FreeRTOS heap available for the semaphore to
	       be created successfully. */
	} else {
		/* The semaphore can now be used. Its handle is stored in the
		  xSemahore variable.  Calling xSemaphoreTake() on the semaphore here
		  will fail until the semaphore has first been given. */
	}

	socket_rx_sema = xSemaphoreCreateBinary();

	socket_snd_Q = xQueueCreate(1, sizeof(sint16));

	if (socket_snd_Q == NULL) {
		NRF_LOG_ERROR("xQueueCreate >>> ERROR >>> socket_snd_Q");
	}

	//	socket_rx_Q = xQueueCreate(1, sizeof(tstrSocketRecvMsg*));
	//
	//	if (socket_rx_Q == NULL) {
	//		NRF_LOG_ERROR("xQueueCreate >>> ERROR >>> socket_rx_Q");
	//	}

	//vTaskDelay(1000);
	/* Create task for LED0 blinking with priority set to 2 */
	err_code = (ret_code_t)xTaskCreate(wifi_task_function, "LAN", configMINIMAL_STACK_SIZE * 11, NULL, 2, &wifi_task_handle);
	if (err_code == pdPASS) {
		NRF_LOG_INFO("WIFI TASK CREATED");
		err_code = NRF_SUCCESS;
	} else {
		NRF_LOG_ERROR("WIFI TASK CREATE ERROR");
		err_code = NRF_ERROR_NO_MEM;
	}

	return err_code;
}

/**
 *@}
 **/
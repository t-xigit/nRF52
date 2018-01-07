/**
 *
 * @addtogroup group_wifi WIFI Module
 * @ingroup group_app
 * @brief WIFI APIs.
 * @details The clock HAL provides basic APIs for accessing the registers of the clock.
 * The clock driver provides APIs on a higher level.
 *
 * @defgroup app_wifi_drv WIFI driver
 * @{
 * @ingroup group_wifi
 * @brief Driver for managing the low-frequency clock (LFCLK) and the high-frequency clock (HFCLK).
 */

#ifndef SDK_APP_WIFI_H__
#define SDK_APP_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "m2m_wifi.h"
#include "sdk_errors.h"
#include "socket.h"

extern struct sockaddr_in resolved_addr;

//Semaphores
extern SemaphoreHandle_t app_wifi_sys_t_Sema;
extern SemaphoreHandle_t app_dns_Sema;
extern SemaphoreHandle_t socket_rx_sema;
//Queues
extern QueueHandle_t socket_snd_Q;
extern QueueHandle_t socket_rx_Q;

#define rxBufferSize 256
#define wifi_req_curr_rssi() m2m_wifi_req_curr_rssi()

typedef struct {
	uint8_t rxBuffer[rxBufferSize];
	/*!<		
		Buffer for the received socket message.
	*/
	tstrSocketRecvMsg tstrSocketRecvMsg;
	/*!<
		Struct with the received socket message meta data.
	*/
} rx_socket_msg_t;

int wifi_start_task(void);
void wifi_turn_off(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif  // SDK_COMMON_H__
/**
 *
 * @addtogroup group_mqtt MQTT Module
 * @ingroup group_app
 * @brief MQTT APIs.
 * @details The clock HAL provides basic APIs for accessing the registers of the clock.
 * The clock driver provides APIs on a higher level.
 *
 * @defgroup group_mqtt MQTT stuff
 * @{
 * @ingroup group_mqtt
 * @brief Driver for managing the low-frequency clock (LFCLK) and the high-frequency clock (HFCLK).
 */

#ifndef __APP_MQTT_
#define __APP_MQTT_

#ifdef __cplusplus
extern "C" {
#endif

#include "MQTTClient.h"

extern SemaphoreHandle_t app_socket_Sema;
extern SemaphoreHandle_t app_BS_Sema;
extern SemaphoreHandle_t app_button4_Sema;
extern MQTTClient mqtt_client;
extern Network network;

extern QueueHandle_t mqtt_publish_Q; /**< Queue for messages to be published */

typedef struct Pub_MQTTMessage_t {
	MQTTMessage Message;	
	MQTTString MessageTopic;
	MessageData MesseagData;
	char payload_buff[30];
} Pub_MQTTMessage;

/* Publish Topics */
typedef enum publishTopics_t {
	white = 1,
	black,
	online,
}publishTopics;

int mqtt_start_task(void);
void app_MQTTPublishSendQueue(publishTopics topic, uint32_t payload);

/** @} */

#ifdef __cplusplus
}
#endif

#endif  // SDK_COMMON_H__
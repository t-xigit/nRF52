/** @file app_mqtt.c
 *  
 * @brief MQTT MODULE FILE
 *
 * This	file contains the source code for the rtc module
 *
 * @addtogroup group_mqtt
 *  this source will also be in the RTC module documentaion.
 *  @{
 */

/* Standard includes. */
#include <stdbool.h>
#include <stdint.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "MQTTClient.h"
#include "MQTTFreeRTOS.h"

/* Application includes. */
#include "app_config.h"
#include "app_mqtt.h"
#include "app_wifi.h"

#define NRF_LOG_MODULE_NAME app_mqtt

#if APP_MQTT_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL APP_RTC_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR APP_RTC_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR APP_RTC_CONFIG_DEBUG_COLOR

#else  //APP_MQTT_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif  //APP_RTC_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define MQTT_BROKER_HOSTNAME "test.mosquitto.org" /**< MQTT Broker Address */

#define MQTT_BROKER_PORT 1883 /**< MQTT Broker Port */

/**
 * @brief RTC instance number used for blinking
 *
 */
#define BLINK_RTC 2

const char topic_online[] = "table/online/";
const char topic_white_goal[] = "goal/white/";
const char topic_black_goal[] = "goal/black/";

unsigned char MQTTsendbuf[80], MQTTreadbuf[80];
MQTTClient mqtt_client;
Network network;

TaskHandle_t mqtt_task_handle;	       /**< Taskhandle for MQTT client */
TaskHandle_t mqtt_publish_handle;      /**< Taskhandle for MQTT client */
SemaphoreHandle_t app_socket_Sema;     /**< Semaphore for MQTT client */
SemaphoreHandle_t app_button4_Sema;    /**< Seamphore for button press indicator */

QueueHandle_t mqtt_publish_Q; /**< Queue for messages to be published */

void messageArrived(MessageData* data) {
	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, data->message->payload);
}

/**@brief Prepares Topic to be published on
 *
 * @param[in] msg   Pointer to message struct
 * @param[in] topic one of the pre-defined topics
 */
static void app_MQTTPublishSetTopic(Pub_MQTTMessage* msg, publishTopics topic) {		

	switch (topic) {
		case white: {
			msg->MessageData.topicName->cstring = (char*)topic_white_goal;
			msg->MessageData.topicName->lenstring.len = strlen(topic_white_goal);			
			break;
		}

		case black: {
			msg->MessageData.topicName->cstring = (char*)topic_black_goal;
			msg->MessageData.topicName->lenstring.len = strlen(topic_black_goal);
			break;
		}

		case online: {
			msg->MessageData.topicName->cstring = (char*)topic_online;
			msg->MessageData.topicName->lenstring.len = strlen(topic_online);
			break;
		}
	}
        
}

void app_MQTTPublishSendQueue(publishTopics topic, uint32_t payload) {
	Pub_MQTTMessage pub_msg;
	memset(&pub_msg, 0, sizeof(Pub_MQTTMessage));

	pub_msg.MessageData.message->payload = (void*)pub_msg.payload_buff;
	pub_msg.MessageData.message->qos = 1;
	pub_msg.MessageData.message->retained = 0;
        sprintf(pub_msg.payload_buff, "%d",payload);
	pub_msg.MessageData.message->payloadlen = strlen(pub_msg.payload_buff);       
	
	app_MQTTPublishSetTopic(&pub_msg, topic);	

	if (mqtt_publish_Q != 0) {
		if (xQueueSend(mqtt_publish_Q,
				(void*)&pub_msg,
				(TickType_t)10) != pdPASS) {
			/* Failed to post the message, even after 10 ticks. */
                        NRF_LOG_ERROR("app_MQTTPublish >>> Queue write error");
		}			
	}
}

void app_MQTTPublishQueueHandler(publishTopics topic, uint32_t payload) {
	Pub_MQTTMessage pub_msg;
	size_t queue_size = 0;
	int rc = 0;
        NRF_LOG_INFO("app_MQTTPublishQueueHandler");
	while (1) {
		NRF_LOG_INFO("app_MQTTPublishQueueHandler");
		// initialize buffer
		memset(&pub_msg, 0, sizeof(Pub_MQTTMessage));
		// check how many items are in the queue
		queue_size = uxQueueMessagesWaiting(mqtt_publish_Q);
		NRF_LOG_INFO("MQTT Publish Queue Items: %d", queue_size);
		// read message from queue
		xQueueReceive(mqtt_publish_Q, &pub_msg, (TickType_t)portMAX_DELAY);
		
		NRF_LOG_INFO("Publishing: %s", pub_msg.MessageData.topicName->cstring);

		if ((rc = MQTTPublish(&mqtt_client, pub_msg.MessageData.topicName->cstring, pub_msg.MessageData.message)) != 0)
			NRF_LOG_DEBUG("Return code from MQTT publish is %d\n", rc);
		NRF_LOG_DEBUG("Return code from MQTT publish is %d\n", rc);
	}
}

static void prvMQTTEchoTask(void* pvParameters) {
	int rc = 0,
	count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

	pvParameters = 0;
	NetworkInit(&network);
	MQTTClientInit(&mqtt_client, &network, 30000, MQTTsendbuf, sizeof(MQTTsendbuf), MQTTreadbuf, sizeof(MQTTreadbuf));

	char* address = MQTT_BROKER_HOSTNAME;

	/* Wait till internet connection is established */
	if (xSemaphoreTake(app_wifi_sys_t_Sema, (TickType_t)portMAX_DELAY) == pdTRUE) {
		NRF_LOG_INFO("NETWORK CONNECTED");
	} else {
		NRF_LOG_ERROR("NETWORK CONNECT TIMEOUT");
		vTaskSuspend(NULL);
	}

	/* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */

	if ((rc = NetworkConnect(&network, address, MQTT_BROKER_PORT)) != 0)
		NRF_LOG_INFO("Return code from network connect is %d", rc);

	/* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
	if (xSemaphoreTake(app_socket_Sema, (TickType_t)portMAX_DELAY) == pdTRUE) {
		NRF_LOG_INFO("SOCKET CONNECTED");
	} else {
		NRF_LOG_ERROR("SOCKET CONNECT TIMEOUT");
		vTaskSuspend(NULL);
	}

#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&mqtt_client)) != pdPASS) {
		NRF_LOG_ERROR("Return code from start tasks is %d\n", rc);

	} else {
		NRF_LOG_INFO("MQTT Task Created");
	}

#endif

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "FreeRTOS_sample";

	NRF_LOG_INFO("MQTT Connect");
	if ((rc = MQTTConnect(&mqtt_client, &connectData)) != 0) {
		NRF_LOG_ERROR("Return code from MQTT connect is %d", rc);
	} else {
		NRF_LOG_INFO("MQTT Connected");
	}
/*
	if ((rc = MQTTSubscribe(&mqtt_client, "testtop/one/", 1, messageArrived)) != 0) {
		NRF_LOG_ERROR("Return code from MQTT subscribe is %d", rc);
	} else {
		NRF_LOG_INFO("MQTT Subscribed");
	}
*/
	MQTTMessage message;
	char payload[30];

	message.qos = 1;
	message.retained = 0;
	message.payload = payload;
	sprintf(payload, "online");
	message.payloadlen = strlen(payload);

	if ((rc = MQTTPublish(&mqtt_client, "testtop/one/", &message)) != 0)
		NRF_LOG_DEBUG("Return code from MQTT publish is %d\n", rc);
	NRF_LOG_DEBUG("Return code from MQTT publish is %d\n", rc);

	app_MQTTPublishSendQueue(online, 1);
        app_MQTTPublishSendQueue(white, 1);
	app_MQTTPublishSendQueue(black, 1);
	
	ret_code_t err_code = (ret_code_t)xTaskCreate(app_MQTTPublishQueueHandler,  /* The function that implements the task. */
		"PQU",								    /* Just a text name for the task to aid debugging. */
		configMINIMAL_STACK_SIZE *4,					    /* The stack size is defined in FreeRTOSIPConfig.h. */
		NULL,								    /* The task parameter, not used in this case. */
		2,								    /* The priority assigned to the task is defined in FreeRTOSConfig.h. */
		mqtt_publish_handle);						    /* The task handle is not used. */

	if (err_code == pdPASS) {
		NRF_LOG_INFO("MQTT Publish TASK CREATED");
		err_code = NRF_SUCCESS;
	} else {
		NRF_LOG_ERROR("MQTT TASK CREATE ERROR");
		err_code = NRF_ERROR_NO_MEM;
	}

        


	vTaskSuspend(NULL);
}

/**@brief RTC task handle function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
int mqtt_start_task(void) {
	ret_code_t err_code;

	/* Attempt to create a semaphore. */
	app_socket_Sema = xSemaphoreCreateBinary();

	/* Attempt to create a semaphore. */
	app_button4_Sema = xSemaphoreCreateBinary();

	if (app_socket_Sema == NULL) {
		/* There was insufficient FreeRTOS heap available for the semaphore to
	       be created successfully. */
		NRF_LOG_ERROR("app_socket_Sema");
	} else {
		/* The semaphore can now be used. Its handle is stored in the
		  xSemahore variable.  Calling xSemaphoreTake() on the semaphore here
		  will fail until the semaphore has first been given. */
		NRF_LOG_INFO("app_socket_Sema");
	}

	mqtt_publish_Q = xQueueCreate(10, sizeof(Pub_MQTTMessage));
	if (mqtt_publish_Q == NULL) {
		NRF_LOG_ERROR("xQueueCreate >>> ERROR >>> mqtt_publish_Q");
	}

	if (socket_rx_Q == NULL) {
		NRF_LOG_ERROR("xQueueCreate >>> ERROR >>> socket_rx_Q");
	}

	err_code = (ret_code_t)xTaskCreate(prvMQTTEchoTask, /* The function that implements the task. */
		"AMQ",											/* Just a text name for the task to aid debugging. */
		configMINIMAL_STACK_SIZE + 800,					/* The stack size is defined in FreeRTOSIPConfig.h. */
		NULL,											/* The task parameter, not used in this case. */
		2,												/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
		mqtt_task_handle);								/* The task handle is not used. */

	if (err_code == pdPASS) {
		NRF_LOG_INFO("MQTT TASK CREATED");
		err_code = NRF_SUCCESS;
	} else {
		NRF_LOG_ERROR("MQTT TASK CREATE ERROR");
		err_code = NRF_ERROR_NO_MEM;
	}

	return err_code;
}

/**
 *@}
 **/
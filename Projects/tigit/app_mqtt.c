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
#include "semphr.h"
#include "task.h"
#include "timers.h"

/* Application includes. */
#include "app_config.h"
#include "app_mqtt.h"
#include "app_wifi.h"

#include "MQTTFreeRTOS.h"
#include "MQTTClient.h"

#define NRF_LOG_MODULE_NAME app_mqtt

#if APP_MQTT_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       APP_RTC_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  APP_RTC_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR APP_RTC_CONFIG_DEBUG_COLOR

#else //APP_RTC_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif //APP_RTC_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define	TASK_DELAY 600	      /**< Task	delay. Delays a	LED0 task for 200 ms */
#define	TIMER_PERIOD 500      /**< Timer period. LED1 timer will expire	after 1000 ms */
#define	RTC_PERIOD 1000000ULL /**< Timer period. LED1 timer will expire	after 1000 ms */

/**
 * @brief RTC instance number used for blinking
 *
 */
#define	BLINK_RTC 2

TaskHandle_t mqtt_task_handle;		  /**< Taskhandle for MQTT client */
SemaphoreHandle_t app_mqtt_Semaphore;	  /**< Semaphore for MQTT client */

void messageArrived(MessageData* data)
{
	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, data->message->payload);
}

static void prvMQTTEchoTask(void *pvParameters)
{
	/* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
	MQTTClient client;
	Network network;
	unsigned char sendbuf[80], readbuf[80];
	int rc = 0, 
		count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

	pvParameters = 0;
	NetworkInit(&network);
	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

        /* Wait till internet connection is established */
	if (xSemaphoreTake(app_mqtt_Semaphore, (TickType_t)3000) == pdTRUE) {
		NRF_LOG_INFO("NETWORK CONNECTED");		
	} else {
		NRF_LOG_ERROR("NETWORK CONNECT TIMEOUT");
	}

	char* address = dns_server_address;
	if ((rc = NetworkConnect(&network, address, 1883)) != 0)
		printf("Return code from network connect is %d\n", rc);

	/* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
	if (xSemaphoreTake(app_mqtt_Semaphore, (TickType_t)3000) == pdTRUE) {
		NRF_LOG_INFO("NETWORK CONNECTED");		
	} else {
		NRF_LOG_ERROR("NETWORK CONNECT TIMEOUT");
	}

#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&client)) != pdPASS)
		printf("Return code from start tasks is %d\n", rc);
#endif

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "FreeRTOS_sample";

	if ((rc = MQTTConnect(&client, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");

	if ((rc = MQTTSubscribe(&client, "FreeRTOS/sample/#", 2, messageArrived)) != 0)
		printf("Return code from MQTT subscribe is %d\n", rc);

	count = 10;
	while (count--)
	{
		MQTTMessage message;
		char payload[30];

		message.qos = 1;
		message.retained = 0;
		message.payload = payload;
		sprintf(payload, "message number %d", count);
		message.payloadlen = strlen(payload);

		if ((rc = MQTTPublish(&client, "FreeRTOS/sample/a", &message)) != 0)
			printf("Return code from MQTT publish is %d\n", rc);
#if !defined(MQTT_TASK)
		if ((rc = MQTTYield(&client, 1000)) != 0)
			printf("Return code from yield is %d\n", rc);
#endif
	}

	/* do not return */
}

#if 0
void vStartMQTTTasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority)
{
	BaseType_t x = 0L;

	xTaskCreate(prvMQTTEchoTask,	/* The function that implements the task. */
			"MQTTEcho0",			/* Just a text name for the task to aid debugging. */
			usTaskStackSize,	/* The stack size is defined in FreeRTOSIPConfig.h. */
			(void *)x,		/* The task parameter, not used in this case. */
			uxTaskPriority,		/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
			NULL);				/* The task handle is not used. */
}
#endif


/**@brief RTC task handle function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
int mqtt_start_task(void){
	
    ret_code_t err_code;

	/* Attempt to create a semaphore. */
	app_mqtt_Semaphore = xSemaphoreCreateBinary();

	if (app_mqtt_Semaphore == NULL) {
		/* There was insufficient FreeRTOS heap available for the semaphore to
	       be created successfully. */
	} else {
		/* The semaphore can now be used. Its handle is stored in the
		  xSemahore variable.  Calling xSemaphoreTake() on the semaphore here
		  will fail until the semaphore has first been given. */
	}

	err_code = (ret_code_t)xTaskCreate(prvMQTTEchoTask,	/* The function that implements the task. */
		    "MQTT",					/* Just a text name for the task to aid debugging. */
		    configMINIMAL_STACK_SIZE + 800,		/* The stack size is defined in FreeRTOSIPConfig.h. */
		    NULL,				/* The task parameter, not used in this case. */
		    2,					/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
		    mqtt_task_handle);					/* The task handle is not used. */
		    
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
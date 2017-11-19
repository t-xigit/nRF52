/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - convert to FreeRTOS
 *******************************************************************************/

#include "FreeRTOS.h"

#include "FreeRTOS_WINC1500_socket_wrapper.h"
#include "socket/include/socket.h"

void FreeRTOS_disconnect(Network* n) {

	int16_t ret;

	ret = close(n->my_socket);
	if (ret == SOCK_ERR_NO_ERROR) {
		NRF_LOG_DEBUG("SOCKET CLOSED");
	} else
		NRF_LOG_ERROR("SOCKET CLOSE ERROR");
}

int FreeRTOS_write(Network* n, unsigned char* buffer, int len, int timeout_ms) {

	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	int sentLen = 0;
	int16_t ret;

	vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */
	do {
		int rc = 0;

		ret = send(n->my_socket, buffer, len, (uint16)0);
                sentLen = ret;

		//TODO FreeRTOS_setsockopt(n->my_socket, 0, FREERTOS_SO_RCVTIMEO, &xTicksToWait, sizeof(xTicksToWait));
		//TODO rc = FreeRTOS_send(n->my_socket, buffer + sentLen, len - sentLen, 0);
		if (ret == SOCK_ERR_NO_ERROR) {
			NRF_LOG_DEBUG("SOCKET SEND OK");
			break;
		} else {
			NRF_LOG_ERROR("SOCKET SEND ERROR");
		}

	} while (xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);

	return sentLen;
}
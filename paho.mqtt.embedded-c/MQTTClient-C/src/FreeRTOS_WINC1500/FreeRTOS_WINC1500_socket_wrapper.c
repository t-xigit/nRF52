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
#include "socket/include/socket.h"

#include "FreeRTOS_WINC1500_socket_wrapper.h"

#include "app_wifi.h"

/** Host name placeholder. */
char dns_server_address[HOSTNAME_MAX_SIZE];


void FreeRTOS_disconnect(Network* n) {

	int16_t ret;

	ret = close(n -> my_socket);
	if (ret == SOCK_ERR_NO_ERROR) {
		NRF_LOG_DEBUG("SOCKET CLOSED");
	} else
		NRF_LOG_ERROR("SOCKET CLOSE ERROR");
}

uint32_t FreeRTOS_gethostbyname(const uint8_t* pcHostName) {

	char resloved_ip_hex[4];

	memcpy(dns_server_address, pcHostName, strlen(pcHostName));
	gethostbyname((uint8_t*)dns_server_address);

	if (xSemaphoreTake(app_dns_Sema, (TickType_t)5000) == pdTRUE) {	      

		memset(resloved_ip_hex, 0, sizeof(resloved_ip_hex));

		resloved_ip_hex[3] = (uint8_t)((resolved_addr.sin_addr.s_addr >> 24) & 0xff);
		resloved_ip_hex[2] = (uint8_t)((resolved_addr.sin_addr.s_addr >> 16) & 0xff);
		resloved_ip_hex[1] = (uint8_t)((resolved_addr.sin_addr.s_addr >> 8) & 0xff);
		resloved_ip_hex[0] = (uint8_t)(resolved_addr.sin_addr.s_addr & 0xff);

		NRF_LOG_INFO("FreeRTOS_gethostbyname >>> DomainName: %s >>> IP : %d.%d.%d.%d",
			pcHostName,
			resloved_ip_hex[0],
			resloved_ip_hex[1],
			resloved_ip_hex[2],
			resloved_ip_hex[3]);
	} else {
		NRF_LOG_ERROR("FreeRTOS_gethostbyname >>> TIMEOUT");
	}

	return resolved_addr.sin_addr.s_addr;
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

int NetworkConnect(Network* n, char* addr, int port) {
	//TODO struct freertos_sockaddr sAddr;
	struct sockaddr_in sAddr;
	int retVal = -1;
	SOCKET TcpClientSocket = -1;
	uint32_t ipAddress;   
	 
        memset(&sAddr, 0, sizeof(struct sockaddr));

	if ((ipAddress = FreeRTOS_gethostbyname(addr)) == 0)
		goto exit;

	//TODO sAddr.sin_port = FreeRTOS_htons(port);
	//sAddr.sin_addr = (in_addr) ipAddress;

	sAddr.sin_family = AF_INET;
	sAddr.sin_port = _htons(port);
	sAddr.sin_addr.s_addr = ipAddress;	

	TcpClientSocket = socket(sAddr.sin_family, SOCK_STREAM, 0);
	if (TcpClientSocket < 0) {
		NRF_LOG_ERROR("SOCKET CREATE ERROR >>> %d", TcpClientSocket);
	} else {
		NRF_LOG_INFO("SOCKET CREATE >>> OK");
		n->my_socket = TcpClientSocket;
	}

	// sockets connect  	
        retVal = connect(TcpClientSocket, (struct sockaddr*) &sAddr, sizeof(struct sockaddr));

	if (retVal != 0) {
		NRF_LOG_ERROR(" SOCKET CONNECT CALL >>> FAILED");
		close(TcpClientSocket);
	} else {
		NRF_LOG_INFO("  SOCKET CONNECT CALL >>> OK");
	}

exit:
	return retVal;
}

int FreeRTOS_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;	   /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	int recvLen = 0;

	vTaskSetTimeOutState(&xTimeOut);				   /* Record the time at which this function was entered. */
	do
	{
		int rc = 0;

		//TODO FreeRTOS_setsockopt(n->my_socket, 0, FREERTOS_SO_RCVTIMEO, &xTicksToWait, sizeof(xTicksToWait));
		//TODO rc = FreeRTOS_recv(n->my_socket, buffer + recvLen, len - recvLen, 0);
		if (rc > 0)
			recvLen += rc;
		else if (rc < 0)
		{
			recvLen = rc;
			break;
		}
	} while (recvLen < len && xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);

	return recvLen;
}
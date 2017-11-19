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
 *******************************************************************************/

#if !defined(MQTTFreeRTOS_H)
#define MQTTFreeRTOS_H

#define USE_WINC1500_WRAPPER 1

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#ifndef  USE_WINC1500_WRAPPER
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP.h"
#endif

#ifdef  USE_WINC1500_WRAPPER
#include "FreeRTOS_WINC1500_socket_wrapper.h"
#include "socket/include/socket.h"

// use WINC SOCKET instead
#define xSocket_t SOCKET
#endif

typedef struct Timer 
{
	TickType_t xTicksToWait;
	TimeOut_t xTimeOut;
} Timer;

//typedef struct Network Network;
//
//struct Network
//{
//	xSocket_t my_socket;
//	int (*mqttread) (Network*, unsigned char*, int, int);
//	int (*mqttwrite) (Network*, unsigned char*, int, int);
//	void (*disconnect) (Network*);
//};

typedef struct Network {
    int signiture;
    int size;
    struct Network* next;
    struct Network* prev;
} Network;

//Network* Network;

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

typedef struct Mutex
{
	SemaphoreHandle_t sem;
} Mutex;

void MutexInit(Mutex*);
int MutexLock(Mutex*);
int MutexUnlock(Mutex*);

typedef struct Thread
{
	TaskHandle_t task;
} Thread;

int ThreadStart(Thread*, void (*fn)(void*), void* arg);

int FreeRTOS_read(Network*, unsigned char*, int, int);
int FreeRTOS_write(Network*, unsigned char*, int, int);
void FreeRTOS_disconnect(Network*);

void NetworkInit(Network*);
int NetworkConnect(Network*, char*, int);
/*int NetworkConnectTLS(Network*, char*, int, SlSockSecureFiles_t*, unsigned char, unsigned int, char);*/

#endif

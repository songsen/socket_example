/*******************************************************************************
 * Copyright (c) 2012, 2018 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Guilherme Maciel Ferreira - add keep alive option
 *******************************************************************************/

 /*
 stdin publisher

 compulsory parameters:

  --topic topic to publish on

 defaulted parameters:

	--host localhost
	--port 1883
	--qos 0
	--delimiters \n
	--clientid stdin-publisher-async
	--maxdatalen 100
	--keepalive 10

	--userid none
	--password none

*/

#include "MQTTAsync.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

#if defined(WIN32)
#include <windows.h>
#define sleep Sleep
#else
#include <unistd.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif


#include <pthread.h>
#include <ctype.h>

#include"ConfigParse.h"
//subscribe 
#define SUBSCRIBE 1

volatile int toStop = 0;
//volatile int finished = 0;
//char* topic_sub = NULL;
int subscribed = 0;
int disconnected = 0;

#define finished toStop
//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t  flag = PTHREAD_COND_INITIALIZER;
pthread_mutex_t conLock = PTHREAD_MUTEX_INITIALIZER;
// struct
// {
// 	char* clientid;
// 	char* delimiter;
// 	int maxdatalen;
// 	int qos;
// 	int retained;
// 	char* username;
// 	char* password;
// 	char* host;
// 	char* port;
// 	int verbose;
// 	int keepalive;
// } opts =
// {
// 	"stdin-publisher-async", "\n", 100, 0, 0, NULL, NULL, "localhost", "1883", 0, 10
// };


FileParseOpts opts;

void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}

//void getopts(int argc, char** argv);

int messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
	/* not expecting any messages */
	printf("messageArrived\n");
#if SUBSCRIBE
	if (opts.showtopics)
		printf("%s\t", topicName);
	if (opts.nodelimiter)
		printf("%.*s", message->payloadlen, (char*)message->payload);
	else
		printf("%.*s%c", message->payloadlen, (char*)message->payload, opts.delimiter);
	fflush(stdout);
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
#endif
	return 1;
}

#if SUBSCRIBE
void onSubscribe(void* context, MQTTAsync_successData* response)
{
	subscribed = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Subscribe failed, rc %d\n", response->code);
	finished = 1;
}
#endif

void deliveryComplete(void* context, MQTTAsync_token token)
{
	printf("deliveryComplete\n");
	return;
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
	disconnected = 1;
	printf("onDisconnect\n");
}

void onConnect(void* context, MQTTAsync_successData* response)
{
#if SUBSCRIBE
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	int rc;

	if (opts.showtopics)
		printf("Subscribing to topic %s with client %s at QoS %d\n", opts.topic_sub, opts.clientid, opts.qos);

	ropts.onSuccess = onSubscribe;
	ropts.onFailure = onSubscribeFailure;
	ropts.context = client;
	if ((rc = MQTTAsync_subscribe(client, opts.topic_sub, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe, return code %d\n", rc);
		finished = 1;
	}
#endif
	printf("Connected\n");
	pthread_mutex_unlock(&conLock);
	connected = 1;
}

static int connected = 0;
MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

void myconnect(MQTTAsync* client)
{
	
	//MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	int rc = 0;
	pthread_mutex_lock(&conLock);
	printf("Connecting\n");
	conn_opts.keepAliveInterval = opts.keepalive;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	//ssl_opts.enableServerCertAuth = 0;
	//conn_opts.ssl = &ssl_opts; need to link with SSL library for this to work
	conn_opts.automaticReconnect = 1;
	connected = 0;
	if ((rc = MQTTAsync_connect(*client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	//connected
	
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response ? response->code : -1);
	connected = -1;

	MQTTAsync client = (MQTTAsync)context;
	myconnect(client);
}




static int published = 0;

void onPublishFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Publish failed, rc %d\n", response ? -1 : response->code);
	published = -1;
}


void onPublish(void* context, MQTTAsync_successData* response)
{
	published = 1;
	//pthread_cond_signal(&flag);	/* raise the flag */
	printf("published\n");
}


void connectionLost(void* context, char* cause)
{
	MQTTAsync client = (MQTTAsync)context;
	printf("connect lost\n");
#if 1
	pthread_mutex_lock(&conLock);
	int rc = 0;
	#if 0
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	

	printf("Connecting\n");
	conn_opts.keepAliveInterval = 10;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	ssl_opts.enableServerCertAuth = 0;
	//conn_opts.ssl = &ssl_opts; need to link with SSL library for this to work
	#endif
	connected = 0;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&conLock);
#else
	if(MQTTASYNC_SUCCESS != MQTTAsync_reconnect(client))
	{
		printf("Failed to start connect, return code \n");
		exit(EXIT_FAILURE);
	}
#endif
}

//FILE *fdLog ;

int main(int argc, char** argv)
{
	MQTTAsync client;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

	
	//MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;

	char* buffer = NULL;
	int rc = 0;
	
	GetOptFromFile(&opts);

	if (opts.verbose)
		printf("URL is %s\n", opts.url);

	//create_opts.sendWhileDisconnected = 1;
	//rc = MQTTAsync_createWithOptions(&client, opts.url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);
	rc = MQTTAsync_create(&client, opts.url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	
	rc = MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, deliveryComplete);

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	//pthread_mutex_lock(&lock);	/* lock the report box now */


	//myconnect(&client);

	conn_opts.keepAliveInterval = opts.keepalive;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	buffer = malloc(opts.maxdatalen);

	pthread_mutex_lock(&conLock);
	pthread_mutex_unlock(&conLock);
	//printf("conned = %d",connected);
	//while(!connected)
	//	sleep(1);
	//printf("conned = %d",connected);

#if SUBSCRIBE
	while (!subscribed)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

	if (finished)
		goto exit;

	while (!finished)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif
#endif	
	MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
	while (!toStop)
	{


		bzero(buffer,opts.maxdatalen);
		FILE *fd = popen(opts.message_producers,"r");
    	fgets(buffer,opts.maxdatalen,fd);

	#if 0
		int data_len = 0;
		int delim_len = 0;

		delim_len = (int)strlen(opts.delimiter);
		do
		{
			buffer[data_len++] = getchar();
			if (data_len > delim_len)
			{
				/* printf("comparing %s %s\n", opts.delimiter, &buffer[data_len - delim_len]); */
				if (strncmp(opts.delimiter, &buffer[data_len - delim_len], delim_len) == 0)
					break;
			}
		} while (data_len < opts.maxdatalen);
	#endif

		if (opts.verbose)
				printf("Publishing data of length %d\n", (int)strlen(buffer));


		pub_opts.onSuccess = onPublish;
		pub_opts.onFailure = onPublishFailure;

		do
		{
			pthread_mutex_lock(&conLock);
			pthread_mutex_unlock(&conLock);
			rc = MQTTAsync_send(client, opts.topic_pub, strlen(buffer), buffer, opts.qos, opts.retained, &pub_opts);
		}
		while (rc != MQTTASYNC_SUCCESS);
		//pthread_cond_wait(&flag, &lock); /* wait for notify */
		sleep(opts.interval);
	}

	printf("Stopping\n");

	free(buffer);

	disc_opts.onSuccess = onDisconnect;
	if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	while	(!disconnected)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif
exit:
	MQTTAsync_destroy(&client);
	freeFileParseOpts(&opts);
	return EXIT_SUCCESS;
}
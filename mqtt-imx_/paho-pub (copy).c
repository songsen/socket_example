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
#include<time.h>
 #include <sys/types.h>
       #include <sys/wait.h>

//subscribe 
#define SUBSCRIBE 1

volatile int toStop = 0;

int subscribed = 0;
int disconnected = 0;

static int published = 0;
static int connected = 0;

FILE *fdLog ;

#define finished toStop

FileParseOpts opts;

MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
pthread_mutex_t conLock = PTHREAD_MUTEX_INITIALIZER;


#define LOGA_DEBUG 0
#define LOGA_INFO 1
#define LOG_TOFILE 2
#define LOG_TOSTDIN 3

#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>

void MyLog(int LOGA_level, char* format, ...)
{
	static char msg_buf[256];
	va_list args;
	struct timeb ts;

	struct tm *timeinfo;

	if (LOGA_level == LOGA_DEBUG && opts.verbose == 0)
	  return;
	
	ftime(&ts);
	timeinfo = localtime(&ts.time);
	strftime(msg_buf, 80, "%Y%m%d %H%M%S", timeinfo);

	sprintf(&msg_buf[strlen(msg_buf)], ".%.3hu ", ts.millitm);

	va_start(args, format);
	vsnprintf(&msg_buf[strlen(msg_buf)], sizeof(msg_buf) - strlen(msg_buf), format, args);
	va_end(args);

	printf("%s\n", msg_buf);

	if (LOGA_level == LOG_TOFILE){
		pthread_mutex_lock(&conLock);
		fprintf(fdLog,"%s\n",msg_buf);
		pthread_mutex_unlock(&conLock);
	}
}

void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}


void onPublish(void* context, MQTTAsync_successData* response)
{
	published = 1;
	//pthread_cond_signal(&flag);	/* raise the flag */
	MyLog(LOGA_DEBUG,"published");
}


int messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
	MyLog(LOGA_DEBUG,"messageArrived");
	
#if SUBSCRIBE
	if (opts.showtopics)
		printf("%s\t\n", topicName);
	if (opts.nodelimiter)
		printf("%.*s\n", message->payloadlen, (char*)message->payload);
	else
		printf("%.*s%c\n", message->payloadlen, (char*)message->payload, opts.delimiter);
	fflush(stdout);
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
#endif

	return 1;
}

#if SUBSCRIBE
void onSubscribeSuccess(void* context, MQTTAsync_successData* response)
{
	MyLog(LOG_TOFILE,"subscribe successed,%d:",context);
	subscribed = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	MyLog(LOG_TOFILE,"Subscribe failed,%d:",context);
	finished = 1;
}
#endif

void deliveryComplete(void* context, MQTTAsync_token token)
{
	MyLog(LOGA_DEBUG,"deliveryComplete");
	
	return;
}

void myconnect(MQTTAsync client)
{
	int rc = 0;
	MyLog(LOG_TOFILE,"Connecting:",client);
	connected = 0;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		MyLog(LOG_TOFILE,"Failed to start connect, return code %d", rc);
		//exit(EXIT_FAILURE);
		finished = 1;
	}
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	MyLog(LOG_TOFILE,"Connect failed, rc %d", response ? response->code : -1);
	connected = 0;
	finished = 1;
	//MQTTAsync client = (MQTTAsync)context;
	//myconnect(client);
}

void onConnect(void* context, MQTTAsync_successData* response)
{
	MyLog(LOG_TOFILE,"Connected:%d",context);
	connected = 1;

#if SUBSCRIBE
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	int rc;

	if (opts.showtopics)
		MyLog(LOG_TOFILE,"Subscribing to topic %s with client %s at QoS %d", opts.topic_sub, opts.clientid, opts.qos);

	ropts.onSuccess = onSubscribeSuccess;
	ropts.onFailure = onSubscribeFailure;
	ropts.context = client;
	if ((rc = MQTTAsync_subscribe(client, opts.topic_sub, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		MyLog(LOG_TOFILE,"Failed to start subscribe, return code %d", rc);
		finished = 1;
	}
#endif
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
	MyLog(LOG_TOFILE,"onDisconnect");
	disconnected = 1;
}

void onPublishFailure(void* context, MQTTAsync_failureData* response)
{
	MyLog(LOG_TOFILE,"Publish failed, rc %d", response ? -1 : response->code);
	if(response != NULL){
		MyLog(LOG_TOFILE,"Publish failed, message: %s", response->message);
		MyLog(LOG_TOFILE,"Publish failed, token: %d", response->token);
	}

	published = -1;
}


void connectionLost(void* context, char* cause)
{
	int rc = 0;
	connected = 0;
	MQTTAsync client = (MQTTAsync)context;
	
	MyLog(LOG_TOFILE,"connect lost:%d,cause:%s",context,cause);
#if 1
	//pthread_mutex_lock(&conLock);
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		MyLog(LOG_TOFILE,"Failed to start connect, return code %d", rc);
		//exit(EXIT_FAILURE);
		finished = 1;
	}
	//pthread_mutex_unlock(&conLock);
#else
	if(MQTTASYNC_SUCCESS != MQTTAsync_reconnect(client))
	{
		MyLog(LOG_TOFILE,"Failed to start connect, return code ");
		exit(EXIT_FAILURE);
	}
#endif
}

void zombie_cleaning(int signo)
{
	int status;
	(void)signo;
	while (waitpid(-1, &status, WNOHANG) > 0);
	printf("zombie is comming\n");
 }

int main(int argc, char** argv)
{
	MQTTAsync client;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	
	MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
	
	char* buffer = NULL;
	int rc = 0;

	GetOptFromFile(&opts);
	
	if (opts.verbose)
		printf("URL is %s\n", opts.url);

	//create_opts.sendWhileDisconnected = 1;
	rc = MQTTAsync_createWithOptions(&client, opts.url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);

	rc = MQTTAsync_create(&client, opts.url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	rc = MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, deliveryComplete);

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
	//signal(SIGCHLD,zombie_cleaning);
	fdLog = fopen("mqtt-imx.log","w");
		
	//MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
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

	myconnect(client);

#if SUBSCRIBE

	while (!subscribed){
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif
		waitpid(-1, NULL, WNOHANG);
	}


	if (finished)
		goto exit;

#endif

	buffer = malloc(opts.maxdatalen);
	MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
	pub_opts.onSuccess = onPublish;
	pub_opts.onFailure = onPublishFailure;
	int status;
	while (!toStop)
	{
		//pthread_cond_wait(&flag, &lock); /* wait for notify */
		sleep(opts.interval);
		//waitpid(-1, NULL, WNOHANG);
		while (waitpid(-1, &status, WNOHANG) > 0);

		if(!connected)
			continue;

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
				/* printf("comparing %s %s", opts.delimiter, &buffer[data_len - delim_len]); */
				if (strncmp(opts.delimiter, &buffer[data_len - delim_len], delim_len) == 0)
					break;
			}
		} while (data_len < opts.maxdatalen);
	#endif


		if (opts.verbose)
				printf("Publishing data of length %d\n", (int)strlen(buffer));

		do
		{

			if(connected == 1)
				rc = MQTTAsync_send(client, opts.topic_pub, strlen(buffer), buffer, opts.qos, opts.retained, &pub_opts);
		}
		while (rc != MQTTASYNC_SUCCESS);
	}

	MyLog(LOG_TOFILE,"Stopping");

	free(buffer);

exit:
	disc_opts.onSuccess = onDisconnect;
	if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
	{
		MyLog(LOG_TOFILE,"Failed to start disconnect, return code %d", rc);
		exit(EXIT_FAILURE);
	}

	while	(!disconnected){
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif
		waitpid(-1, NULL, WNOHANG);
	}


	MQTTAsync_destroy(&client);
	freeFileParseOpts(&opts);
	fflush(stdout);
	fclose(fdLog);
	return EXIT_SUCCESS;
}
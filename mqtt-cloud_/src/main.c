#include<string.h>
#include<stdio.h>
#include<strings.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

#include "ConfigParse.h" 

#if 0
int main(int argc,char **argv)
{
    FileParseOpts opts ;//= FileParseOpts__initializer;
    GetOptFromFile(&opts);
    printf("clientid:%s\n",opts.clientid);
    printf("host    :%s\n",opts.host);
    printf("port    :%s\n",opts.port);
    printf("username:%s\n",opts.username);
    printf("password:%s\n",opts.password);
    printf("URL     :%s\n",opts.url);
    freeFileParseOpts(&opts);
}
#else

/*******************************************************************************
 * Copyright (c) 2012, 2017 IBM Corp.
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
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>
#include <time.h>

#if 0
#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "new"
#define PAYLOAD     "Hello World!"
#define USERNAME    "imx"
#define PASSWORD     "yss"
#define QOS         1
#define TIMEOUT     10000L
#endif

volatile MQTTClient_deliveryToken deliveredtoken;
volatile int static sendCompleteOK = 0;
#define PAYLOAD     "Hello World!"

#define TIMEOUT     10000L

volatile int toStop = 0;
MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
FileParseOpts opts ;//= FileParseOpts__initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;

FILE *fdLog ;
const char* logpath = "/tmp/mqtt-imx.log";

void writeLog(char* log,void* message,size_t size)
{
    time_t   timep;   
    time(&timep);   

    fprintf(fdLog,"%s: %s\n",ctime(&timep),log);
    fwrite(message,sizeof(char),size,fdLog);
}

int SendTask()
{
    int ret;
    if(sendCompleteOK > 0)
        return -1;
    sendCompleteOK = 1;
    //ch = getchar();
    char inbuff[256] ={0};
    // bzero(inbuff,256);
// if(fgets(inbuff,256,stdin)!=NULL){
    FILE *fd = popen(opts.message_producers,"r");
    fgets(inbuff,256,fd);
    pubmsg.payload = inbuff;
    pubmsg.payloadlen = strlen(inbuff);
    pubmsg.qos = opts.qos;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(client, opts.topic_pub, &pubmsg, &token);
    // printf("Waiting for up to %d seconds for publication of %s\n"
    //         "on topic_sub %s for client with ClientID: %s\n",
    //         (int)(TIMEOUT/1000), inbuff, opts.topic_sub, opts.clientid);
    ret = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    sendCompleteOK = 0;
    return ret ;
//}
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    //int i;
    //char* payloadptr;

    // printf("Message arrived\n");
    // printf("   subscribe topic: %s\n", topicName);
    // printf("message: ");

    // payloadptr = message->payload;
    // for(i=0; i<message->payloadlen; i++)
    // {
    //     putchar(*payloadptr++);
    // }
    // putchar('\n');

    writeLog(topicName,message->payload,message->payloadlen);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    SendTask();
    return 1;
}

void connlost(void *context, char *cause)
{
   // int rc;

    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
    // if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	// {
	// 	printf("Failed to start connect, return code %d\n", rc);
	// 	toStop = 1;
	// }
}


void disconnect(int i)
{
    printf("CTRL + C quit \n");
    toStop = 1;
}

int main(int argc, char* argv[])
{
    int rc;
    //int ch;

    signal(SIGINT,disconnect);
    GetOptFromFile(&opts);

    printf("%s:%s:%s:%s:%s",opts.url,opts.username,opts.password,opts.clientid,opts.topic_sub);

    MQTTClient_create(&client, opts.url, opts.clientid,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = opts.keepalive;
    conn_opts.cleansession = 1;
    conn_opts.username = opts.username;
    conn_opts.password = opts.password;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Subscribing to topic_sub %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", opts.topic_sub, opts.clientid, opts.qos);
    MQTTClient_subscribe(client, opts.topic_sub, opts.qos);

    fdLog = fopen(logpath,"a");    
    do 
    {
        SendTask();
        sleep(opts.interval);
    } while(!toStop);

    MQTTClient_unsubscribe(client, opts.topic_sub);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    freeFileParseOpts(&opts);
    return rc;
}


#endif
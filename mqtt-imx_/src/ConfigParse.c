#include<string.h>
#include<stdio.h>
#include<strings.h>
#include<stdlib.h>
#include<errno.h>

#include "ConfigParse.h"

const char* configFIle = "mqtt-imx.conf" ;

void usage(FileParseOpts *opts)
{
	printf("MQTT stdin publisher\n");
	printf("Usage: stdinpub topicname <options>, where options are:\n");
	printf("  --host <hostname> (default is %s)\n", opts->host);
	printf("  --port <port> (default is %s)\n", opts->port);
	printf("  --qos <qos> (default is %d)\n", opts->qos);
	printf("  --retained (default is %s)\n", opts->retained ? "on" : "off");
	printf("  --delimiter <delim> (default is \\n)\n");
	printf("  --clientid <clientid> (default is %s)\n", opts->clientid);
	printf("  --maxdatalen <bytes> (default is %d)\n", opts->maxdatalen);
	printf("  --username none\n");
	printf("  --password none\n");
	printf("  --keepalive <seconds> (default is 10 seconds)\n");
	exit(-1);
}


int GetOptFromFile(FileParseOpts *opts)
{
    int ret;
    char *buff ,*para;
    
   // char buff[BUFF_SIZE] = {0};

    buff = calloc(sizeof(char),BUFF_SIZE); 
    if(buff ==NULL){     
        perror("calloc error");     
        exit(EXIT_FAILURE); 
    }  

    // opts = calloc(sizeof(FileParseOpts),1); 
    // if(opts ==NULL){     
    //     perror("calloc error");     
    //     exit(EXIT_FAILURE); 
    // }

    opts->maxdatalen = 100;
    opts->qos = 0;
    opts->retained = 0;
    opts->verbose = 0;
    opts->keepalive = 10;  
    opts->clientid = NULL;
    opts->delimiter = '\n';
    opts->url = NULL;  
    opts->topic_sub = NULL;
    opts->topic_pub = NULL;
    opts->interval = 10;
    opts->message_producers = NULL;
    opts->showtopics = 1;

    FILE *conf = fopen(configFIle,"r");

    while(fgets(buff,BUFF_SIZE,conf)!= NULL){

        if(strcmp(buff,"\n")==0){
            continue;
        }
        para = strtok(buff," = \n\r");
        if(para == NULL){
            continue;
        }

        do{
            if( para[0] =='#'){
                break;
            }

            ret++;

            if(strcmp(para,"topic_sub")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->topic_sub = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->topic_sub ==NULL){     perror("calloc error");    
                    exit(EXIT_FAILURE); }
                    strncpy(opts->topic_sub,para,strlen(para)+1);}
                break;
            } 
            else if(strcmp(para,"message_producers")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->message_producers = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->message_producers ==NULL){     perror("calloc error");    
                    exit(EXIT_FAILURE); }
                    strncpy(opts->message_producers,para,strlen(para)+1);}
                break;
            }
            else if(strcmp(para,"topic_pub")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->topic_pub = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->topic_pub ==NULL){     perror("calloc error");    
                    exit(EXIT_FAILURE); }
                    strncpy(opts->topic_pub,para,strlen(para)+1);}
                break;
            }
            else if(strcmp(para,"url")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->url = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->url ==NULL){     perror("calloc error");    
                    exit(EXIT_FAILURE); }
                    //opts->url = loc;;
                    strcpy(opts->url,para);}
                break;
            }
            else if(strcmp(para,"host")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->host = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->host ==NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
                    //opts->host = loc;;
                    strncpy(opts->host,para,strlen(para)+1);}
                    break;
            }
            else if(strcmp(para,"port")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->port = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->port ==NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
                    //opts->port = loc;;
                    strncpy(opts->port,para,strlen(para)+1);}
                break;
            }
            else if(strcmp(para,"username")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->username  = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->username  ==NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
                    //opts->username = loc;;
                    strncpy(opts->username,para,strlen(para)+1);}
                break;
            }
            else if(strcmp(para,"password")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->password = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->password ==NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
                    //opts->password = loc;;
                    strncpy(opts->password,para,strlen(para)+1);}
                break;
            }
            else if(strcmp(para,"client_id")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    {opts->clientid = calloc(sizeof(char),strlen(para)+1); 
                    if(opts->clientid ==NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
                //opts->clientid = loc;;
                strncpy(opts->clientid,para,strlen(para)+1);}
                break;
            }
            else if(strcmp(para,"qos")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    opts->qos = atoi(para);
                break;
            }
            else if(strcmp(para,"maxdatalen")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    opts->maxdatalen = atoi(para);
                break;
            }
            else if(strcmp(para,"retained")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    opts->retained = atoi(para);
                break;
            }
            else if(strcmp(para,"verbose")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    opts->verbose = atoi(para);
                break;
            }
            else if(strcmp(para,"keepalive")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    opts->keepalive = atoi(para);
                break;
            }
            else if(strcmp(para,"interval")==0){
                para = strtok(NULL," =\n\r");
                if(para != NULL)
                    opts->interval = atoi(para);
                break;
            }
            else{
                usage(opts);
                exit(EXIT_FAILURE);
            }
            
        }while((para = strtok(NULL," =\n\r")) != NULL );

        bzero(buff,BUFF_SIZE);
    }

    if(opts->clientid == NULL){
        opts->clientid = calloc( sizeof(char),sizeof("stdin-publisher") );
        if(opts->clientid == NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
        strcpy(opts->clientid,"stdin-publisher-async");
    }

    if(opts->url == NULL){
        opts->url = calloc( sizeof(char),sizeof("localhost:1883") );
        if(opts->url == NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
        strcpy(opts->url,"localhost:1883");
    }

    if(opts->topic_sub == NULL){
        opts->topic_sub = calloc( sizeof(char),sizeof("new") );
        if(opts->topic_sub == NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
        strcpy(opts->topic_sub,"new");
    }
    //message_producers
    if(opts->message_producers == NULL){
        opts->message_producers = calloc( sizeof(char),sizeof("date") );
        if(opts->message_producers == NULL){     perror("calloc error");     exit(EXIT_FAILURE); }
        strcpy(opts->message_producers,"date");
    }   

    fclose(conf);
    free(buff);
    return ret;
}

void freeFileParseOpts(FileParseOpts *opts)
{
    if(opts == NULL )
        return;

    if(opts->username != NULL)
        free(opts->username);
    if(opts->password != NULL)
        free(opts->password);
    if(opts->clientid != NULL)
        free(opts->clientid);
    if(opts->url != NULL)
        free(opts->url);
    if(opts->topic_sub != NULL)
        free(opts->topic_sub);
    if(opts->message_producers != NULL)
        free(opts->message_producers);
    //free(opts);
}


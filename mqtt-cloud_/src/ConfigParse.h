#ifndef _CONFIG_PARSE_
#define _CONFIG_PARSE_

#define BUFF_SIZE 256
#define OPTS_SIZE 128

typedef struct
{
	char* clientid;
	char delimiter;
	int maxdatalen;
	int qos;
	int retained;
	char* username;
	char* password;
	char* host;
	char* port;
	int verbose;
	int keepalive;
	char* url;
	char* topic_pub;
	char* topic_sub;
	int interval;
	char* message_producers;
	int showtopics;
	int nodelimiter;
} FileParseOpts;

#define FileParseOpts__initializer \
{\
	.clientid="stdin-publisher-async",\
    .delimiter= '\n', \
    .maxdatalen= 100, \
    .qos= 0,\
    .retained= 0,\
    .username= NULL,\
    .password= NULL,\
    .host= "localhost",\
    .port= "1883",\
    .verbose= 0,\
    .keepalive= 10\
};

int GetOptFromFile(FileParseOpts *opts);
void freeFileParseOpts(FileParseOpts *opts);

#endif
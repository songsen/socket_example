#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<stdbool.h>
#include<stdint.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<time.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT ((uint16_t)7008)
#define BUFF_SIZE	(1024*8)

char * get_time()
{
	time_t now;
	struct tm *timenow;
	
	time(&now);
	timenow = localtime(&now);
	
	return asctime(timenow);
}

int main(int argc, char *argv[])
{
	int fd_sock;
	char buff_str[BUFF_SIZE] = "client will be connect server";
	struct sockaddr_in address_server;
	
	(void)memset(&address_server,0,sizeof(address_server));
	address_server.sin_family = AF_INET;
	
	if(argc == 1)
	{	
		address_server.sin_addr.s_addr=inet_addr(SERVER_IP);
		address_server.sin_port	  = htons(SERVER_PORT);
	}	
	else
	{
		char *p ;
		p = strtok(argv[1],":");

		if( p!= NULL)
			address_server.sin_addr.s_addr=inet_addr(p);
		else{
			printf("usage : ip:port");
			exit(-1);
		}

		p = strtok(NULL,":");
		if( p != NULL)
			address_server.sin_port	  = htons(atoi(p));
		else{
			printf("usage : ip:port");
			exit(-1);
		}
		//snprintf(buff_str,BUFF_SIZE,"%s",argv[2]);
	}

	fd_sock = socket(AF_INET,SOCK_STREAM,0);
	if(fd_sock<0){
		perror("socket(2) error");
		goto create_fault;			
	}

	if(connect(fd_sock,	(struct sockaddr *)&address_server,sizeof(address_server))<0){
		perror("connect(2) error");
		goto error;
	}

	while(1)
	{
		(void)memset(buff_str,0,BUFF_SIZE);
		fgets(buff_str,sizeof(buff_str),stdin);

		if(write(fd_sock,buff_str,strlen(buff_str))<0){
			perror("send data error");
			break;
		}

		if(!strcasecmp(buff_str,"quit\n")){
			exit(0);
		}
		
		(void)memset(buff_str,0,BUFF_SIZE);
		if(read(fd_sock,buff_str,BUFF_SIZE)<0){
			perror("recevie data error");
			break;
		}
		
		printf("%s",get_time());
		printf("%s	",buff_str);
	}
	shutdown(fd_sock,SHUT_WR);
	shutdown(fd_sock,SHUT_RD);
	return 1;

	error:close(fd_sock);
	create_fault:fprintf(stderr,"client error");
	exit:	return EXIT_FAILURE;	
}

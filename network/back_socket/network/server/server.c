#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<arpa/inet.h>

int main(int argc, char *argv[])
{
	int sockfd,acceptfd,portnumber;
	struct sockaddr_in srv_addr, clt_addr;
	socklen_t addrlen=sizeof(clt_addr);
	
	char rxbuff[1024*8];
	char reply[]="input:\n";
	if(2!=argc || 0> (portnumber=atoi(argv[1])))
	{
		printf("Usage:%s prot\n",argv[0]);
		exit(1);
	}
	
	printf("listenning port:%s\n", argv[1]);	
	
	//creat ipv4 steam 
	if(-1==(sockfd=socket(AF_INET,SOCK_STREAM,0))){
		perror("socket creat error: ");
		exit(1);
	}
	
	//full server sockaddr struct;
	bzero(&srv_addr,sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(portnumber);

	if(-1==bind(sockfd,(struct sockaddr *)(&srv_addr),sizeof(struct sockaddr))){
		perror("bind error:");
	}
	
	if(-1==listen(sockfd,5)){
		perror("listen error:");
	}
	
	while(1){
		if(-1 == (acceptfd=accept(sockfd,(struct sockaddr *)(&clt_addr),&addrlen))){
			perror("accept error:");
			close(sockfd);
			exit(1);
		}
		
		printf("come from %s:%u's user say:\n",inet_ntoa(clt_addr.sin_addr),ntohs(clt_addr.sin_port));
		fflush(stdout);

		ssize_t rxlen=0;
		bzero(rxbuff,sizeof(rxbuff));
		rxlen=read(acceptfd,rxbuff,sizeof(rxbuff));
		if(-1 == rxlen){
			printf("read error:");
			close(acceptfd);
			close(sockfd);
			exit(1);
		}
		
		if(-1 ==  write(acceptfd,reply,strlen(reply))){
			perror("send error:");
			close(acceptfd);
			close(sockfd);
			exit(1);
		}
		shutdown(acceptfd,SHUT_RD);
		fflush(stdin);		

		printf("	%s",rxbuff);
		fflush(stdout);
	
		close(acceptfd);
		if(!strcasecmp(rxbuff,"quit\n")){
			break;
		}
	}
	close(sockfd);
	exit(0);
}

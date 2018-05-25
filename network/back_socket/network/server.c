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
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>
#include<string.h>

#define BUF_SIZE  1024*8

int chldproccess(pid_t chldfd, int sockfd);

void zombie_clear(int signo)
{
	int status;
	while(waitpid(-1,&status,WNOHANG)>0){
		printf("zombie is clearn");
	}
	printf("zombie is clearn");
}

int main(int argc, char *argv[])
{
	int sockfd,acceptfd,port;
	struct sockaddr_in srv_addr, clt_addr;
	socklen_t addrlen = sizeof(clt_addr);
	
	struct sigaction act;
	bzero(&act,sizeof(struct sigaction));
	act.sa_handler = zombie_clear;
	//act.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD,&act,NULL)<0){
		perror("sigaction ");
		exit(-1);
	}

	if(2 > argc ){
		printf("Usage:%s prot\n",argv[0]);
		exit(-1);
	}

	if(0> (port = atoi(argv[1]))){
		printf("Port Error\n");
		exit(-1);
	}
	
	//signal( SIGPIPE, SIG_IGN );
	printf("listenning port:%s\n", argv[1]);	
	
	//creat ipv4 steam 
	if(-1==(sockfd=socket(AF_INET,SOCK_STREAM,0))){
		perror("socket creat error: ");
		exit(-1);
	}
	
	//full server sockaddr struct;
	bzero(&srv_addr,sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(port);

	if(-1==bind(sockfd,(struct sockaddr *)(&srv_addr),sizeof(struct sockaddr))){
		perror("bind error:");
		exit(-1);
	}
	
	if(-1==listen(sockfd,5)){
		perror("listen error:");
		exit(-1);
	}
	
	while(1){
		if(-1 == (acceptfd=accept(sockfd,(struct sockaddr *)(&clt_addr),&addrlen))){
			//if(errno == EINTR) continue;
			perror("accept:");
			exit(-1);
		}

		printf("come from %s:%u's user say:\n",
		inet_ntoa(clt_addr.sin_addr),ntohs(clt_addr.sin_port));

		chldproccess(fork(),acceptfd);
		close(acceptfd);

	}
	close(sockfd);
	exit(0);
}


int chldproccess(pid_t chldfd, int sockfd)
{
	if(chldfd > 0){
		return chldfd;
	}else if( chldfd == -1){
		return -1;
	}

	char *buff = (char *)calloc(sizeof(char),BUF_SIZE);

	while(1){
		bzero(buff,sizeof(buff));
		if(read(sockfd,buff,sizeof(buff)) < 0){
			printf("read error:");
			break;
		}

		printf("%s",buff);

		if(-1 ==  write(sockfd,buff,strlen(buff))){
			perror("send error:");
			break;
		}	
			
		if(!strcasecmp(buff,"quit\n")){
			break;
		}
	}
	free(buff);
	close(sockfd);
	return getpid();
}

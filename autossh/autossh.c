/*
 * ssh对硬件/系统的要求很低,利用ssh做端口转发实用性广,几乎任何上Linux系统都自带ssh.
 * 
 * issue:
 * 1.由于在服务器端把ServerAliveInterval和ServerAliveCountMax写入sshd配置文件,会导致不安全,
 * 2.ssh长时间不用,路由会阻断ssh的长连接
 * 3.网络故障导致的网络中断ssh连接
 * 
 * 原理:
 * 可以通过服务器的反馈信息来,自动判断连接/端口是否有效,是否需要重新连接
 * 1. Connection refused 服务器没有监听目标端口, 可以reverse ssh
 * 2. forwarding failed  端口被占用, 需要关闭进程,然后尝试连接,若返回Connection reset则可以reverse ssh
 * Warning: remote port forwarding failed for listen port 2200
 * 3. Connection reset   服务器与客户端网络断开,服务器没有收到FIN , 再次连接可成功
 * ssh_exchange_identification: read: Connection reset by peer
 * EHOSTUNREACH   No route to host,可能是ip不存在
 * ETIMEDOUT   : 可能是由于防火墙屏蔽了端口
 * 
 * create data :2018-04-18 18:21:21 by Eason
 */

/*
 * command format: 
 * ./autosshlite ssh -N -o "ServerAliveInterval 60" -o "ServerAliveCountMax 3" -R  0.0.0.0:2202:localhost:22 eason@1.2.3.1
 * _________________________ 检测数据包的发送间隔 __________ 检测的最大发送次数 超过之后将会timeout 然后关掉ssh ______________________
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
 #include <sched.h>
#include <netdb.h>

#define TEST_STRING "http://songsen.top"
#define connrefuse "Connection refused"  //ECONNREFUSED
#define connfailed "forwarding failed"
#define connreset "Connection reset"     //ECONNRESET

typedef enum{
    CONN_INIT,
    CONN_SUCCESS,
    CONN_FAILED,
    CONN_REFUSE = ECONNREFUSED,
    CONN_RESET = ECONNRESET
} conn_statu;

typedef struct {
    int pid;   /*child pid*/
    int done;  /*thread loop flag*/
    int fd;    /*pipe redirect fd*/
    conn_statu statu; /*error number*/
} argpth;


/*
 *     detect child's status , and get error number
 */
void * getErrnoFromChild(void *arg);


/*
 *SYNOPSIS  
 *     @check the server's status 
 *     @host:server's ip
 *     @portnum:server's port number
 *     @rwbuf:
 *     @rwbuflen:
 * RETURN VALUE
 *     @These functions return a pointer to the errorno of the located substring, 
 *      or 0 if error not occur ,
*/
int getStatusFromServer(const char *host, int portnum,char *rwbuf,char rwbuflen);


/*
 * entry
*/
int main(int argc, char **argv)
{
	int  newpid, newfd, pipefd[2];
    int err, wait_rv;		/* return value from wait() */;
    pthread_t t1;
    argpth pritarg;
    char serip[32], serport[32];
    char retbuf[]=TEST_STRING;
   
    
    sched_yield();

    signal( SIGPIPE, SIG_IGN );

    bzero(serip,32);
    bzero(serport,32);
    pritarg.statu = CONN_INIT;

    char *c1;
    for(int i=0;i < argc;i++){
        c1 = strchr(argv[i],':');
        if( NULL != c1){
            c1++;
            for(int i=0;*c1!=':';i++,c1++)
                serport[i]=*c1;
        }
        c1 = strchr(argv[i],'@');
        if( NULL != c1){
            c1++;
            for(int i=0; *c1!=0;i++,c1++)
                serip[i]=*c1;
        }
    }

    printf("Will onnect to %s:%s\n",serip,serport);

    if(-1 == pipe(pipefd)){
        perror("pipe error");
        exit(-1);
    }

    for(int n=2;--n;){
        //diagnose connect issue
        sched_yield();
        strncpy(retbuf,TEST_STRING,sizeof(retbuf));
        printf("Checking server's status ... \n");

        err = getStatusFromServer(serip,atoi(serport),retbuf,sizeof(retbuf));
        if(0 == err){              //端口已经被占用
            printf("Server's port is used !\n");
            exit(-1);
        }else if((ETIMEDOUT == err) //ETIMEDOUT   : 可能是由于NAT防火墙屏蔽了端口
        ||(ENETUNREACH == err)      //ENETUNREACH : 本机有可能断网
        ||(EHOSTUNREACH == err)){   //EHOSTUNREACH:No route to host,可能是ip地址不存在
            printf("Trying it agian after 60 second, left %d\n",n);
            sleep(60);// 需要等待本机网络正常后在尝试;
            continue;
        }else if((ECONNREFUSED == err)
        ||(ECONNRESET == err)){
            printf("Server status:%s\nbuilding a connect pid: %d\n", retbuf,getpid());
        }else if(0 == n){
            continue;    
        }else{
            printf("Unknown reason.Trying it agian after 60 second, left %d\n",n);
            sleep(60);// 需要等待本机网络正常后在尝试;
            continue;
        }
        
        if ( (newpid = fork()) == -1 ){
            perror("fork error");
            exit(-1);
        }
        else if ( newpid == 0 ){
            close(pipefd[0]);
            if(-1 == dup2(pipefd[1],2)){
                perror("dup error");
                exit(-1);
            }
            execvp(argv[1],argv+1);
            close(pipefd[1]);
        }else{
            close(pipefd[1]);

            pritarg.done = 1;
            pritarg.pid = newpid;
            pritarg.fd = pipefd[0];
            pthread_create(&t1,NULL,getErrnoFromChild,(void *)&pritarg);

	        wait_rv = wait(NULL);

            pritarg.done = 1;
            pthread_join(t1,NULL);

            //kill(newpid,SIGKILL);
            close(pipefd[0]);
        }
    }	
}

/* 
 * get status fo server 
 */
int getStatusFromServer(const char *host, int portnum,char *rwbuf,char rwbuflen)
{
	int sockfd;
	struct sockaddr_in  servadd;        /* the number to call */
	//struct hostent      *hp;            /* used to get number */
   // char [64] = "test";
	/** Step 1: Get a socket **/

	sockfd = socket( AF_INET, SOCK_STREAM, 0 );    /* get a line   */
	if ( sockfd == -1 ) 
		return errno;

    //unsigned int timeout =10000;
    // if(-1 == setsockopt(sockfd,IPPROTO_TCP,TCP_USER_TIMEOUT,&timeout,sizeof(timeout))){
    //     perror("setsockopt error");
    // }
	/** Step 2: connect to server **/
    int synRetring =2; //connect 尝试2次(7s)后返回 , 默认是尝试7次共127秒
    if(setsockopt(sockfd,IPPROTO_TCP,TCP_SYNCNT,&synRetring,sizeof(synRetring))<0){
        perror("setsockopt err");
    }

    bzero( &servadd, sizeof(servadd) );     /* zero the address     */
    struct hostent      *hp;            /* used to get number */
    hp = gethostbyname( host );            /* lookup host's ip #   */
	if (hp == NULL) 
		perror(host);            	/* or die               */
	bcopy(hp->h_addr, (struct sockaddr *)&servadd.sin_addr, hp->h_length);

    // if(inet_aton(host,&servadd.sin_addr)<=0)
    //     return errno;

	servadd.sin_port = htons(portnum);      /* fill in port number  */
	servadd.sin_family = AF_INET ;          /* fill in socket type  */

    // int	flags = fcntl(sockfd, F_GETFL, 0);
	// fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	if ( connect(sockfd,(struct sockaddr *)&servadd, sizeof(servadd)) !=0){
        strncpy(rwbuf,"test ok!",rwbuflen);
        perror("connect error");
        close(sockfd);
        return errno;
    }

    struct timeval timeout = {3,0};
    if(setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout))<0){  /*设置写socket 超时*/
        perror("setsockopt err");
    }
    if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout))<0){  /*设置读socket 超时*/
        perror("setsockopt err");
    }

    if(write(sockfd,rwbuf,strlen(rwbuf))<0){
        strncpy(rwbuf,"test ok!",rwbuflen);
        perror("send data error");
        close(sockfd);
        return errno;
    }
    bzero(rwbuf,rwbuflen);
    if(read(sockfd,rwbuf,rwbuflen)<0){
        strncpy(rwbuf,"test ok!",rwbuflen);
        perror("read error:");
        close(sockfd);
        return errno;
    }

    close(sockfd);
	return errno;
}

/*
 *  check connect status
 */
void * getErrnoFromChild(void *arg)
{
    argpth *para;
    int n ;
    FILE *fds;
    char *rd;
    char buf[256];

    para = (argpth *)arg;
    fds = fdopen(para->fd,"r");

    while(para->done){
        bzero(buf,256);
        rd = fgets(buf,256,fds);
        if(rd == NULL){
            break;
        }
        printf("info:%s",rd);
        rd = strstr(buf,connfailed);
        if(rd != NULL){
            para->statu = CONN_FAILED;
            kill(para->pid,SIGKILL);
            printf("get error: %s\n",connfailed);
            break;
        }
        rd = strstr(buf,connrefuse);
        if(rd != NULL){
            para->statu = CONN_REFUSE;
            kill(para->pid,SIGKILL);
            printf("get error: %s\n",connrefuse);
            break;
        }
        rd = strstr(buf,connreset);
        if(rd != NULL){
            para->statu = CONN_RESET;
            kill(para->pid,SIGKILL);
            printf("get error: %s\n",connreset);
            break;
        }
    }
    return NULL;
}

/*
 * 开机网络检测
 */


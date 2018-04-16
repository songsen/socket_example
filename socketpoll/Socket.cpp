#include "Socket.h"
#include <exception>
#include <iostream>
#include <errno.h>
#include <stdarg.h>
#include "csocket.h"
#include	<limits.h>		/* for OPEN_MAX */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
//#include <curses.h>
// extern "C"{
//     int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
// }


Socket::Socket(int portnum,int backlog, int domain , int type)
{
    this->listenfd = socket(domain,type,0);
    if(listenfd == -1){
        perror("socket error");
        throw errno;
    }
    int on=1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero((void *)&saddr,sizeof(struct sockaddr_in));
    saddr.sin_port = htons(portnum);
    saddr.sin_family = domain;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd,(struct sockaddr*)&saddr,sizeof(saddr))){
        perror("bind error:");
        throw errno;
    }

    if(listen(listenfd,backlog)){
        perror("listen error:");
        throw errno;
    }

    logfd = open("/tmp/webtest.log",O_WRONLY|O_CREAT|O_APPEND);
    if(logfd < 0){
        perror("fopen log error");
        throw errno;
    }
    dup2(logfd,2);

    perror("test dup retrict");

   // pll = new ThreadPoll(10);
}

Socket::Socket(const char *host,int portnum,int domain , int type)
{
    connfd = socket(domain,type,0);
    if(connfd == -1){
        perror("socket error");
        throw errno;;
    }

    bzero((void *)&caddr,sizeof(struct sockaddr_in));
    caddr.sin_family = domain;
    caddr.sin_port = htons(portnum);
    if(inet_aton(host,&caddr.sin_addr)<=0){
        perror("aton error");
    }
    //printf("%s\n",inet_ntoa(caddr.sin_addr));
   // caddr.sin_addr.s_addr = inet_addr(host);

    if(connect(connfd, (struct sockaddr *)&caddr,sizeof(caddr))){
        perror("connect error:");
        throw errno;;
    }
}

Socket::~Socket()
{
    ;
}

int Socket::getServFd()
{
    return listenfd;
}

int Socket::getClientFd()
{
    return connfd;
}
/*
 * install a handler, tell kernel who to notify on input, enable signals
 */
void enable_kbd_signals()
{
	int  fd_flags;

	fcntl(0, F_SETOWN, getpid());
	fd_flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, (fd_flags|O_ASYNC));
}

void on_input(int signum)
{
	int	c = getchar();		  /* grab the char */

	if ( c == 'Q' || c == EOF )
		exit(1);
}

void task(int fd)
{
            // strcpy(request,"this is serv\n \r\n \n xxxddfasdfxxxx yyyyy");
        // if(write(fd,request,strlen(request))<0){
        //     perror("send data error");
        //     throw errno;
        // }

        //int fd = (*(int*)afd);
        char request[BUFSIZ];

        // FILE	*fpin , *fpout;

        // fpin = fdopen(dup(fd), "r" );

        // /* read request */
        // fgets(request,BUFSIZ,fpin);
        // printf("got a call: request = %s\n", request);
        // //read_til_crnl(fpin);

        
        // fpout = fdopen(dup(fd), "w" );
        // fprintf(fpout, "HTTP/1.0 501 Not Implemented\r\n");
        // fprintf(fpout, "Content-type: text/plain\r\n");
        // fprintf(fpout, "\r\n");

        // fprintf(fpout, "That command is not yet implemented\r\n");
       

        /* do what client asks */
        //process_rq(request, fd);
        //strcpy(request,"HTTP/1.0 404 Not Found write \n\r");
        read(fd,request,BUFSIZ);
        write(fd,request,strlen(request));
       // fflush(fpout);

        // if(fclose(fpin)!=0){
        //     perror("fclose in error");
        // }
        // if(fclose(fpout)!=0){
        //      perror("fclose out error");
        // }
        close(fd);
}

int Socket::accept(){
    int 	sock, fd;

	char	request[BUFSIZ];
    socklen_t sock_len = sizeof(caddr);

    signal(SIGIO, on_input);          /* install a handler        */
	enable_kbd_signals();             /* turn on kbd signals      */

    while(true){
        /* take a call and buffer it */
        bzero((void *)&caddr,sizeof(caddr));

        fd = csocket::accept( listenfd, (struct sockaddr *)&caddr,&sock_len);
        if(fd < 0){
            perror("accept error");
            exit(-1);
        }

        printf("come from %s:%u's user say:\n",
        inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));

        //pll->pop_front();
        pll->push_back(task,fd);
        
    }
}

int Socket::readcbuf(char * str){
        int n;

        strcpy(request,str);

        if(writen(connfd,request,strlen(request))<0){
            perror("send data error");
            exit(-1);
           // throw errno;
        }

        if((n=read(connfd,recvbuf,BUFSIZ))<0){
            perror("read error:");
            exit(-1);
        }

        printf("%s",recvbuf);
        close(connfd);
      //  close(connfd)
        return n;
}


int Socket::WriteLog(const char * format, ...)
{
    logfile = fopen("/tmp/webtest.log","w");
    va_list arg_ptr;
    va_start(arg_ptr, format);
    int nWrittenBytes = vfprintf(logfile, format, arg_ptr);
    va_end(arg_ptr);
    return nWrittenBytes;
}


ssize_t Socket::writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = (const char	*)vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

int Socket::select()
{
    int					i, maxi, maxfd , connfd, sockfd,infd;
	int					nready, client[FD_SETSIZE] ,allconnfd;
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
    struct timeval interval ,timeout;
    char inbuf[16];

    snprintf(buf,MAXLINE,"I got it !");

    infd = open("/dev/tty",O_RDONLY);
    if(infd == -1){
        perror("fd open error");
    }

	maxfd = listenfd>infd?listenfd:infd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
    FD_SET(infd, &allset);
/* end fig01 */
    allconnfd = 0;
        /** set timeout value **/
    interval.tv_sec =  5;  /* set seconds */
    interval.tv_usec = 0;           /* no useconds */
/* include fig02 */
	for ( ; ; ) {
        timeout = interval; /*select有可能导致timeout值改变,因此需要每次初始化中间变量传递 */
		rset = allset;		/* structure assignment , select有可能导致allset改变,因此需要中间变量传递 */ 
		nready = csocket::select(maxfd+1, &rset, NULL, NULL,NULL);
        if(nready < 0){
            perror("select error:");
            exit(1);
        }else if(nready == 0){
            printf("select timeout \n");
            continue;
        }

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = csocket::accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
            if(connfd == -1){
                perror("accept error");
                continue;
            }
#ifdef	NOTDEF
			printf("new client: %s:%d,total:%d/%d\n",
					inet_ntoa(cliaddr.sin_addr),
					ntohs(cliaddr.sin_port),
                    allconnfd,maxi);
#endif
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
                    allconnfd++;
					break;
				}
			if (i == FD_SETSIZE)
				printf("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

        if(FD_ISSET(infd,&rset)){
            bzero(inbuf,sizeof(inbuf));
            int n = read(infd,inbuf,sizeof(inbuf));
            if(strcmp("quit\n",inbuf)==0){
                printf("exit ",inbuf);
                goto endmain;
            }else if(strcmp("client\n",inbuf)==0){
                printf("client id:\n");
                for (i = 0; i <= maxi; i++) {	/* check all clients for data */
                if ((sockfd = client[i]) > 0)
                    printf("%8d",i);
                }
            }
        }

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = read(sockfd, buf, MAXLINE)) <= 0) {
						/*connection closed by client */
                    if(n<0){
                        perror("server read error");
                    }

                    close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
                    allconnfd--;
				} else
					writen(sockfd, buf, n);

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
            //else{  //存在一些发生错误的连接占用了client[i]数组
//
           // }
            
		}
	}
endmain:
    for (i = 0; i <= maxi; i++) {	/* check all clients for data */
    if ((sockfd = client[i]) > 0)
        close(sockfd);
    }
    close(listenfd);
}
#if 0
int Socket::poll()
{
    int			        i, maxi, listenfd, connfd, sockfd;
	int					nready;
	ssize_t				n;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct pollfd		client[FD_SETSIZE];
	struct sockaddr_in	cliaddr, servaddr;

	for (i = 1; i < FD_SETSIZE; i++)
		client[i].fd = -1;		/* -1 indicates available entry */
	maxi = 0;					/* max index into client[] array */

    	client[0].fd = listenfd;
	client[0].events = POLLIN;
    
/* end fig01 */

/* include fig02 */
	for ( ; ; ) {
		nready = csocket::poll(client, maxi+1, 5000);
        if(nready == -1){
            perror("poll error");
        }else if(nready == 0){
            printf("poll timeout \n");
            continue;
        }

        printf("have message\n");
		if (client[0].revents & (POLLIN)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = csocket::accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
            if(connfd <0){
                perror("accepet error");
            }

#ifdef	NOTDEF
        printf("new client: %s:%d,total:%d/%d\n",
            inet_ntoa(cliaddr.sin_addr),
            ntohs(cliaddr.sin_port),
            i,maxi);
#endif

			for (i = 1; i < FD_SETSIZE; i++)
				if (client[i].fd < 0) {
					client[i].fd = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE)
				perror("too many clients");

			client[i].events = POLLIN;
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

		for (i = 1; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i].fd) < 0)
				continue;
			if (client[i].revents & (POLLIN | POLLERR)) {
				if ( (n = read(sockfd, buf, MAXLINE)) < 0) {
					if (errno == ECONNRESET) {
							/*4connection reset by client */
#ifdef	NOTDEF
						printf("client[%d] aborted connection\n", i);
#endif
						close(sockfd);
						client[i].fd = -1;
					} else
						perror("read error");
				} else if (n == 0) {
						/*4connection closed by client */
#ifdef	NOTDEF
					printf("client[%d] closed connection\n", i);
#endif
					close(sockfd);
					client[i].fd = -1;
				} else
					writen(sockfd, buf, n);

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}
#else 
int Socket::poll()
{
    int             timeout,nready, acceptfd,infd,maxi ,n;
    struct pollfd   fds[FD_SETSIZE],fdssock;
    nfds_t          nfds;
    char			buf[MAXLINE];
    struct sockaddr_in cliaddr;
    socklen_t clilen;

    signal( SIGPIPE, SIG_IGN );


    maxi = 0;
    fdssock.fd = listenfd;
    fdssock.events = POLLRDNORM;
    nfds = FD_SETSIZE;
    int i=0;
    for(i=1;i<FD_SETSIZE;i++){
        fds[i].fd = -1;
    }

    fds[0] = fdssock;
    
    for(;;){
        nready = csocket::poll(fds,maxi+1,-1);
        if(nready == -1){
            perror("poll error");
        }else if(nready == 0){
            printf("poll timeout \n");
            continue;
        }

        if(fds[0].revents & POLLRDNORM){
           
            clilen = sizeof(cliaddr);
            acceptfd = csocket::accept(listenfd,(struct sockaddr *)&cliaddr, &clilen);
            if(acceptfd<0){
                perror("accpet error");
                exit(1);
            }

            printf("new client: %s:%d,total:%d/%d\n",
			inet_ntoa(cliaddr.sin_addr),
			ntohs(cliaddr.sin_port),
            nready,maxi);

            for(i=1;i<FD_SETSIZE;i++){
                if(fds[i].fd < 0){
                    fds[i].fd = acceptfd;
                    fds[i].events = POLLRDNORM;
                    break;
                }   
            }

            if (i > maxi)
				maxi = i;	

            if(--nready <= 0)
                continue;
        }
       // printf("socket event\n");
        for(i=1;i<maxi;i++){
            //printf("socket event\n");
            if((infd = fds[i].fd) < 0
            ||fds[i].revents == 0)
                continue;

            //if (fds[i].revents & (POLLRDNORM | POLLERR)){
                if((n = read(infd,buf,MAXLINE))<=0){
                    close(infd);
                    fds[i].fd = -1;
                }else{
                    writen(infd,buf,n);
                }

                if(--nready == 0)
                    break;
           // }
        }
    }
}
#endif

#include <sys/epoll.h>

#define MAX_EVENT 20

int Socket::epoll()
{
    int epfd , err , nready , acceptfd;
    struct epoll_event ev , event[MAX_EVENT];

    struct sockaddr_in cliaddr;
    socklen_t clilen;

    epfd = epoll_create(MAX_EVENT);
    if(epfd == -1){
        perror("epoll create error");
        exit(-1);
    }

    listenfd = this->listenfd;

    ev.events = EPOLLIN|EPOLLOUT;
    ev.data.fd = listenfd;
    err = epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
    if(err == -1){
        printf("fd=%d,epfd=%d\n",listenfd,epfd);
        perror("epoll ctl error");
        exit(-1);
    }

    for(;;){
        nready = epoll_wait(epfd,event,MAX_EVENT,-1);
        if(nready == -1){
            perror("epoll wait error");
            exit(-1);
        }

        for(int n=0;n<nready;n++){
            if(event[n].data.fd == listenfd){
                            clilen = sizeof(cliaddr);
                acceptfd = csocket::accept(listenfd,(struct sockaddr *)&cliaddr, &clilen);
                if(acceptfd<0){
                    perror("accpet error");
                    exit(1);
                }

                printf("new client: %s:%d,total\n",
                inet_ntoa(cliaddr.sin_addr),
                ntohs(cliaddr.sin_port));

                ev.events = EPOLLIN|EPOLLET;
                ev.data.fd = acceptfd;
                if(epoll_ctl(epfd,EPOLL_CTL_ADD,acceptfd,&ev)<0){
                    perror("epoll ctl error");
                    exit(-1);
                }  
            }else{
                task(event[n].data.fd);
            }
        }
    }

}



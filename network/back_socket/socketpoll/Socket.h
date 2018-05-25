/**
 * 
 * 
 * */

#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include <arpa/inet.h>
#include	<netdb.h>
#include	<time.h>
#include	<strings.h>
#include <string.h>

#include "ThreadPoll.h"

//extern "C" 

class Socket{
public:
    explicit Socket(int portnum,int backlog, int domain=PF_INET , int type=SOCK_STREAM);
    ~Socket();

    explicit Socket(const char *host,int portnum,int domain=AF_INET , int type=SOCK_STREAM);

    enum {
        SERVER,
        CLIENT
    };
    
    #define HOSTEN 206
    #define	MAXLINE	20
    #define NOTDEF 1

    int getServFd();
    int getClientFd();

    int readcbuf(char * str);
    int WriteLog(const char *format, ...);

    virtual int accept();
    virtual int select();
    virtual int poll();
    virtual int epoll();

    void read_til_crnl(FILE *fp){
        char	buf[BUFSIZ];
        while( fgets(buf,BUFSIZ,fp) != NULL && strcmp(buf,"\r\n") != 0 )
            ;
    }

static ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n);

private:
    int listenfd ;
    int connfd;

    FILE *fpin;
    FILE *logfile;
    int logfd;

    char request[BUFSIZ];
    char recvbuf[BUFSIZ];
    
    ThreadPoll *pll;

    struct sockaddr_in saddr;
    struct sockaddr_in caddr;

};
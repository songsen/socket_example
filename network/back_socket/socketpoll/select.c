#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void showdata(char *fname, int fd)
{
	char buf[BUFSIZ];
	int  n;

	printf("%s:%d\n", fname, n);
	fflush(stdout);
	n = read(fd, buf, BUFSIZ);
	if ( n == -1 )
		perror(fname);
	write(1, buf, n);
	write(1, "\n", 1);
}

int main(int argc, char **argv)
{
    fd_set fdset , allfd;
    int fd1,fd2,nfds, nready;

    char buf1[16];
    char buf2[16];

    strncpy(buf1,"tty",16);
    strncpy(buf2,"stdin",16);

    fd1 = open("/dev/tty",O_RDONLY);
    fd2 = open("/dev/input/mice",O_RDONLY);

    FD_ZERO(&fdset);
    FD_SET(fd1,&fdset);
    FD_SET(fd2,&fdset);

    nfds = fd1>fd2?fd1:fd2;
    
    for(;;){
        allfd = fdset;
        nready = select(nfds+1,&allfd,NULL,NULL,NULL);
        if(nready == -1){
            perror("select error");
        }

        if(FD_ISSET(fd1,&fdset)){
            showdata(buf1,fd1);
        }else if(FD_ISSET(fd2,&fdset)){
            showdata(buf2,fd2);
        } 
    }
}
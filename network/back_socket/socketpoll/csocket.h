#ifndef __C_SOCKET_H
#define __C_SOCKET_H

    #include <sys/types.h>          /* See NOTES */
    #include <sys/socket.h>
      #include <sys/time.h>
       #include <sys/types.h>
       #include <unistd.h>
       #include <poll.h>

       
namespace csocket{
 extern "C"{
     int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
     int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout);
     int poll(struct pollfd *fds, nfds_t nfds, int timeout);
 }
}

#endif
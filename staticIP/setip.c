/* set_interface_ip_address_ioctl.c */

#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void set_ipaddr(const char *, const char *);

int main(int argc, char *argv[])
{

    if (argc != 3) {
        fprintf(stderr, "Usage: %s [network interface name] [ip address]\n",
        argv[0]);
        exit(EXIT_FAILURE);
    }

    char ifname[IFNAMSIZ] = {'\0'};
    strncpy(ifname, argv[1], IFNAMSIZ-1);
    char ipaddr[INET_ADDRSTRLEN] = {'\0'};
    strncpy(ipaddr, argv[2], INET_ADDRSTRLEN);

    set_ipaddr(ifname, ipaddr);

    printf("Interface %s : ip address is set to %s\n", ifname, ipaddr);
    
    return 0;
}

static void set_ipaddr(const char *dev, const char *ip)
{
    int sfd, saved_errno, ret;
    struct ifreq ifr;
    struct sockaddr_in sin;

    sfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(sin.sin_addr));

    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

    errno = saved_errno;
    ret = ioctl(sfd, SIOCSIFADDR, &ifr);
    if (ret == -1) {
        if (errno == 19) {
            fprintf(stderr, "Interface %s : No such device.\n", dev);
            exit(EXIT_FAILURE);
        }
        if (errno == 99) {
            fprintf(stderr, "Interface %s : No IPv4 address assigned.\n", dev);
            exit(EXIT_FAILURE);
        }
    }
    saved_errno = errno;

    close(sfd);
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>

#define DEV_NAME "/dev/dht11_misc"

int main(int argc, char *argv[])
{
    int i;
    int fd = 0;
    unsigned char buff[6];
    

    fd = open (DEV_NAME, O_RDONLY);
    if (fd < 0) {
        perror("Open "DEV_NAME" Failed!\n");
        exit(1);
    }

    printf("read orig data from device\n");
    
    i = read(fd, &buff, 6);
    if (!i) {
        perror("read "DEV_NAME" Failed!\n");
        close(fd);
        exit(1);
    }

    printf("mosi:%d.%d  tem:%d.%d  sum:%d\n",buff[0],buff[1],buff[2],buff[3],buff[4],buff[5]);

    printf("\n");
    close(fd);
    return 0;
}



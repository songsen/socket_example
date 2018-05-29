#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>  
#include "shmread.h"
/*
int shm_open(const char *name, int oflag, mode_t mode);
//创建或打开一个共享内存,成功返回一个整数的文件描述符，错误返回-1。
1.name:共享内存区的名字；
2.标志位；open的标志一样
3.权限位
int shm_unlink(const char *name);

编译时要加库文件-lrt
*/

//#define SHMNAME "mqtt_shm"
#define OPEN_FLAG O_RDWR|O_CREAT
#define OPEN_MODE 00777
#define FILE_SIZE 512

#define SHMNAME path

char* readshmm(char *path)
{
    int ret = -1;
    int fd = -1;

    
    //char buf[512] = {0};

    char* buf = calloc(sizeof(char),256);

    void* add_r = NULL;

    //创建或者打开一个共享内存
    fd = shm_open(SHMNAME, OPEN_FLAG, OPEN_MODE);
    if(-1 == (ret = fd))
    {
        perror("shm  failed: ");
       return NULL;
    }
    
    //调整确定文件共享内存的空间
    ret = ftruncate(fd, FILE_SIZE);
    if(-1 == ret)
    {
        perror("ftruncate faile: ");
       return NULL;
    }
    
    //映射目标文件的存储区
    add_r = mmap(NULL, FILE_SIZE, PROT_READ, MAP_SHARED, fd, 0);//SEEK_SET
    if(NULL == add_r)
    {
        perror("mmap add_r failed: ");
       return NULL;
    }    // shm_unlink(SHMNAME);
    // if(-1 == ret)
    // {
    //     perror("shm_unlink faile: ");
    //    return NULL;
    // }

    //memcpy 内存共享 写入内容
    memcpy(buf, add_r, 256);
    
   // printf("buf = %s\n", buf);

    //取消映射
    ret = munmap(add_r, FILE_SIZE);
    if(-1 == ret)
    {
        perror("munmap add_r faile: ");
       return NULL;
    }
    //删除内存共享
    // shm_unlink(SHMNAME);
    // if(-1 == ret)
    // {
    //     perror("shm_unlink faile: ");
    //    return NULL;
    // }


_OUT:    
    return buf;
}





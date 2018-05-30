#ifndef _SHM_PUBLIC_H__
#define _SHM_PUBLIC_H__

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define MM_NAME "/tmp/mqtt_shm"
#define MM_SIZE 512


#endif

#include <stdlib.h>   
#include <string.h>  
#include <sys/shm.h>  
#include <sys/types.h>  
  
int my_shm_open(char* filename, int open_flag){  
    int shm_id;  
    key_t key;  
    key = ftok(filename, 0x03);  
    if(key == -1){  
        return -1;  
    }  
    if(open_flag)  
        shm_id = shmget(key, 4096, IPC_CREAT|IPC_EXCL|0600);  
    else  
        shm_id = shmget(key, 0, 0);  
    if(shm_id == -1){  
        return -1;  
    }  
    return shm_id;  
}  
  
int my_shm_update(int shm_id, char* content){  
    char* addr;  
    addr = (char*)shmat(shm_id, NULL, 0);  
    if(addr == (char*)-1){  
        return -1;  
    }  
    if(strlen(content) > 4095)  
        return -1;  
    strcpy(addr, content);  
    shmdt(addr);  
    return 0;  
}  
  
int my_shm_close(int shm_id){  
    shmctl(shm_id, IPC_RMID, NULL);  
    return 0;  
}  
  
char* my_shm_read(char* filename){  
    int shm_id;  
    char* addr;  
    char* s;  
    shm_id = my_shm_open(filename, 0);  
    if(shm_id == -1)  
        return NULL;  
    addr = (char*)shmat(shm_id, NULL, 0);  
    if(addr == (char*)-1){  
        return NULL;  
    }  
    s = (char*)malloc(strlen(addr) + 1);  
    strcpy(s, addr);  
    shmdt(addr);  
    return s;  
}  
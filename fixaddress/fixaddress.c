#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#define ADDR_CONFIG "/etc/fix_net_ip.config"

char *getIP(char *hostip,size_t len,const char* interfacename ,const char *inetaddr);
char *setIP(char *cmd,size_t n,const char *interface,const char *ip);
char *parseConfig(char *interface,size_t n,const char *pathchar ,const char *para_name);
int WriteLog(const char * format, ...);

int main(int argc, char **argv)
{
    char interface[64];
    char hostip[64];
    char netmask[64];
    int newpid, status, retpid;
    char buf[256];

    sched_yield();
    parseConfig(interface,64,ADDR_CONFIG,"interface");
    WriteLog("interface:%s\n",interface);

    char *pchar = NULL;
    FILE* fdnet = NULL;
    while( pchar == NULL){
        fdnet = fopen("/proc/net/if_inet6","r");
        if(fdnet == NULL){
            perror("fopen");
            exit(-1);
        }

        while(NULL != fgets(buf,256,fdnet)){
            pchar = strstr(buf,interface);
            if(NULL != pchar)
                break;
        }

        fclose(fdnet);
        sched_yield();
       sleep(30);
    }

    getIP(hostip,64,interface,"inet addr");
    WriteLog("%s\n",hostip);

    char *p = strrchr(hostip,'.');
    *(++p) = '2';
    *(++p) = '0';
    *(++p) = '0';
    WriteLog("%s\n",hostip);

    getIP(netmask,64,interface,"Mask");
    WriteLog("%s\n",netmask);

    // char *cmd = (char *)calloc(sizeof(char),126);
    // WriteLog("%s\n",setIP(cmd,126,interface,hostip));
    //free(cmd);

    newpid = fork();
    if(newpid == 0){
        execlp("dhclient","dhclient","-r",NULL);
    }
    retpid = wait(&status);
    WriteLog("ret's pid = %d; status = %d\n",retpid,status);

    newpid = fork();
    if(newpid == 0){
        execlp("ifconfig","ifconfig",interface,hostip,"netmask",netmask,NULL);
    }
    retpid = wait(&status);
    WriteLog("ret's pid = %d; status = %d\n",retpid,status);

    newpid = fork();
    if(newpid == 0){
        execlp("dhclient","dhclient",NULL);
    }
    retpid = wait(&status);
    WriteLog("ret's pid = %d; status = %d\n",retpid,status);

    return 0;
}

int WriteLog(const char * format, ...)
{
    static int isopen = 0;
    static FILE *logfile ;

    va_list arg_ptr;

    if(0 == isopen){
        logfile = fopen("/tmp/fix_net_ip","w");
        if(NULL == logfile){
            perror("fopen");
            exit(-1);
        }
        isopen = 1;
    }
   
    va_start(arg_ptr, format);
    int nWrittenBytes = vfprintf(logfile, format, arg_ptr);
    va_end(arg_ptr);
    return nWrittenBytes;
}

char *parseConfig(char *interface,size_t n,const char *pathchar ,const char *para_name)
{
    FILE *fd;
    char *p;
    char buf[256];
    bzero(buf,256);
    bzero(interface,n);

    fd = fopen(pathchar,"r");
    while(fgets(buf,256,fd) != NULL){
        p = strstr(buf,para_name);
        if(NULL != p){
            p += strlen(para_name);
            while(!isalnum(*p))
                p++;
            strncpy(interface,p,strcspn(p," "));
            fclose(fd);
            return interface;  
        }
    }
    fclose(fd);
    return NULL;
}

char *setIP(char *cmd,size_t n,const char *interface,const char *ip)
{
    strncpy(cmd,"ifconfig ",n);
    n -= sizeof("ifconfig ");
    strncat(cmd,interface,n);
    n -= strlen(interface);
    strncat(cmd," ",n);
    n--;
    strncat(cmd,ip,n);
    WriteLog("%s\n",cmd);

    return cmd;
}

char *getIP(char *hostip,size_t len,const char* interfacename ,const char *inetaddr)
{
    FILE *fp;
    char *p;
    char buf[256];

    bzero(hostip,len);
    fp = popen("ifconfig","r");

    while(fgets(buf,256,fp) != NULL){
        p = strstr(buf,interfacename);
        if(NULL != p){
          break;  
        }
    }

    if(NULL == p){
        pclose(fp);
        return NULL;
    }

    while(fgets(buf,256,fp) != NULL){
        p = strstr(buf,inetaddr);
        if(NULL != p){
            p += strlen(inetaddr);
            p++;
            //WriteLog("%s",p); 
            int  i =0;
            while(*p!=32){
                hostip[i] = *p;
                i++;
                p++;  
            }
            break;
        } 
    }
    pclose(fp);
    if(NULL == p)
        return NULL;

    if(0 == strcmp(hostip,"0.0.0.0")){
        return NULL;
    }

    //WriteLog("ip=%s,size=%d\n",hostip,(int)strlen(hostip));
    return hostip;
}
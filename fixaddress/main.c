#include <stdio.h>
#include <string.h>
#include<ctype.h>
#include <strings.h>
#include	<stdlib.h>


#define ADDR_CONFIG "setaddress.config"

char *getIP(char *hostip,size_t len,const char* interfacename ,const char *inetaddr);
char *setIP(char *cmd,size_t n,const char *interface,const char *ip);
char *parseConfig(char *interface,size_t n,const char *pathchar ,const char *para_name);

int main(int argc, char **argv)
{
    char interface[32];
    char hostip[32];

    parseConfig(interface,32,ADDR_CONFIG,"interface");
    printf("interface:%s\n",interface);

    getIP(hostip,32,interface,"inet addr");
    printf("%s\n",hostip);

    char *cmd = (char *)calloc(sizeof(char),126);
    printf("%s\n",setIP(cmd,sizeof(cmd),interface,hostip));
    free(cmd);

    return 0;
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
    strncpy(cmd,"ifconfig ",126);
    n -= sizeof("ifconfig ");
    strncat(cmd,interface,n);
    n -= strlen(interface);
    strncat(cmd," ",n);
    n--;
    strncat(cmd,ip,n);
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
            //printf("%s",p); 
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

    p = strrchr(hostip,'.');
    *(++p) = '2';
    *(++p) = '0';
    *(++p) = '0';
    //printf("ip=%s,size=%d\n",hostip,(int)strlen(hostip));
    return hostip;
}

#include "Socket.h"
#include <iostream>
#include  <pthread.h>

int cnt=0;


pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void *clientn(void *str)
{   
    char sendbuf[64];
    pthread_mutex_lock(&counter_lock);
    snprintf(sendbuf,MAXLINE,"send count %d",cnt++);
    pthread_mutex_unlock(&counter_lock);
    try{
        Socket *sock = new Socket("127.0.0.1",8080);
        int i = sock->readcbuf((char *)sendbuf);
    }catch(int &i){
        std::cout<<i<<std::endl;
    }
}

void *test(void * null)
{
        //snprintf(sendbuf,MAXLINE,"send count %d",cnt++);
        pthread_t t1, t2,t3,t4,t5,t6,t7;		/* two threads */
        pthread_create(&t1, NULL, clientn, (void *)"1\n");
        pthread_create(&t2, NULL, clientn, (void *)"2\n");
        pthread_create(&t3, NULL, clientn, (void *)"3\n");
        pthread_create(&t4, NULL, clientn, (void *)"4\n");
        pthread_create(&t5, NULL, clientn, (void *)"5\n");
        pthread_create(&t6, NULL, clientn, (void *)"6\n");
        pthread_create(&t7, NULL, clientn, (void *)"7\n");

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        pthread_join(t3, NULL);
        pthread_join(t4, NULL);
        pthread_join(t5, NULL);
        pthread_join(t6, NULL);
        pthread_join(t7, NULL);
}

void more()
{
    int n=800;
    pthread_t t[n];
    while(n--){
        pthread_create(&t[n], NULL, test, NULL);
    }
    n=800;
    while(n--){
        pthread_join(t[n],NULL);
    }
}

int main(int argc, char **argv)
{
     pthread_t t1;	

    //     pthread_create(&t1, NULL, test, NULL);
    //     pthread_create(&t2, NULL, test, NULL);
    //     pthread_create(&t3, NULL, test, NULL);
    //     pthread_create(&t4, NULL, test, NULL);
    //     pthread_create(&t5, NULL, test, NULL);
    //     pthread_create(&t6, NULL, test, NULL);
    //     pthread_create(&t7, NULL, test, NULL);

    //     pthread_join(t1, NULL);
    //     pthread_join(t2, NULL);
    //     pthread_join(t3, NULL);
    //     pthread_join(t4, NULL);
    //     pthread_join(t5, NULL);
    //     pthread_join(t6, NULL);
    //     pthread_join(t7, NULL);
    //pthread_create(&t1, NULL, clientn, (void *)"1\n");
    //pthread_join(t1, NULL);
     //more();
    //char sendbuf[64] = ; 
    try{ //123.207.20.45
        Socket *sock = new Socket("1.2.3.1",2202);
        int i = sock->readcbuf("SSH-2.0-OpenSSH_5.3");
    }catch(int &i){
        std::cout<<i<<std::endl;
    }
    return 0;
}


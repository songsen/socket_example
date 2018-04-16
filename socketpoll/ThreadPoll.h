#include <pthread.h>
#include <stdlib.h>
#include <error.h>
#include <stdio.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

class ThreadPoll {
    public:
    typedef void (*fn)(int a); 

    typedef struct {
        int  para;
        fn function ;
    } task ;

    ThreadPoll(int nthread);
    ~ThreadPoll();

    static int push_back(fn f,int a);
    static task pop_front();
    static int sem();
    int getTaskSize();

    // empty
    // size
    // front
    // back
    // push_back
    // pop_front

    private:
    static void *waitpoll(void *arg);

    static task *taskQueue;
    static pthread_mutex_t lock ;
    static pthread_cond_t  flag ;
    static pthread_mutex_t lock_queue;
    static int size_queue;
    static int capa_queue;
    static int pop_queue;
    static int push_queue;
    static sem_t *semsnet;

    pthread_t *t;
    const int nthread ;
};



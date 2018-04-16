#include "ThreadPoll.h"
#include <semaphore.h>

ThreadPoll::ThreadPoll(int nthread):nthread(nthread)
{
    t = (pthread_t *)calloc(sizeof(pthread_t),nthread);
    sem();
  //  int *i = (int *)calloc(sizeof(int),nthread);
    pthread_mutex_lock(&lock);
    while(nthread--){
        pthread_create(&t[nthread],NULL,waitpoll,&nthread);
    }
}

ThreadPoll::~ThreadPoll()
{
    int i = nthread;
    sem_close(semsnet);
    while(i--){
        free(&t[i]);
    }
}

pthread_mutex_t ThreadPoll::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPoll::flag = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ThreadPoll::lock_queue = PTHREAD_MUTEX_INITIALIZER;

int ThreadPoll::capa_queue = 5;
ThreadPoll::task* ThreadPoll::taskQueue \
= (ThreadPoll::task* )calloc(sizeof(ThreadPoll::task),capa_queue);

int ThreadPoll::size_queue = 0;
int ThreadPoll::pop_queue = 0;
int ThreadPoll::push_queue = 0;



#define semname "sem net server semaphore"

sem_t *ThreadPoll::semsnet = NULL;

#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
					/* default permissions for new files */

int ThreadPoll::sem()
{
    semsnet = sem_open(semname,O_CREAT,FILE_MODE,0);
    if(semsnet == nullptr){
        perror("sem_open error");
        exit(EXIT_FAILURE);
    }
    return 1;
}

int ThreadPoll::getTaskSize()
{
    return size_queue;
}

void* ThreadPoll::waitpoll(void *arg)
{
    task t;
    int i = (*(int *)arg);
    // if(taskQueue == nullptr){
    //     taskQueue = (ThreadPoll::task* )calloc(sizeof(ThreadPoll::task),50);
    // }

    while(1){
        t = pop_front();//block
        printf("pthread myID=%d push_queue=%d,pop_queue=%d\n",
        i,push_queue,pop_queue);
        if(t.function == NULL)
            continue;
        t.function(t.para);
    }
}

ThreadPoll::task ThreadPoll::pop_front()
{
    task ret;
    
    //pthread_cond_wait(&flag, &lock); /* wait for notify */
    sem_wait(semsnet);
    pthread_mutex_lock(&lock_queue);
    ret.function = NULL;
    if(taskQueue[pop_queue].function != NULL){
        ret = taskQueue[pop_queue];
        taskQueue[pop_queue].function = NULL;

        if(++pop_queue == capa_queue){
            pop_queue = 0;
        }
        size_queue--;
    }
    pthread_mutex_unlock(&lock_queue);
    return ret;
}

int ThreadPoll::push_back(fn f,int a)
{
    pthread_mutex_lock(&lock_queue);
    if(taskQueue[push_queue].function == NULL){
        taskQueue[push_queue].para = a;
        taskQueue[push_queue].function = f;
    }else{
        taskQueue = (task*)realloc(taskQueue,capa_queue*2*sizeof(task));
        if(taskQueue == NULL){
            perror("realloc error");
            exit(-1);
        }

        push_queue = capa_queue;
        capa_queue *= 2;
        pop_queue = 0;
        
        for(int i=push_queue;i<capa_queue;i++){
            taskQueue[i].function = NULL;
        }

        taskQueue[push_queue].para = a;
        taskQueue[push_queue].function = f;
    }

    if(++push_queue == capa_queue){ // next null
        push_queue = 0;
    }
    size_queue++;
    pthread_mutex_unlock(&lock_queue);
    //pthread_cond_signal(&flag);	/* announce state change */
    sem_post(semsnet);

    return 1;
}

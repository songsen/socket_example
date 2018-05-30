#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
 #include <unistd.h>

int tostop = 0;

void fin(int signo)
{
    tostop = 1;
    printf("I will exit\n");
}

int main(int argc ,char** argv)
{
    signal(SIGINT,fin);
    while(!tostop){
        printf("I will sleep\n");
        sleep(2);
        printf("I'm weakup\n");
    }
    return 1;
}
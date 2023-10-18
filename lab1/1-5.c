#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>

int main() {
    pid_t pid,pid1;
    pid = fork();
    if(pid < 0){
        fprintf(stderr,"Fork Faild");
        return 1;
    }
    else if(pid == 0){ 
        pid = getpid();
        printf("child 1: pid = %d\n", pid);
    }
    else {
        pid = fork();
        if (pid < 0){
            fprintf(stderr, "Fork Faild");
        }
        else if(pid ==0){
            pid = getpid();
            system("./system_call");
            printf("child 2: pid = %d\n",pid);
        }
        else {
            pid = getpid();
            printf("parent: pid = %d\n", pid);
            wait(NULL);
            wait(NULL);
        }
    }
    return 0;
}

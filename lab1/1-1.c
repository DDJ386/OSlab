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
        pid1 = getpid();
        printf("child: pid = %d", pid);
        printf("child: pid1 = %d",pid1);
        sleep(2);
        printf("after father terminated ppid: %d ",getppid());
    }
    else {
        pid1 = getpid();
        printf("parent: pid = %d", pid);
        printf("parent: pid1 = %d",pid1);
    }
    return 0;
}

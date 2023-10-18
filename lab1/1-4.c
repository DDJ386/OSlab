#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>

int main() {
    pid_t pid,pid1;
    pid = fork();
    int global_value = 0;
    if(pid < 0){
        fprintf(stderr,"Fork Faild");
        return 1;
    }
    else if(pid == 0){ 
        global_value++;
        printf("child: value = %d",global_value);
        printf("child: &value = %p",&global_value);
    }
    else {
        printf("parent: value= %d", global_value);
        printf("parent: &value= %p", &global_value);
        wait(NULL);
    }
    global_value *= 2;
    printf("\nbefore return: value = %d\n", global_value);
    return 0;
}

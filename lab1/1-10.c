#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

void *thread1() {
    printf("thread1 create success!\n");
    printf("thread1 tid: %u pid: %d\n",pthread_self(),getpid());
    execv("./system_call",NULL);
    printf("thread1 systemcall return\n");
}
void *thread2() {
    printf("thread2 create success!\n");
    printf("thread2 tid: %u pid: %d\n",pthread_self(),getpid());
    execv("./system_call",NULL);
    printf("thread2 systemcall return\n");
}
int main() {
    pthread_t tid1,tid2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid1,&attr,thread1,NULL);
    pthread_create(&tid2,&attr,thread2,NULL);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    return 0;
}

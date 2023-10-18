#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

int global_value = 0;
sem_t add,sub;

void *func1() {
    sem_wait(&add);
    for(int i = 0; i < 100000; i++){
        global_value++;
    }
    sem_post(&sub);
}
void *func2() {
    sem_wait(&sub);
    for(int i = 0; i < 100000; i++){
        global_value--;
    }
    sem_post(&add);
}

int main() {
    sem_init(&add,0,1);
    sem_init(&sub,0,0);
    pthread_t tid1;
    pthread_t tid2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid1, &attr, func1, NULL);
    pthread_create(&tid2, &attr, func2, NULL);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    printf("sum = %d\n",global_value);
    return 0;
}

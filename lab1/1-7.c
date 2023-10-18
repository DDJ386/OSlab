#include <pthread.h>
#include <stdio.h>

int global_value = 0;

void *func1() {
    for(int i = 0; i < 100000; i++){
        global_value++;
    }
}
void *func2() {
    for(int i = 0; i < 100000; i++){
        global_value--;
    }
}

int main() {
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

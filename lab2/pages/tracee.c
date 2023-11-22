#include <stdio.h>
#include <stdlib.h>

int main () {
    int *datas[100];
    for(int i = 0; i < 100; i++) {
        datas[i] = (int*)malloc(sizeof(int) * 100);           
    }
    for(int i = 0; i < 100; i++) {
        free(datas[i]);
    }
}

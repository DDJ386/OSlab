#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int pid = getpid();
    printf("system_call PID:  %d\n", pid);
}

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    // create a "disk" file with 64 MB size
    int fd = open("./disk",O_CREAT|O_RDWR);
    for(int i = 0; i < 64*1024*1024; i++) {
        write(fd,"\0",1);
    }
    return 0;
}

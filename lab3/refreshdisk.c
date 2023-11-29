#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  char buf[64 * 1024] = {0};
  int fd = open("./disk", O_CREAT | O_RDWR, 0777);
  for (int i = 0; i < 1024; i++) {
    write(fd, buf, 64 * 1024);
  }
  return 0;
}

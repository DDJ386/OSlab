#include "pages.h"

void rand_input(int process_max) {
  while (1) {
    int process = (int)(random() % process_max);
    uint64_t address = (uint64_t)random();
    if (access(process, address)) {
      printf("process%d access %lu success\n", process, address);
    } else {
      printf("process%d access %lu faild\n", process, address);
    }
  }
}

void man_input(int process_max) {
  printf("please input process and adress: \n");
  printf("e.g 1 0xfffa01\n");
  while (1) {
    int process;
    uint64_t address;
    scanf("%d %lu", &process, &address);
    if (access(process, address)) {
      printf("process%d access %lu success\n", process, address);
    } else {
      printf("process%d access %lu faild\n", process, address);
    }
  }
}

int choise_input_mode(){
  printf("input mode:\n");
  printf("1. manual\n");
  printf("2. auto\n");
  int mode;
  scanf("%d",&mode);
  return mode;
}

int set_max_processes(){
  int process_max;
  scanf("%d",&process_max);
  return process_max;
}
int main() {
  int mode = choise_input_mode();
  int process_max = set_max_processes();
  switch(mode){
    case 1:
      man_input(process_max);
      break;
    case 2:
      rand_input(process_max);
  }
}
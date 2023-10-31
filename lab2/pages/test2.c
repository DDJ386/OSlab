#include <time.h>
#include <assert.h>

#include "pages.h"
void rand_input(int process_max) {
  /* 在test1的基础上提升测试数据的局部性 */
  /* 每个进程的地址空间都被划分为栈段, 堆段, 数据段, 代码段,
  因此一个进程访问的地址应集中分布在这几个段地址的附近 */
  uint64_t addrs[process_max][4];
  for (int i = 0; i < process_max; i++) {
    /* 假设地址空间一共有64k, 下面为各个段的基地址 */
    addrs[i][0] = 0x0000;  /* 代码段 */
    addrs[i][1] = 0x2500;  /* 数据段 */
    addrs[i][2] = 0x5000;  /* 堆段 */
    addrs[i][3] = 0x10000; /* 栈段 */
  }
  srand((unsigned)time(NULL));
  for (int i = 0; i < 1000; i++) {
    int process = (int)(rand() % process_max);
    /* 每个进程在其被调度期间可以对内存进行数次读写操作 */
    for (int j = 0; j < 30; j++) {
      uint64_t address;
      int seg = rand() % 4;
      switch (seg) {
        case 0:
          /* 假设每次访问代码段读入五条指令 */
          address = addrs[process][seg];
          addrs[process][seg] += 5; 
          assert(addrs[process][seg] < 0x2500);
          break;
        case 1:
          /* 假设访问数据段的行为为读取数据段内随机一偏移地址处的数据 */
          address = addrs[process][seg] + (uint64_t)(rand()%0x2500);
          break;
        case 2:
          /* 假设访问堆段的行为为读取堆段内随机一偏移地址处的数据 */
          address = addrs[process][seg] + (uint64_t)(rand()%0x2500);
        case 3:
          address = addrs[process][seg];
          addrs[process][seg] -= (uint64_t)rand()%10;
      }

      if (access(process, address)) {
        printf("process%d access %lu success\n", process, address);
      } else {
        printf("process%d access %lu faild\n", process, address);
      }
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
    if (process == -1) return;
    if (access(process, address)) {
      printf("process%d access %lu success\n", process, address);
    } else {
      printf("process%d access %lu faild\n", process, address);
    }
  }
}

int choise_input_mode() {
  printf("input mode:\n");
  printf("1. manual\n");
  printf("2. auto\n");
  int mode;
  scanf("%d", &mode);
  return mode;
}

int set_max_processes() {
  printf("input the maximum number of processes:\n");
  int process_max;
  scanf("%d", &process_max);
  return process_max;
}

void set_algo() {
  int algo;
  printf("input the algorithm:\n");
  printf("1. FIFO\n2. LRU\n");
  scanf("%d", &algo);
  set_algorithm(algo);
}

int main() {
  int mode = choise_input_mode();
  int process_max = set_max_processes();
  set_algo();
  switch (mode) {
    case 1:
      man_input(process_max);
      break;
    case 2:
      rand_input(process_max);
  }
  summarize();
}
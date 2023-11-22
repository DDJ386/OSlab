#include <assert.h>
#include <time.h>

#include "pages.h"
void rand_input(int process_max) {
  /* 在test1的基础上提升测试数据的局部性 */
  /* 每个进程的地址空间都被划分为栈段, 堆段, 数据段, 代码段,
  因此一个进程访问的地址应集中分布在这几个段地址的附近 */
  uint64_t addrs[process_max][4];
  for (int i = 0; i < process_max; i++) {
    /* 假设地址空间一共有64k, 下面为各个段的基地址 */
    addrs[i][0] = 0x000000; /* 代码段 */
    addrs[i][1] = 0x100000; /* 数据段 */
    addrs[i][2] = 0x200000; /* 堆段 */
    addrs[i][3] = 0x400000; /* 栈段 */
  }
  srand((unsigned)time(NULL));
  for (int i = 0; i < 100000; i++) {
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
          assert(addrs[process][seg] < addrs[process][1]);
          break;
        case 1:
          /* 假设访问数据段的行为为读取数据段内随机一偏移地址处的数据 */
          address = addrs[process][seg] + (uint64_t)(rand() % 0x2500);
          break;
        case 2:
          /* 假设访问堆段的行为为读取堆段内随机一偏移地址处的数据 */
          address = addrs[process][seg] + (uint64_t)(rand() % 0x2500);
        case 3:
          address = addrs[process][seg];
          addrs[process][seg] -= (uint64_t)rand() % 10;
      }

      if (access(process, address)) {
        // printf("process%d access %lu success\n", process, address);
      } else {
        // printf("process%d access %lu faild\n", process, address);
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
    assert(process < process_max);
    if (process == -1) return;
    if (access(process, address)) {
      printf("process%d access %lu success\n", process, address);
    } else {
      printf("process%d access %lu faild\n", process, address);
    }
  }
}

void read_from_real_process(int process_max) {
  FILE **fd = (FILE **)malloc(sizeof(FILE *) * process_max);
  for (int i = 0; i < process_max; i++) {
    fd[i] = fopen("./process", "r");
  }
  int terminated = 0;
  int *end = (int *)calloc(process_max, sizeof(int));
  srand((unsigned)time(NULL));
  int process = 0;
  uint64_t address;
  while (1) {
    /* 所有进程全部结束后退出循环, 计算结果 */
    if (terminated == process_max) {
      break;
    }

    /* 随机选取一个进程, 并执行100次访存 */
    // process = (process + 1) % process_max;
    process = rand() % process_max;
    for (int i = 0; i < 100; i++) {
      if (fscanf(fd[process], "%lx", &address) == EOF) {
        if (end[process] == 0) {
          terminated++;
          end[process] = 1;
        }
        break;
      }
      if (access(process, address)) {
        // printf("process%d access %lu success\n", process, address);
      } else {
        // printf("process%d access %lu faild\n", process, address);
      }
    }
  }
  /* 释放资源 */
  for (int i = 0; i < process_max; i++) {
    fclose(fd[i]);
  }
  free(fd);
  free(end);
}

int choise_input_mode() {
  printf(
      "input mode:\n"
      "1. manual\n"
      "2. auto\n"
      "3. read from real process\n");
  int mode;
  scanf("%d", &mode);
  assert(mode >= 1 && mode <= 3);
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
  printf(
      "input the algorithm:\n"
      "1. FIFO\n"
      "2. LRU\n");
  scanf("%d", &algo);
  assert(algo >= 1 && algo <= 2);
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
      break;
    case 3:
      read_from_real_process(process_max);
      break;
    default:
      printf("invalid mode.\n");
      break;
  }
  summarize();
}
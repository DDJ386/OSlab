#include "pages.h"

/* 因为只是模拟页面调度算法, 实际并不向页面中存储信息,
 * 因此以能装下的页面总数来描述内存大小 */
#define MEMORY_SIZE 10
/* 页面大小 */
#define PAGE_SIZE 4096
/* 页面置换算法 */
#define FIFO 0
#define LRU  1

int total_cnt = 0;
int access_cnt = 0;
int miss_cnt = 0;
int algorithm = 0;
page_info *memory[MEMORY_SIZE];


inline bool match_page(int process, uint64_t addr, page_info *page) {
  return ((process == page->process && addr > page->start_addr &&
           addr < (page->start_addr + PAGE_SIZE))
              ? 1
              : 0);
}

page_info *create_page(uint64_t addr, int process) {
  page_info *new_page = (page_info *)malloc(sizeof(page_info));
  new_page->start_addr = (addr & (-1 << 12)); /* 页面对齐 */
  new_page->process = process;
  new_page->time_scale = time(NULL);
  return new_page;
}

int replace_FIFO(){
  static int cnt = 0;
  int retval = cnt;
  cnt = (cnt+1)%MEMORY_SIZE;
  return retval;
}

int replace_LRU() {
  time_t min_time = memory[0]->time_scale;
  int retval = 0;
  for(int i = 1; i < MEMORY_SIZE; i++) {
    if(memory[i]->time_scale < min_time) {
      min_time = memory[i]->time_scale;
      retval = i;
    }
  }
  return retval;
}

int replace(int algorithm) {
  switch (algorithm) {
    case FIFO:
      return replace_FIFO();
    case LRU:
      return replace_LRU();
  }
}

int access(int process, uint64_t addr) {
  /* 匹配成功返回1, 否则返回0*/
  total_cnt++;
  for(int i = 0; i < MEMORY_SIZE; i++) {
    if(match_page(process, addr, memory[i])){
      access_cnt++;
      return;
    }
  }
  /* 内存中无匹配页面, 需要创建新页面并进行页面的置换 */
  miss_cnt++;
  int rep_index = replace(algorithm);
  free(memory[rep_index]);
  page_info *new_page = create_page(addr, process);
  memory[rep_index] = new_page;
  return 0;
}

void set_algorithm(int new_algorithm) {
  algorithm = new_algorithm;
}
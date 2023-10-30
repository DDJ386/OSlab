#ifndef _PAGES_H_
#define _PAGES_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* 页面数据结构 */
typedef struct {
  uint64_t start_addr;
  unsigned int time_scale;
  int process;
} page_info;

/* 进程对指定地址进行操作 */
int access(int process, uint64_t addr);
void set_algorithm(int algorithm);
void summarize();
#endif
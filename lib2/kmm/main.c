#include "kmm.h"

int main() {
  char choice;
  init_free_block(DEFAULT_MEM_SIZE);  // 初始化空闲区
  while (1) {
    display_menu();  // 显示菜单
    fflush(stdin);
    choice = getchar();  // 获取用户输入
    while (choice == '\n') {
      choice = getchar();
    }
    switch (choice) {
      case '1':
        set_mem_size();
        break;  // 设置内存大小
      case '2':
        set_algorithm();
        break;  // 设置算法
      case '3':
        new_process();
        break;  // 创建新进程
      case '4':
        kill_process();
        break;  // 删除进程
      case '5':
        display_mem_usage();
        break;  // 显示内存使用
      case '0':
        do_exit();
        exit(0);
        // 释放链表并退出
      default:
        break;
    }
  }
}
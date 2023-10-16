#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#define PROCESS_NAME_LEN 32   /*进程名长度*/
#define MIN_SLICE 10          /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024 /*内存大小*/
#define DEFAULT_MEM_START 0   /*起始位置*/

/* 内存分配算法 */
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

/* 空闲块结构 */
struct free_block_type {
  int size;
  int start_addr;
  struct free_block_type *next;
};

/* 已分配块结构 */
struct allocated_block {
  int pid;
  int size;
  int start_addr;
  char process_name[PROCESS_NAME_LEN];
  struct allocated_block *next;
};

/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
struct free_block_type *init_free_block(int mem_size);
/*显示菜单*/
void display_menu();
/*设置内存的大小*/
int set_mem_size();
/* 设置当前的分配算法 */
void set_algorithm();
/*按指定的算法整理内存空闲块链表*/
void rearrange(int algorithm);
/*按 FF 算法重新整理内存空闲块链表*/
void rearrange_FF();
/*按 BF 算法重新整理内存空闲块链表*/
void rearrange_BF();
/*按 WF 算法重新整理内存空闲块链表*/
void rearrange_WF();
/*创建新的进程，主要是获取内存的申请数量*/
int new_process();
/*分配内存模块*/
int allocate_mem(struct allocated_block *ab);
/* 通过pid寻找进程所在的内存块 */
struct allocated_block *find_process(int pid);
/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
void kill_process();
/*将 ab 所表示的已分配区归还，并进行可能的合并*/
int free_mem(struct allocated_block *ab);
/*释放 ab 数据结构节点*/
int dispose(struct allocated_block *free_ab);
/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
int display_mem_usage();
/* 释放所有内存 */
void do_exit();

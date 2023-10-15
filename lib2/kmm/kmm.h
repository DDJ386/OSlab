#include <stdio.h>

#define PROCESS_NAME_LEN 32   /*进程名长度*/
#define MIN_SLICE 10          /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024 /*内存大小*/
#define DEFAULT_MEM_START 0   /*起始位置*/

/* 内存分配算法 */
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

int mem_size = DEFAULT_MEM_SIZE; /*内存大小*/
int ma_algorithm = MA_FF;        /*当前分配算法*/
static int pid = 0;              /*初始 pid*/
int flag = 0;                    /*设置内存大小标志*/

/* 空闲块结构 */
struct free_block_type {
  int size;
  int start_addr;
  struct free_block_type *next;
};
/* 指向空闲块链表表头的指针 */
struct free_block_type *free_block;

/* 已分配块结构 */
struct allocated_block {
  int pid;
  int size;
  int start_addr;
  char process_name[PROCESS_NAME_LEN];
  struct allocated_block *next;
};
/* 指向已分配块链表表头的指针 */
struct allocated_block *allocated_block_head = NULL;

/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
struct free_block_type *init_free_block(int mem_size);
/*显示菜单*/
void display_menu();
/*设置内存的大小*/
void set_mem_size();
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
/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
void kill_process();
/*将 ab 所表示的已分配区归还，并进行可能的合并*/
int free_mem(struct allocated_block *ab);
/*释放 ab 数据结构节点*/
int dispose(struct allocated_block *free_ab);
/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
int display_mem_usage();

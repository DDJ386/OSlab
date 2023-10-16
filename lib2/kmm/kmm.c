#include "kmm.h"

int mem_size = DEFAULT_MEM_SIZE; /*内存大小*/
int ma_algorithm = MA_FF;        /*当前分配算法*/
static int pid = 0;              /*初始 pid*/
int flag = 0;                    /*设置内存大小标志*/

/* 指向空闲块链表表头的指针 */
struct free_block_type *free_block;

/* 指向已分配块链表表头的指针 */
struct allocated_block *allocated_block_head = NULL;

struct free_block_type *init_free_block(int mem_size) {
  struct free_block_type *fb;
  fb = (struct free_block_type *)malloc(sizeof(struct free_block_type));
  if (fb == NULL) {
    printf("No mem\n");
    return NULL;
  }
  fb->size = mem_size;
  fb->start_addr = DEFAULT_MEM_START;
  fb->next = NULL;
  free_block = fb;
  return fb;
}

void display_menu() {
  printf("\n");
  printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
  printf("2 - Select memory allocation algorithm\n");
  printf("3 - New process \n");
  printf("4 - Terminate a process \n");
  printf("5 - Display memory usage \n");
  printf("0 - Exit\n");
}

int set_mem_size() {
  int size;
  if (flag != 0) {  // 防止重复设置
    printf("Cannot set memory size again\n");
    return 0;
  }
  printf("Total memory size =");
  scanf("%d", &size);
  if (size > 0) {
    mem_size = size;
    free_block->size = mem_size;
  }
  flag = 1;
  return 1;
}

void set_algorithm() {
  int algorithm;
  printf("\t1 - First Fit\n");
  printf("\t2 - Best Fit \n");
  printf("\t3 - Worst Fit \n");
  scanf("%d", &algorithm);
  if (algorithm >= 1 && algorithm <= 3) ma_algorithm = algorithm;
  // 按指定算法重新排列空闲区链表
  rearrange(ma_algorithm);
  flag = 1;
}

static int comp_FF(const void *a, const void *b) {
  if ((*(struct free_block_type **)a)->start_addr <
      (*(struct free_block_type **)b)->start_addr)
    return -1;
  if ((*(struct free_block_type **)a)->start_addr ==
      (*(struct free_block_type **)b)->start_addr)
    return 0;
  if ((*(struct free_block_type **)a)->start_addr >
      (*(struct free_block_type **)b)->start_addr)
    return 1;
}
static int comp_BF(const void *a, const void *b) {
  if ((*(struct free_block_type **)a)->size <
      (*(struct free_block_type **)b)->size)
    return -1;
  if ((*(struct free_block_type **)a)->size ==
      (*(struct free_block_type **)b)->size)
    return 0;
  if ((*(struct free_block_type **)a)->size >
      (*(struct free_block_type **)b)->size)
    return 1;
}
static int comp_WF(const void *a, const void *b) {
  if ((*(struct free_block_type **)a)->size >
      (*(struct free_block_type **)b)->size)
    return -1;
  if ((*(struct free_block_type **)a)->size ==
      (*(struct free_block_type **)b)->size)
    return 0;
  if ((*(struct free_block_type **)a)->size <
      (*(struct free_block_type **)b)->size)
    return 1;
}
static int fill_in_array(struct free_block_type **blocks) {
  int block_count = 0;
  struct free_block_type *index = free_block;
  for (; index != NULL; index = index->next) {
    blocks[block_count++] = index;
  }
  return block_count;
}
static void rearrange_list(struct free_block_type **blocks, int block_count) {
  for (int i = 0; i < block_count - 1; i++) {
    blocks[i]->next = blocks[i + 1];
  }
  blocks[block_count - 1]->next = NULL;
}

void rearrange(int algorithm) {
  struct free_block_type *blocks[DEFAULT_MEM_SIZE / MIN_SLICE];
  int block_count = fill_in_array(blocks);  // 将链表装入数组中
  switch (algorithm) {
    case MA_FF:
      qsort(blocks, block_count, sizeof(struct free_block_type *), comp_FF);
      break;
    case MA_BF:
      qsort(blocks, block_count, sizeof(struct free_block_type *), comp_BF);
      break;
    case MA_WF:
      qsort(blocks, block_count, sizeof(struct free_block_type *), comp_WF);
      break;
  }
  rearrange_list(blocks, block_count);  // 对数组中的结点按顺序进行重排列
  free_block = blocks[0];               // 重新定位头指针
}

int new_process() {
  flag = 1;
  struct allocated_block *ab;
  int size;
  int ret;
  ab = (struct allocated_block *)malloc(sizeof(struct allocated_block));
  if (!ab) exit(-5);
  ab->next = NULL;
  pid++;
  sprintf(ab->process_name, "PROCESS-%02d", pid);
  ab->pid = pid;
  printf("Memory for %s:", ab->process_name);
  scanf("%d", &size);
  if (size > 0) ab->size = size;
  ret = allocate_mem(ab); /* 从空闲区分配内存， ret==1 表示分配 ok*/
  /*如果此时 allocated_block_head 尚未赋值，则赋值*/
  if ((ret == 1) && (allocated_block_head == NULL)) {
    allocated_block_head = ab;
    return 1;
  }
  /*分配成功，将该已分配块的描述插入已分配链表*/
  else if (ret == 1) {
    ab->next = allocated_block_head;
    allocated_block_head = ab;
    return 2;
  } else if (ret == -1) { /*分配不成功*/
    printf("Allocation fail\n");
    free(ab);
    return -1;
  }
  return 3;
}

int allocate_mem(struct allocated_block *ab) {
  struct free_block_type *fbt, *pre;
  int request_size = ab->size;
  fbt = pre = free_block;
  // 根据当前算法在空闲分区链表中搜索合适空闲分区进行分配，分配时注意以下情况：
  //  1. 找到可满足空闲分区且分配后剩余空间足够大，则分割
  //  2. 找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
  //  3. 找不到可满足的空闲分区, 但根据free的实现, 空闲队列时刻保持最优状态,
  //      因此直接返回-1
  //  4. 在成功分配内存后，应保持空闲分区按照相应算法有序
  //  5. 分配成功则返回 1，否则返回-1
  if (fbt->size < request_size) {
    fbt = fbt->next;
    while (fbt != NULL && fbt->size < request_size) {
      pre = fbt;
      fbt = fbt->next;
    }
  }
  if (fbt == NULL) {
    return -1;
  }
  ab->start_addr = fbt->start_addr;
  if ((fbt->size - request_size) > MIN_SLICE) {
    // 分割
    fbt->size -= request_size;
    fbt->start_addr += request_size;
  } else {
    pre->next = fbt->next;
    free(fbt);
  }
  rearrange(ma_algorithm);
  return 1;
}

struct allocated_block *find_process(int pid) {
  struct allocated_block *ab = allocated_block_head;
  while (ab != NULL) {
    if (ab->pid == pid) {
      return ab;
    }
    ab = ab->next;
  }
}

void kill_process() {
  struct allocated_block *ab;
  int pid;
  printf("Kill Process, pid=");
  scanf("%d", &pid);
  ab = find_process(pid);
  if (ab != NULL) {
    free_mem(ab); /*释放 ab 所表示的分配区*/
    dispose(ab);  /*释放 ab 数据结构节点*/
  }
  flag = 1;
}

int free_mem(struct allocated_block *ab) {
  int algorithm = ma_algorithm;
  struct free_block_type *fbt;
  fbt = (struct free_block_type *)malloc(sizeof(struct free_block_type));
  if (!fbt) return -1;
  // 将结点插入链表并进行合并操作, 完成后按照当前算法进行排序
  fbt->size = ab->size;
  fbt->start_addr = ab->start_addr;
  fbt->next = free_block;
  free_block = fbt;
  rearrange(MA_FF);
  fbt = free_block;
  while (fbt->next != NULL) {
    if (fbt->start_addr + fbt->size == fbt->next->start_addr) {
      fbt->size += fbt->next->size;
      struct free_block_type *next = fbt->next;
      fbt->next = next->next;
      free(next);
    } else {
      fbt = fbt->next;
    }
  }
  rearrange(ma_algorithm);
  return 1;
}

int dispose(struct allocated_block *free_ab) {
  struct allocated_block *pre, *ab;
  if (free_ab == allocated_block_head) { /*如果要释放第一个节点*/
    allocated_block_head = allocated_block_head->next;
    free(free_ab);
    return 1;
  }
  pre = allocated_block_head;
  ab = allocated_block_head->next;
  while (ab != free_ab) {
    pre = ab;
    ab = ab->next;
  }
  pre->next = ab->next;
  free(ab);
  return 2;
}

int display_mem_usage() {
  flag = 1;
  struct free_block_type *fbt = free_block;
  struct allocated_block *ab = allocated_block_head;
  if (fbt == NULL) return (-1);
  printf("----------------------------------------------------------\n");
  /* 显示空闲区 */
  printf("Free Memory:\n");
  printf("%20s %20s\n", " start_addr", " size");
  while (fbt != NULL) {
    printf("%20d %20d\n", fbt->start_addr, fbt->size);
    fbt = fbt->next;
  }
  /* 显示已分配区 */
  printf("\nUsed Memory:\n");
  printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr", " size");
  while (ab != NULL) {
    printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name, ab->start_addr,
           ab->size);
    ab = ab->next;
  }
  printf("----------------------------------------------------------\n");
  return 0;
}

void do_exit() {
  while (free_block != NULL) {
    struct free_block_type *next = free_block->next;
    free(free_block);
    free_block = next;
  }
  while (allocated_block_head != NULL) {
    struct allocated_block *next = allocated_block_head->next;
    free(allocated_block_head);
    allocated_block_head = next;
  }
}
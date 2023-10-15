#include "kmm.h"

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

void set_mem_size() {
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
}

void rearrange(int algorithm) {
  switch (algorithm) {
    case MA_FF:
      rearrange_FF();
      break;
    case MA_BF:
      rearrange_BF();
      break;
    case MA_WF:
      rearrange_WF();
      break;
  }
}

static int comp_FF(const void *a, const void *b) {
  if ((*(struct free_block_type **)a)->start_addr <
      (*(struct free_block_type **)b)->start_addr)
    return -1;
  if ((*(struct free_block_type **)a)->start_addr =
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
  if ((*(struct free_block_type **)a)->size =
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
  if ((*(struct free_block_type **)a)->size =
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
  free_block = blocks[0];
}

void rearrange_FF() {
  // 按start_addr从低到高排序
  // 简便起见, 采用选择排序, 可优化为其他更快的排序方式
  // 为尽可能贴近裸机实现,不调用malloc进行动态分配,
  // 因此提前开一个足够大的指针数组
  struct free_block_type *blocks[DEFAULT_MEM_SIZE / MIN_SLICE];
  // 将链表装进指针数组中
  int block_count = fill_in_array(block_count);
  // 接下来进行排序
  qsort(blocks, block_count, sizeof(struct free_block_type *), comp_FF);
  // 排好序后将数组顺序作为链表顺序重排链表
  rearrange_list(blocks, block_count);
  blocks[block_count - 1]->next = NULL;
  free_block = blocks[0];
}

void rearrange_BF() {
  // 按size从小到大排序
  // 思路与FF算法一致
  struct free_block_type *blocks[DEFAULT_MEM_SIZE / MIN_SLICE];
  int block_count = fill_in_array(block_count);
  qsort(blocks, block_count, sizeof(struct free_block_type *), comp_BF);
  rearrange_list(blocks, block_count);
  blocks[block_count - 1]->next = NULL;
  free_block = blocks[0];
}

void rearrange_WF() {
  // 按size从大到小排序
  // 思路与FF算法一致
  struct free_block_type *blocks[DEFAULT_MEM_SIZE / MIN_SLICE];
  int block_count = fill_in_array(block_count);
  qsort(blocks, block_count, sizeof(struct free_block_type *), comp_WF);
  rearrange_list(blocks, block_count);
  blocks[block_count - 1]->next = NULL;
  free_block = blocks[0];
}

int new_process() {
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
  //  3.
  //  找不可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术，进行空闲分区的合并，然后再分配
  // 4. 在成功分配内存后，应保持空闲分区按照相应算法有序
  // 5. 分配成功则返回 1，否则返回-1
  // 请自行补充。。。。。
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
}

int free_mem(struct allocated_block *ab) {
  int algorithm = ma_algorithm;
  struct free_block_type *fbt, *pre, *work;
  fbt = (struct free_block_type *)malloc(sizeof(struct free_block_type));
  if (!fbt) return -1;
  // 进行可能的合并，基本策略如下
  // 1. 将新释放的结点插入到空闲分区队列末尾
  // 2. 对空闲链表按照地址有序排列
  // 3. 检查并合并相邻的空闲分区
  // 4. 将空闲链表重新按照当前算法排序
  // 请自行补充……
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
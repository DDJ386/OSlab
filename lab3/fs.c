#include "fs.h"

/* 磁盘大小及块大小定义 */
#define DISK_SIZE 64 * 1024 * 1024
#define BLOCK_SIZE 4096

/*
整个磁盘被分为2^14个块, 因此设计INODE占用512个块, 超级块占用1个块,
两个位图各占用1个块, 因此DATA占用2^14-514个块
*/

/* 根据磁盘大小和块大小定义的区域大小 */
#define SUPER_BLOCK_BEGIN 0
#define SUPER_BLOCK_SIZE (BLOCK_SIZE)
#define I_BMAP_BEGIN (SUPER_BLOCK_BEGIN + SUPER_BLOCK_SIZE)
#define I_BMAP_REGION_SIZE (1 * BLOCK_SIZE)
#define I_BMAP_SIZE (INODE_REGION_SIZE / INODE_SIZE)
#define D_BMAP_BEGIN (I_BMAP_BEGIN + I_BMAP_REGION_SIZE)
#define D_BMAP_REGION_SIZE (1 * BLOCK_SIZE)
#define D_BMAP_SIZE (DATA_REGION_SIZE / BLOCK_SIZE)
#define INODE_BEGIN (D_BMAP_BEGIN + D_BMAP_REGION_SIZE)
#define INODE_REGION_SIZE (512 * BLOCK_SIZE)
#define INODE_SIZE 128
#define DATA_REGION_BEGIN (INODE_BEGIN + INODE_REGION_SIZE)
#define DATA_REGION_SIZE (((1 << 14) - 512) * BLOCK_SIZE)

#define MAX_OPEN_FILE 256
#define MAX_PATH_LEN 256

int uid = 0;
char i_bmap[I_BMAP_SIZE];
char d_bmap[D_BMAP_SIZE];
FILE *disk;
super_block sb;
openfile_table_entry open_file_table[MAX_OPEN_FILE] = {0};  // 管理打开的文件

char path[MAX_PATH_LEN] = {'/'};  // 当前路径
int curr_dir_inode = 0;           // 当前目录的inode编号

/*==========================================================================================================*/

/* static functions */
static void format_fs();
static int set_i_bmap_1(int inode_num);
static void set_i_bmap_0(int inode_num);
static int set_d_bmap_1(int addr);
static void set_d_bmap_0(int addr);
static int alloc_inode();
static int alloc_block();
static void free_inode(int inode_num);
static void free_block(int addr);
static void read_inode(int inode_num, inode *buf);
static void write_inode(int inode_num, inode *buf);
static int get_data_block_addr(int inode_num, int offset);
static void read_data_block(int inode_num, int offset, void *buf);
static void write_data_block(int inode_num, int offset, void *buf, int size);
static int seek_free_open_file_table_entry();
static int file_size_adjust(int inode_num, int size);

static void format_fs() {
  sb.sb_block_size = BLOCK_SIZE;
  sb.sb_disk_size = DISK_SIZE;
  sb.inode_unused = INODE_REGION_SIZE / INODE_SIZE;
  sb.blk_unused = DATA_REGION_SIZE / BLOCK_SIZE;
  strcpy(sb.sb_volume_name, "my file system");
  i_bmap[0] = 1;
  int blk_index = alloc_block();
  assert(blk_index != -1);
  inode buf;
  read_inode(0, &buf);
  buf.block_addr[0] = blk_index;
  buf.limitation = READ | WRITE;
  buf.size = 0;
  buf.type = DIRECTORY;
  time(&buf.create_time);
  buf.attach_time = buf.create_time;
  buf.modify_time = buf.modify_time;
  write_inode(0, &buf);
  dir dir_entry = {0, ".", DIRECTORY};
  int fd = open_file(0);
  write_file(fd, sizeof(dir), (char *)&dir_entry);
  close_file(fd);
}

static int set_i_bmap_1(int inode_num) {
  int byte_index = inode_num / sizeof(char);
  int bit_index = inode_num % sizeof(char);
  char bit_mask = (char[]){1, 2, 4, 8, 16, 32, 64, 128}[bit_index];
  if (i_bmap[byte_index] & bit_mask != 0)
    return -1;
  else
    i_bmap[byte_index] ^= bit_mask;
}

static void set_i_bmap_0(int inode_num) {
  int byte_index = inode_num / sizeof(char);
  int bit_index = inode_num % sizeof(char);
  char bit_mask = (char[]){1, 2, 4, 8, 16, 32, 64, 128}[bit_index];
  i_bmap[byte_index] &= (~bit_mask);
}

static int set_d_bmap_1(int addr) {
  int byte_index = addr / sizeof(char);
  int bit_index = addr % sizeof(char);
  char bit_mask = (char[]){1, 2, 4, 8, 16, 32, 64, 128}[bit_index];
  if (i_bmap[byte_index] & bit_mask != 0)
    return -1;
  else
    i_bmap[byte_index] ^= bit_mask;
}

static void set_d_bmap_0(int addr) {
  int byte_index = addr / sizeof(char);
  int bit_index = addr % sizeof(char);
  char bit_mask = (char[]){1, 2, 4, 8, 16, 32, 64, 128}[bit_index];
  i_bmap[byte_index] &= (~bit_mask);
}

static int alloc_inode() {
  static int last_alloc = 0;
  if (!sb.inode_unused) return -1;
  for (int i = (last_alloc + 1) % I_BMAP_SIZE; i != last_alloc; i++) {
    if (i + 1 % I_BMAP_SIZE == 0) i = 0;
    if (set_i_bmap_1(i) != -1) {
      last_alloc = i;
      sb.inode_unused--;
      return i;
    }
  }
  return -1;
}

static int alloc_block() {
  static int last_alloc = 0;
  if (!sb.blk_unused) return -1;
  for (int i = (last_alloc + 1) % D_BMAP_SIZE; i != last_alloc; i++) {
    if (i + 1 % D_BMAP_SIZE == 0) i = 0;
    if (set_i_bmap_1(i) != -1) {
      last_alloc = i;
      sb.blk_unused--;
      return i;
    }
  }
  return -1;
}

static void free_inode(int inode_num) {
  inode file;
  read_inode(inode_num, &file);
  for (int i = 0; i < 11; i++) {
    if (file.block_addr[i] != 0) {
      free_block(file.block_addr[i]);
    }
  }
  if (file.block_addr[11] != 0) {
    int addr_table[512];
    int block_addr = file.block_addr[11];
    fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * block_addr, SEEK_SET);
    fread(addr_table, BLOCK_SIZE, 1, disk);
    for (int i = 0; i < 512; i++) {
      if (addr_table[i] != 0) {
        free_block(addr_table[i]);
        addr_table[i] = 0;
      }
    }
    fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * block_addr, SEEK_SET);
    fwrite(addr_table, BLOCK_SIZE, 1, disk);
    file.block_addr[11] = 0;
  }
  set_i_bmap_0(inode_num);
  write_inode(inode_num, &file);
}

static void free_block(int addr) {
  set_d_bmap_0(addr);
  /* (可选)刷新block */
  char buf[BLOCK_SIZE] = {0};
  fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * addr, SEEK_SET);
  fwrite(buf, BLOCK_SIZE, 1, disk);
}

/* 读inode块 */
static void read_inode(int inode_num, inode *buf) {
  fseek(disk, INODE_BEGIN + inode_num * sizeof(inode), SEEK_SET);
  fread(buf, sizeof(inode), 1, disk);
}

/* 写回inode块 */
static void write_inode(int inode_num, inode *buf) {
  fseek(disk, INODE_BEGIN + inode_num * sizeof(inode), SEEK_SET);
  fwrite(buf, sizeof(inode), 1, disk);
}

/* 根据文件inode, offset, 计算出data块的地址 */
static int get_data_block_addr(int inode_num, int offset) {
  /* 读取文件inode信息 */
  inode file;
  read_inode(inode_num, &file);

  /* 获取data块的地址 */
  int block_addr;
  assert(offset <= FILE_SIZE);
  if (offset > FILE_SIZE) {
    /* 偏移量大于文件大小, 不合法 */
    // printf("wrong address.\n");
    return -1;
  } else if (offset > 11 * BLOCK_SIZE) {
    /* offset 位于二级索引处 */
    int addr_table[512];
    block_addr = file.block_addr[11];
    fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * block_addr, SEEK_SET);
    fread(addr_table, BLOCK_SIZE, 1, disk);
    block_addr = addr_table[offset / BLOCK_SIZE - 11];
  } else {
    /* offset 位于一级索引处*/
    block_addr = file.block_addr[offset / BLOCK_SIZE];
  }
  return block_addr;
}

/* 读data块 */
static void read_data_block(int inode_num, int offset, void *buf) {
  int block_addr = get_data_block_addr(inode_num, offset);
  fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * block_addr, SEEK_SET);
  fread(buf, BLOCK_SIZE, 1, disk);
}

/* 写回data块*/
static void write_data_block(int inode_num, int offset, void *buf, int size) {
  int block_addr = get_data_block_addr(inode_num, offset);
  int rest_size = BLOCK_SIZE - (offset % BLOCK_SIZE);
  int write_size = rest_size > size ? size : rest_size;
  /* 向数据块写回数据 */
  fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * block_addr, SEEK_SET);
  fwrite(buf, write_size, 1, disk);
}

/* 寻找空闲的FCB块 */
static int seek_free_open_file_table_entry() {
  for (int i = 0; i < MAX_OPEN_FILE; i++) {
    if (open_file_table[i].used == 0) {
      return i;
    }
  }
  return -1;
}

/* 调整文件大小 */
static int file_size_adjust(int inode_num, int size) {
  inode file;
  read_inode(inode_num, &file);
  /* inode 中应有的地址数量 */
  int adddr_array_edge;
  if (size > FILE_SIZE) {
    return -1;
  } else if (size > BLOCK_SIZE * 11) {
    adddr_array_edge = 11;
  } else {
    adddr_array_edge = size / BLOCK_SIZE;
  }
  /* 从第0个开始检查, 该位是否还未被申请 */
  for (int i = 0; i < adddr_array_edge + 1; i++) {
    /* 当地址数为11时, 说明启用了二级索引, 因此读取二级索引的信息进行检查 */
    if (i == 11) {
      /* 二级索引数据的缓冲区 */
      int addr_table[512];
      /* 读取整个块的信息到缓冲区中 */
      fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * file.block_addr[11],
            SEEK_SET);
      fread(addr_table, BLOCK_SIZE, 1, disk);
      /* 二级索引中启用的地址的数量 */
      int blocks = size / BLOCK_SIZE - 11;
      /* 检查二级索引中所有地址的启用情况是否符合要求,
       * 若不符合则进行分配或释放*/
      for (int j = 0; j < blocks + 1; j++) {
        if (addr_table[j] == 0) {
          addr_table[j] = alloc_block();
        }
      }
      for (int j = blocks + 1; j < 512; j++) {
        if (addr_table[j] != 0) {
          free_block(addr_table[j]);
          addr_table[j] = 0;
        }
      }
      /* 写回二级索引块的信息 */
      fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * file.block_addr[11],
            SEEK_SET);
      fwrite(addr_table, BLOCK_SIZE, 1, disk);
    }
    if (file.block_addr[i] == 0) {
      file.block_addr[i] = alloc_block();
    }
  }

  /* 从边界后一个开始检查, 每个地址的启用情况是否符合要求,
   * 若不符合则进行分配或释放 */
  for (int i = adddr_array_edge + 1; i < 12; i++) {
    /* 若二级索引被启用, 则检查里面是否有被启用的地址, 然后将其设为未启用 */
    if (i == 11 && file.block_addr[i] != 0) {
      int addr_table[512];
      fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * file.block_addr[11],
            SEEK_SET);
      fread(addr_table, BLOCK_SIZE, 1, disk);
      for (int j = 0; j < 512; j++) {
        if (addr_table[j] != 0) {
          free_block(addr_table[j]);
          addr_table[j] = 0;
        }
      }
      fseek(disk, DATA_REGION_BEGIN + BLOCK_SIZE * file.block_addr[11],
            SEEK_SET);
      fwrite(addr_table, BLOCK_SIZE, 1, disk);
      file.block_addr[i] = 0;
    } else if (file.block_addr[i] != 0) {
      free_block(file.block_addr[i]);
      file.block_addr[i] = 0;
    }
  }
  write_inode(inode_num, &file);
  return 1;
}

/* 刷新缓冲区 */
void flush_buffer(char *buf, int size) {
  for (int i = 0; i < size; i++) {
    buf[i] = '\0';
  }
}
/*==========================================================================================================*/

/* api接口 */
int init_fs() {
  disk = fopen("./disk", "r+");
  fseek(disk, SUPER_BLOCK_BEGIN, SEEK_SET);
  fread(&sb, SUPER_BLOCK_SIZE, 1, disk);
  /* 读取位图信息 */
  fseek(disk, I_BMAP_BEGIN, SEEK_SET);
  fread(i_bmap, I_BMAP_REGION_SIZE, 1, disk);
  fseek(disk, D_BMAP_BEGIN, SEEK_SET);
  fread(d_bmap, D_BMAP_REGION_SIZE, 1, disk);
  /* 若root结点未初始化, 则初始化它 */
  if (!i_bmap[0]) {
    format_fs();
  }
}

void close_fs() {
  // TODO
  fseek(disk, SUPER_BLOCK_BEGIN, SEEK_SET);
  fwrite(&sb, SUPER_BLOCK_SIZE, 1, disk);
  fseek(disk, I_BMAP_BEGIN, SEEK_SET);
  fwrite(i_bmap, I_BMAP_REGION_SIZE, 1, disk);
  fseek(disk, D_BMAP_BEGIN, SEEK_SET);
  fwrite(d_bmap, D_BMAP_REGION_SIZE, 1, disk);
}

int open_file(int inode_num) {
  int ofte_index = seek_free_open_file_table_entry();
  assert(ofte_index != -1);
  inode file_inode;
  read_inode(inode_num, &file_inode);
  if (file_inode.uid != 0 && file_inode.uid != uid) return -1;
  open_file_table[ofte_index].inode_num = inode_num;
  open_file_table[ofte_index].offset = 0;
  open_file_table[ofte_index].size = file_inode.size;
  open_file_table[ofte_index].limitation = file_inode.limitation;
  open_file_table[ofte_index].used = 1;
  open_file_table[ofte_index].type = file_inode.type;
  time(&open_file_table[ofte_index].attach_time);
  return ofte_index;
}

void close_file(int fd) {
  inode file;
  openfile_table_entry *ofte = &open_file_table[fd];
  if (ofte->used == 0) return;
  read_inode(ofte->inode_num, &file);
  file.size = ofte->size;
  file.attach_time = ofte->attach_time;
  file.limitation = ofte->limitation;
  write_inode(ofte->inode_num, &file);
  ofte->used = 0;
}

int read_file(int fd, int size, char *buffer) {
  openfile_table_entry *ofte = &open_file_table[fd];
  /* 若没有读取权限, 则直接返回 */
  if (ofte->limitation & READ == 0) return -1;
  /* 更新访问时间 */
  time(&ofte->attach_time);
  /* 每次从磁盘中读取文件的一个块, 若不够则继续取一个块 */
  char buf[BLOCK_SIZE];
  int inbolck_offset = open_file_table[fd].offset % BLOCK_SIZE;
  read_data_block(ofte->inode_num, ofte->offset, buf);

  /* 若 size == -1 则读取文件至文件末尾*/
  if (size == -1) size = ofte->size - open_file_table[fd].offset;

  /* 记录真正读取的字节数, 作为返回值 */
  int real_read_size = 0;
  while (size-- > 0 && open_file_table[fd].offset++ < ofte->size) {
    if (inbolck_offset + 1 % BLOCK_SIZE == 0) {
      read_data_block(ofte->inode_num, ofte->offset, buf);
      inbolck_offset = 0;
    }
    buffer[real_read_size++] = buf[inbolck_offset++];
  }
  return real_read_size;
}

int write_file(int fd, int size, char *buffer) {
  openfile_table_entry *ofte = &open_file_table[fd];
  if (ofte->limitation & WRITE == 0) return -1;
  time(&ofte->modify_time);
  if (file_size_adjust(ofte->inode_num, ofte->offset + size) == -1) return -1;
  ofte->size = ofte->offset + size;
  int buffer_offset = 0;
  while (size > 0) {
    /* 写入数据大小 write_size = min{该块剩余大小, size} */
    int write_size = BLOCK_SIZE - (ofte->offset % BLOCK_SIZE);
    if (write_size > size) write_size = size;
    write_data_block(ofte->inode_num, ofte->offset, buffer, write_size);
    ofte->offset += write_size;
    size -= write_size;
    buffer += write_size;
  }
  return 1;
}

int file_set_offset(int fd, int offset) {
  inode file_inode;
  read_inode(open_file_table[fd].inode_num, &file_inode);
  if (offset < file_inode.size) {
    open_file_table[fd].offset = offset;
    return offset;
  } else
    return -1;
}

int find_file(const char *name) {
  int position_inode_num = curr_dir_inode;
  const char *ptname = name;
  char next_node_name[MAX_PATH_LEN];
  while (*ptname != '\0') {
    dir file_entry;
    int dir_entry = open_dir(position_inode_num);
    int i = 0;

    /* 提取路径上的下一个目录名 */
    while (*ptname != '/' && *ptname != '\0') {
      next_node_name[i++] = *ptname;
      ptname++;
    }
    next_node_name[i] = '\0';

    /* 提取出下一个文件或目录名后, 在当前目录中进行查找,
     * 若找到则将当前目录设置为找到的值*/
    int find = 0;
    while (read_dir(dir_entry, 1, &file_entry) == sizeof(file_entry)) {
      if (strcmp(file_entry.file_name, next_node_name) == 0) {
        position_inode_num = file_entry.inode_num;
        find = 1;
        break;
      }
    }
    close_dir(dir_entry);
    if (!find) {
      // printf("file not find\n");
      return -1;
    }
    if (*ptname == '/') ptname++;
  }
  return position_inode_num;
}

void get_path(char *dstpath) { strcpy(dstpath, path); }

void set_path(const char *dstpath) {
  if (strlen(dstpath) > MAX_PATH_LEN) return;
  strcpy(path, dstpath);
}

int open_dir(int inode_num) {
  int ofte_index = open_file(inode_num);
  if (open_file_table[ofte_index].type != DIRECTORY) {
    close_file(inode_num);
    return -1;
  }
  return ofte_index;
}

int read_dir(int fd, int count, dir *buffer) {
  int size = count * sizeof(dir);
  return read_file(fd, size, (char *)buffer);
}

void close_dir(int fd) { close_file(fd); }

int remove_file(int file_inode, int dir_inode) {
  int fd = open_dir(dir_inode);
  dir directory[FILE_SIZE / sizeof(dir)];
  read_file(fd, -1, (char *)directory);
  for (int i = 0; i < FILE_SIZE / sizeof(dir); i++) {
    if (directory[i].inode_num == file_inode) {
      if (directory[i].type == DIRECTORY) return -1;
      directory[i].inode_num = -1;
      free_inode(file_inode);
      break;
    }
  }
  file_set_offset(fd, 0);
  write_file(fd, open_file_table[fd].size, (char *)directory);
  close_dir(fd);
}

int create_file(const char *name, int dir_inode) {
  if (find_file(name) >= 0) return -1;
  int fd = open_dir(dir_inode);
  dir directory[FILE_SIZE / sizeof(dir)];
  read_file(fd, -1, (char *)directory);
  file_set_offset(fd, 0);
  int index = -1;
  for (int i = 0; i < open_file_table[fd].size / sizeof(dir); i++) {
    if (directory[i].inode_num == -1) {
      index = i;
      break;
    }
  }
  if (index == -1) {
    open_file_table[fd].size += sizeof(dir);
    index = open_file_table[fd].size / sizeof(dir) - 1;
  }
  int new_inode = alloc_inode();
  directory[index].inode_num = new_inode;
  directory[index].type = REGULAR_FILE;
  strcpy(directory[index].file_name, name);
  inode file;
  read_inode(new_inode, &file);
  file.limitation = READ | WRITE;
  file.size = 0;
  file.type = REGULAR_FILE;
  file.uid = uid;
  time(&file.create_time);
  file.attach_time = file.create_time;
  file.modify_time = file.create_time;
  write_inode(new_inode, &file);
  write_file(fd, open_file_table[fd].size, (char *)directory);
  close_dir(fd);
}

int create_dir(const char *name, int dir_inode) {
  int fd = open_dir(dir_inode);
  dir directory[FILE_SIZE / sizeof(dir)];
  read_file(fd, -1, (char *)directory);
  file_set_offset(fd, 0);
  /* 寻找一个空闲的dir块 */
  int index = -1;
  for (int i = 0; i < open_file_table[fd].size / sizeof(dir); i++) {
    if (directory[i].inode_num == -1) {
      index = i;
      break;
    }
  }
  if (index == -1) {
    open_file_table[fd].size += sizeof(dir);
    index = open_file_table[fd].size / sizeof(dir) - 1;
  }
  /* 修改当前目录 */
  int new_inode = alloc_inode();
  directory[index].inode_num = new_inode;
  directory[index].type = DIRECTORY;
  strcpy(directory[index].file_name, name);
  /* 初始化新的目录inode */
  inode file;
  read_inode(new_inode, &file);
  file.limitation = READ | WRITE;
  file.size = 0;
  file.type = DIRECTORY;
  file.uid = uid;
  time(&file.create_time);
  file.attach_time = file.create_time;
  file.modify_time = file.create_time;
  file.block_addr[0] = alloc_block();
  write_inode(new_inode, &file);
  write_file(fd, open_file_table[fd].size, (char *)directory);
  close_dir(fd);
  /* 向新目录中添加两个特殊的目录 */
  dir dir_entry[2] = {{new_inode, ".", DIRECTORY},
                      {dir_inode, "..", DIRECTORY}};
  fd = open_file(new_inode);
  write_file(fd, 2 * sizeof(dir), (char *)dir_entry);
  close_file(fd);
}

int remove_dir(int inode_num, int dir_inode) {
  /* 读取这个目录文件 */
  int rmfd = open_dir(inode_num);
  dir rmdir[BLOCK_SIZE * 11 / sizeof(dir)];
  read_file(rmfd, BLOCK_SIZE, (char *)rmdir);
  /* 删除这个目录文件中的所有文件 */
  for (int i = 2; i < open_file_table[rmfd].size / sizeof(dir); i++) {
    if (rmdir[i].inode_num != 0 && rmdir[i].inode_num != -1) {
      if (rmdir[i].type == DIRECTORY)
        remove_dir(rmdir[i].inode_num, inode_num);
      else
        remove_file(rmdir[i].inode_num, inode_num);
    }
  }
  close_dir(inode_num);
  /* 删除这个目录文件 */
  free_inode(inode_num);
  /* 修改本层目录项 */
  int fd = open_dir(dir_inode);
  dir directory[BLOCK_SIZE * 11 / sizeof(dir)];
  read_file(fd, -1, (char *)directory);
  for (int i = 0; i < open_file_table[fd].size / sizeof(dir); i++) {
    if (directory[i].inode_num == inode_num) {
      directory[i].inode_num = -1;
    }
  }
  file_set_offset(fd, 0);
  write_file(fd, open_file_table[fd].size, (char *)directory);
  close_dir(fd);
}

/*==========================================================================================================*/

/* 命令层 */
void ls() {
  int fd = open_file(curr_dir_inode);
  dir directory[FILE_SIZE / sizeof(dir)];
  read_file(fd, -1, (char *)directory);
  for (int i = 0; i < open_file_table[fd].size / sizeof(dir); i++) {
    if (directory[i].inode_num == -1) continue;
    inode file_inode;
    read_inode(directory[i].inode_num, &file_inode);
    char limitation[5];
    limitation[0] = "_d"[file_inode.type - 1];
    limitation[1] = "_r"[(file_inode.limitation & READ) != 0];
    limitation[2] = "_w"[(file_inode.limitation & WRITE) != 0];
    limitation[3] = "_x"[(file_inode.limitation & EXECUTE) != 0];
    limitation[4] = '\0';
    printf(
        "%s\t%s\tuser:%d\n"
        "creat time:\t%s"
        "attach time:\t%s"
        "modify time:\t%s",
        directory[i].file_name, limitation, file_inode.uid,
        ctime(&file_inode.create_time), ctime(&file_inode.attach_time),
        ctime(&file_inode.modify_time));
  }
  close_file(fd);
}

void create() {
  char name[256];
  scanf("%s", name);
  if (find_file(name) >= 0) {
    printf("\"%s\" has already existed", name);
    return;
  }
  create_file(name, curr_dir_inode);
}

void mkdir() {
  char name[256];
  scanf("%s", name);
  if (find_file(name) >= 0) {
    printf("\"%s\" has already existed", name);
    return;
  }
  create_dir(name, curr_dir_inode);
}

void rmdir() {
  char name[256];
  scanf("%s", name);
  int inode_num = find_file(name);
  inode file_inode;
  read_inode(inode_num, &file_inode);
  if (inode_num == -1) {
    printf("\"%s\" not found.\n", name);
    return;
  }
  if (file_inode.uid != 0 && file_inode.uid != uid) {
    printf("you are not permitted\n");
    return;
  }
  if (file_inode.type != DIRECTORY) {
    printf("\"%s\" is not a directory", name);
    return;
  }
  remove_dir(inode_num, curr_dir_inode);
}

void delete() {
  char name[256];
  scanf("%s", name);
  int inode_num = find_file(name);
  inode file_inode;
  read_inode(inode_num, &file_inode);
  if (inode_num == -1) {
    printf("\"%s\" not found.\n", name);
    return;
  }
  if (file_inode.uid != 0 && file_inode.uid != uid) {
    printf("you are not permitted\n");
    return;
  }
  remove_file(inode_num, curr_dir_inode);
}

void cd() {
  char name[256];
  scanf("%s", name);
  int file_inode_num = find_file(name);
  if (file_inode_num == -1) {
    printf("\"%s\" not found.\n", name);
    return;
  }
  inode file_inode;
  read_inode(file_inode_num, &file_inode);
  if (file_inode.uid != 0 && file_inode.uid != uid) {
    printf("you are not permitted\n");
    return;
  }
  if (file_inode.type != DIRECTORY) {
    printf("\"%s\" is not a directory", name);
    return;
  }
  curr_dir_inode = file_inode_num;
  /* 修改路径 */
  char pathbuf[512] = {0};
  strcpy(pathbuf, path);
  char *p1 = pathbuf + strlen(path) - 1;
  const char *p2 = name;
  while (*p2 != '\0') {
    if (*p2 == '.' && *(p2 + 1) == '/') {
      p2 += 2;
    } else if (*p2 == '.' && *(p2 + 1) == '.') {
      do {
        *(p1--) = '\0';
      } while (*p1 != '/');
      *(p1--) = '\0';
      p2 += 2;
    } else if (*p2 == '.' && *(p2 + 1) == '\0') {
      p2++;
    } else {
      *(++p1) = *(p2++);
    }
  }
  *(++p1) = '/';
  if (strlen(pathbuf) > 255) {
    printf("the path is too long");
  }
  strncpy(path, pathbuf, 255);
}

void open() {
  char name[256];
  scanf("%s", name);
  int inode_num = find_file(name);
  if (inode_num == -1) {
    printf("\"%s\" is not found", name);
    return;
  }
  int fd = open_file(inode_num);
  printf("fd = %d", fd);
}

void close() {
  int fd;
  scanf("%d", &fd);
  close_file(fd);
}

void read() {
  int fd;
  scanf("%d", &fd);
  char buffer[FILE_SIZE];
  read_file(fd, -1, buffer);
  printf("%s", buffer);
  flush_buffer(buffer, FILE_SIZE);
}

void write() {
  int fd;
  scanf("%d", &fd);
  char buffer[FILE_SIZE];
  int size = 0;
  char c = getchar();
  while (c == ' ') {
    c = getchar();
  }
  if (c != '\"') {
    printf("please put the text in quotation marks");
    return;
  }
  while ((c = getchar()) != '\"' && size < FILE_SIZE) {
    buffer[size++] = c;
  }
  if (size == FILE_SIZE) printf("the text is too long !");
  write_file(fd, size, buffer);
}

void cat() {
  char name[256];
  scanf("%s", name);
  int file_inode = find_file(name);
  if (file_inode == -1) {
    printf("\"%s\" does not exist\n", name);
    return;
  }
  char buffer[FILE_SIZE];
  int fd = open_file(file_inode);
  read_file(fd, -1, buffer);
  close_file(fd);
  printf("%s", buffer);
}

void chmod() {
  char name[256];
  scanf("%s", name);
  int limitation;
  scanf("%d", &limitation);
  int inode_num = find_file(name);
  inode file_inode;
  read_inode(inode_num, &file_inode);
  if (file_inode.uid != 0 && file_inode.uid != uid) {
    printf("you don't have permission\n");
    return;
  }
  file_inode.limitation = limitation;
  write_inode(inode_num, &file_inode);
}

void login() {
  printf("please input your uid: ");
  int temp_uid;
  scanf("%d", &temp_uid);
  user *logging_in = 0;
  for (int i = 0; i < 63; i++) {
    if (sb.users[i].uid == temp_uid) {
      logging_in = &sb.users[i];
      break;
    }
  }
  if (logging_in == 0) {
    printf("there is no user: %d", uid);
    return;
  }
  printf("please input password: ");
  char password[56];
  scanf("%s", password);

  for (int i = 0; i < 4; i++) {
    if (strcmp(password, logging_in->password) == 0) {
      printf("welcome!\n");
      uid = temp_uid;
      return;
    } else
      printf(
          "please try again\n"
          "please input password: ");
    scanf("%s", password);
  }
  printf("login faild\n");
  return;
}

void signin() {
  printf("please input your uid: ");
  int temp_uid;
  scanf("%d", &temp_uid);
  for (int i = 0; i < 63; i++) {
    if (sb.users[i].uid == temp_uid) {
      printf("this uid has been used, please try again: ");
      scanf("%d", &temp_uid);
      i = 0;
    }
  }
  printf("please input password: ");
  char password[56];
  scanf("%s", password);
  for (int i = 0; i < 63; i++) {
    if (sb.users[i].uid == 0) {
      sb.users[i].uid = temp_uid;
      strcpy(sb.users[i].password, password);
      return;
    }
  }
  printf("sorry, the user count is full\n");
}
void logout() { uid = 0; }
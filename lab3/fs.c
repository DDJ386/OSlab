#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUMK_SIZE 4096
#define BLOCK_SIZE 4096
#define INODE_NUM 4000
#define BLOCK_NUM 1 << 14
#define BLK_PER_FILE 32
#define SUPBLK_BGN 0
#define SUPBLK_CHUNK_COUNT (sizeof(super_block) / CHUMK_SIZE + 1)
#define INODE_BGN (SUPBLK_CHUNK_COUNT * CHUMK_SIZE + SUPBLK_BGN)
#define INODE_CHUNK_COUNT ((sizeof(inode) * INODE_NUM) / CHUMK_SIZE + 1)
#define BLK_BGN (INODE_CHUNK_COUNT * CHUMK_SIZE + INODE_BGN)
#define BLK_ADDR(index) (index * CHUMK_SIZE + BLK_BGN)
#define MAX_FILE_SIZE (CHUMK_SIZE * BLK_PER_FILE)

typedef struct {
  char i_bmap[INODE_NUM];
  char d_bmap[BLOCK_NUM];
  int inode_used;
  int blk_used;
} super_block;

typedef struct {
  int limitation;                             // 读写权限 read|write|execute
  enum { DIRECTORY = 0, REGULAR_FILE } type;  // 文件类型
  int size;                                   // 文件大小
  long block_addr[BLK_PER_FILE];
} inode;

typedef struct {
  int idnode_num;
  char file_name[28];
} dir;

FILE *disk;
super_block sb;
inode inodes[INODE_NUM];
char path[256] = {'/'};  // 当前路径
int curr_dir_inode = 0;  // 当前目录的inode编号
char buf[BLK_PER_FILE * CHUMK_SIZE];

void print_welcom_data();
void flush_buffer(char *buf, int size);
int alloc_inode();
int alloc_blk();
void free_inode(int inode);
void free_blk(int inode);
void fs_init();
void fs_close();
void list_dir(int inode);
void enter_dir(int inode, const char *name);
// void make_dir(int inode, const char *name);
void open_file(int inode);
void close_file(int inode);
void print_file(int inode);
void make_file(int inode, const char *name, int mode);
int find_file(int inode, const char *name);
void remove_file(int inode, const char *name);
void write_file(int inode, const char *name);

void print_welcom_data() {
  // TODO
}

void flush_buffer(char *buf, int size) {
  for (int i = 0; i < size; i++) {
    buf[i] = '\0';
  }
}

int alloc_inode() {
  for (int i = 0; i < INODE_NUM; i++) {
    if (sb.i_bmap[i] == 0) {
      sb.i_bmap[i] = 1;
      return i;
    }
  }
  return -1;
}

int alloc_blk() {
  for (int i = 0; i < BLOCK_NUM; i++) {
    if (sb.d_bmap[i] == 0) {
      sb.d_bmap[i] = 1;
      return i;
    }
  }
  return -1;
}

void free_inode(int inode) {
  int chunks = inodes[inode].size / CHUMK_SIZE + 1;
  for (int i = 0; i < chunks; i++) {
    free_blk(inodes[inode].block_addr[i]);
  }
  sb.i_bmap[inode] = 0;
}
void free_blk(int addr) { sb.d_bmap[(addr - BLK_BGN) / CHUMK_SIZE] = 0; }

void fs_init() {
  disk = fopen("./disk", "r+");
  fseek(disk, SUPBLK_BGN, SEEK_SET);
  fread(&sb, sizeof(super_block), 1, disk);
  fseek(disk, INODE_BGN, SEEK_SET);
  fread(inodes, sizeof(inodes), 1, disk);
  if (!sb.i_bmap[0]) {
    sb.i_bmap[0] = 1;
    int blk_index = alloc_blk();
    inodes[0].block_addr[0] = BLK_ADDR(blk_index);
    inodes[0].limitation = 6;
    inodes[0].size = 0;
    inodes[0].type = DIRECTORY;
  }
}

void fs_close() {
  fseek(disk, INODE_BGN, SEEK_SET);
  fwrite(inodes, sizeof(inodes), 1, disk);
  fseek(disk, SUPBLK_BGN, SEEK_SET);
  fwrite(&sb, sizeof(super_block), 1, disk);
  fclose(disk);
}

void list_dir(int inode) {
  open_file(inode);
  dir *dbuf = (dir *)buf;
  for (int i = 0; i < inodes[inode].size / sizeof(dir); i++) {
    if (inodes[dbuf[i].idnode_num].type == DIRECTORY) {
      printf("\033[1;32m%s\t\033[0m", dbuf[i].file_name);
    } else
      printf("%s\t", dbuf[i].file_name);
    if (!(i + 1) % 6) printf("\n");
  }
  close_file(inode);
}

void enter_dir(int inode, const char *name) {
  int file_inode = find_file(inode, name);
  if (file_inode == -1) {
    printf("\"%s\" is not exist", name);
    return;
  }
  if (inodes[file_inode].type != DIRECTORY) {
    printf("\"%s\" is not a directory", name);
    return;
  }
  curr_dir_inode = file_inode;
  /* 修改路径变量 */
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
  if (strlen(pathbuf) > 256) {
    printf("the path is too long");
  }
  strncpy(path, pathbuf, 255);
}

void open_file(int inode) {
  int filesize = inodes[inode].size;
  int i = 0;
  while (filesize > 0) {
    int read_size;
    if (filesize < CHUMK_SIZE)
      read_size = filesize;
    else
      read_size = CHUMK_SIZE;
    fseek(disk, inodes[inode].block_addr[i], SEEK_SET);
    fread(((void *)buf) + i * CHUMK_SIZE, read_size, 1, disk);
    filesize -= CHUMK_SIZE;
    i++;
  }
}

void close_file(int inode) {
  /* 写回文件中的数据 */
  int filesize = inodes[inode].size;
  int i = 0;
  while (filesize > 0) {
    int write_size;
    if (filesize < CHUMK_SIZE)
      write_size = filesize;
    else
      write_size = CHUMK_SIZE;
    fseek(disk, inodes[inode].block_addr[i], SEEK_SET);
    fwrite(((void *)buf) + i * CHUMK_SIZE, write_size, 1, disk);
    filesize -= CHUMK_SIZE;
    i++;
  }
  /* 刷新缓冲区 */
  flush_buffer(buf, BLK_PER_FILE * CHUMK_SIZE);
}

void print_file(int inode) {
  open_file(inode);
  char *fbuf = buf;
  printf("%s", fbuf);
  close_file(inode);
}

int find_file(int inode, const char *name) {
  open_file(inode);
  dir *dbuf = (dir *)buf;
  int ret = -1;
  for (int i = 0; i < inodes[inode].size / sizeof(dir); i++) {
    if (strcmp(name, dbuf[i].file_name) == 0) {
      ret = dbuf[i].idnode_num;
      break;
    }
  }
  close_file(inode);
  return ret;
}

void remove_file(int inode, const char *name) {
  int rm_inode = find_file(inode, name);
  free_inode(rm_inode);
  open_file(inode);
  dir *dbuf = (dir *)buf;
  for (int i = 0; i < inodes[inode].size / sizeof(dir); i++) {
    if (strcmp(dbuf[i].file_name, name) == 0) {
      dbuf[i].idnode_num = -1;
    }
  }
  close_file(inode);
}

void make_file(int inode, const char *name, int mode) {
  open_file(inode);
  /* 寻找空闲位置 */
  dir *bufp = (dir *)buf;
  int index = -1;
  int dirs = inodes[inode].size / sizeof(dir);
  for (int i = 0; i < dirs; i++) {
    if (bufp[i].idnode_num == -1) {
      index = i;
    }
    if (strcmp(bufp[i].file_name, name) == 0) {
      printf("\"%s\" is already exists", name);
      return;
    }
  }
  if (index == -1) {
    index = dirs;
    inodes[inode].size += sizeof(dir);
  }
  /* 若该片满了则申请一个新的片 */
  if ((char *)&bufp[index] - buf > inodes[inode].size) {
    int old_chunks = inodes[inode].size / CHUMK_SIZE;
    int new_chunks = (inodes[inode].size + sizeof(dir)) / CHUMK_SIZE;
    if (old_chunks != new_chunks) {
      if (new_chunks >= MAX_FILE_SIZE) {
        printf("the file is too big\n");
        close_file(inode);
        return;
      }
      inodes[inode].block_addr[new_chunks] = BLK_ADDR(alloc_blk());
    }
  }
  /* 申请一个新的文件 */
  int new_inode = alloc_inode();
  /* 将新目录文件插入buf中(即本层目录文件中) */
  bufp[index].idnode_num = new_inode;
  strcpy(bufp[index].file_name, name);
  close_file(inode);
  /* 初始化新文件 */
  inodes[new_inode].block_addr[0] = BLK_ADDR(alloc_blk());
  switch (mode) {
    case 0: {  // 目录文件
      inodes[new_inode].limitation = 6;
      inodes[new_inode].size = 2 * sizeof(dir);
      inodes[new_inode].type = DIRECTORY;
      open_file(new_inode);
      dir *bufp = (dir *)buf;
      strcpy(bufp[0].file_name, ".");
      bufp[0].idnode_num = new_inode;
      strcpy(bufp[1].file_name, "..");
      bufp[1].idnode_num = inode;
      close_file(new_inode);
      break;
    }
    case 1: {  // 普通文件
      inodes[new_inode].limitation = 7;
      inodes[new_inode].size = 0;
      inodes[new_inode].type = REGULAR_FILE;
      break;
    }
    default:
      break;
  }
}

void write_file(int inode, const char *name) {
  int file_inode = find_file(inode, name);
  open_file(file_inode);
  int size = 0;
  char c = getchar();
  while (c == ' ') {
    c = getchar();
  }
  if (c != '\"') {
    printf("please put the text in quotation marks");
    return;
  }
  while ((c = getchar()) != '\"' && size < MAX_FILE_SIZE) {
    buf[size++] = c;
  }
  inodes[file_inode].size = size;
  if (size == MAX_FILE_SIZE) printf("the text is too long !");
  close_file(file_inode);
}

int main() {
  print_welcom_data();
  fs_init();
  char command[32];
  char *command_list[] = {"ls", "pwd",    "cat",   "mkdir", "cd",
                          "rm", "mkfile", "write", "exit",  NULL};
  while (1) {
    printf("\n%s >", path);
    flush_buffer(command, 32);
    scanf("%s", command);
    int operand = -1;
    for (int i = 0; command_list[i] != NULL; i++) {
      if (strcmp(command, command_list[i]) == 0) {
        operand = i;
        break;
      }
    }
    switch (operand) {
      case 0: {  // ls
        list_dir(curr_dir_inode);
        break;
      }
      case 1: {  // pwd
        printf("%s", path);
        break;
      }
      case 2: {  // cat
        char file_name[128] = {0};
        scanf("%s", file_name);
        int file_inode = find_file(curr_dir_inode, file_name);
        print_file(file_inode);
        break;
      }
      case 3: {  // mkdir
        char file_name[128] = {0};
        scanf("%s", file_name);
        make_file(curr_dir_inode, file_name, 0);
        break;
      }
      case 4: {  // cd
        char file_name[128] = {0};
        scanf("%s", file_name);
        enter_dir(curr_dir_inode, file_name);
        break;
      }
      case 5: {  // rm
        char file_name[128] = {0};
        scanf("%s", file_name);
        remove_file(curr_dir_inode, file_name);
        break;
      }
      case 6: {  // mkfile
        char file_name[128] = {0};
        scanf("%s", file_name);
        make_file(curr_dir_inode, file_name, 1);
        break;
      }
      case 7: {  // write
        char file_name[128] = {0};
        scanf("%s", file_name);
        write_file(curr_dir_inode, file_name);
        break;
      }
      case 8: {  // exit
        fs_close();
        exit(1);
        break;
      }
      default:
        printf("command \"%s\" not found\n", command);
    }
  }
}
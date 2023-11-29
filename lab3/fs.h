#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 其他定义 */
// inode中地址指针数量
#define FILE_SIZE (2 * 1024 * 1024 + 44 * 1024)
#define BLK_PER_FILE 12
// 读写权限
#define READ 4
#define WRITE 2
#define EXECUTE 1

typedef struct {
  long uid;
  char password[56];
} user;

typedef struct {
  char sb_volume_name[16];  // 文件系统名
  int sb_disk_size;         // 磁盘大小
  int sb_block_size;        // 块大小
  int inode_unused;         // 未使用的inode数量
  int blk_unused;           // 未使用的data块数量
  user users[63];
} super_block;

typedef enum {
  UNKNOWN = 0,
  REGULAR_FILE,
  DIRECTORY,
  DEVICE,
  PIPE,
  SOCKET
} file_type;
typedef struct {
  int limitation;  // 读写权限 read|write|execute
  file_type type;  // 文件类型
  int size;        // 文件大小
  int block_addr[BLK_PER_FILE];  // 11个一级索引 1个二级索引 故一个文件最大为
                                 // 2MB+44KB, 该地址实际为数据块的偏移
  int uid;  // 用户id
  time_t create_time;
  time_t attach_time;
  time_t modify_time;
  // int blocks;
  char padding[40];
} inode;
// int a = sizeof(inode);
typedef struct {
  int inode_num;
  char file_name[28];
  file_type type;
} dir;

typedef struct {
  int inode_num;
  int offset;
  int used;
  int size;
  int limitation;
  // int uid;
  time_t attach_time;
  file_type type;
  time_t modify_time;
} openfile_table_entry;

void flush_buffer(char *buf, int size);

/* api version 2.0 */
int init_fs();
void close_fs();
int open_file(int inode);
int read_file(int fd, int size, char *buffer);
int write_file(int fd, int size, char *buffer);
int file_set_offset(int fd, int offset);
void close_file(int fd);
int find_file(const char *name);
int create_file(const char *name, int dir_inode);
int remove_file(int inode, int dir_inode);
int create_dir(const char *name, int dir_inode);
int remove_dir(int inode, int dir_inode);
int open_dir(int inode_num);
int read_dir(int fd, int count, dir *buffer);
void close_dir(int fd);
void get_path(char *path);
void set_path(const char *path);

/* ===================================================================================================================*/
/* commands */
void ls();
void create();
void mkdir();
void rmdir();
void delete();
void cd();
void open();
void close();
void read();
void write();
void cat();
void chmod();
void signin();
void login();
void logout();
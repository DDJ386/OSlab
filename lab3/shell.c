#include <signal.h>

#include "fs.h"

char *command_list[] = {"ls",     "pwd",   "cat",    "mkdir", "cd",
                        "rm",     "rmdir", "mkfile", "write", "exit",
                        "open",   "read",  "chmod",  "login", "signin",
                        "logout", "close", "help",   NULL};
char curr_path[256];

void kill_handler(int signum) { printf("press \"exit\" to exit\n"); }

void print_welcom_information() {
  printf(
      "==========================================\n"
      "welcom to my file system\n"
      "enter \"help\" to get more information\n"
      "==========================================\n");
}

void help() {
  printf(
      "========================================================================"
      "===============\n"
      "ls\tlist directory\n"
      "pwd\tprint current path\n"
      "cat\trint the file using the file name\n"
      "mkdir\tmake a directory\n"
      "cd\tstep into the directory\n"
      "rm\tremove the file (if want to remove a directory use \"rmdir\")\n"
      "rmdir\tremove the directrory\n"
      "mkfile\tmake a file\n"
      "open\topen a file\n"
      "write\tplease input the fd provided by \"open\" and input text with "
      "quotation marks\n"
      "read\tread the opening file with fd\n"
      "close\tremember to close the file after you open it\n"
      "chmod\tchange the mod of the file(rwx)\n"
      "login\tlog your fs account\n"
      "signin\tset up your account\n"
      "logout\tlog out your account(not exit the fs)\n"
      "exit\texit the fs\n"
      "========================================================================"
      "===============\n");
}

int main() {
  init_fs();
  signal(SIGINT, kill_handler);
  print_welcom_information();
  char command[32];
  while (1) {
    get_path(curr_path);
    printf("\n%s >", curr_path);
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
        ls();
        break;
      }
      case 1: {  // pwd
        printf("%s", curr_path);
        break;
      }
      case 2: {  // cat
        cat();
        break;
      }
      case 3: {  // mkdir
        mkdir();
        break;
      }
      case 4: {  // cd
        cd();
        break;
      }
      case 5: {  // rm
        delete ();
        break;
      }
      case 6: {  // rmdir
        rmdir();
        break;
      }
      case 7: {  // mkfile
        create();
        break;
      }
      case 8: {  // write
        write();
        break;
      }
      case 9: {  // exit
        close_fs();
        exit(1);
        break;
      }
      case 10: {
        open();
        break;
      }
      case 11: {
        read();
        break;
      }
      case 12: {
        chmod();
        break;
      }
      case 13: {
        login();
        break;
      }
      case 14: {
        signin();
        break;
      }
      case 15: {
        logout();
        break;
      }
      case 16: {
        close();
        break;
      }
      case 17: {
        help();
        break;
      }
      default:
        printf("command \"%s\" not found\n", command);
    }
  }
}
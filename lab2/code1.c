#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
pid_t pid1 = -1, pid2 = -1;
int flag = 0;

void kill_children() {
  kill(pid1, SIGSTKFLT);
  kill(pid2, SIGCHLD);
}

void alarm_handler() {
  printf("%d stop test\n", SIGALRM);
  kill_children();
}
void quit_handler() {
  printf("%d stop test\n", SIGQUIT);
  kill_children();
}
void int_handler() {
  printf("%d stop test\n", SIGINT);
  kill_children();
}

void child_handler() { printf("%d stop test\n", SIGCHLD); }

void stkfel_handler() { printf("%d stop test\n", SIGSTKFLT); }

int main() {
  // TODO: 五秒之后或接收到两个信号
  sigset_t mask, prev_mask, unmask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);
  sigaddset(&mask, SIGSTKFLT);
  sigaddset(&mask, SIGCHLD);
  sigaddset(&unmask, SIGSTKFLT);
  sigaddset(&unmask, SIGCHLD);

  while (pid1 == -1) pid1 = fork();
  if (pid1 > 0) {
    while (pid2 == -1) pid2 = fork();
    if (pid2 > 0) {
      // TODO: 父进程
      int status;
      signal(SIGALRM, alarm_handler);
      signal(SIGQUIT, quit_handler);
      signal(SIGINT, int_handler);
      alarm(5);
      pause();
      wait(NULL);
      wait(NULL);
      printf("\nParent process is killed!!\n");
    } else {
      // TODO: 子进程 2
      sigprocmask(SIG_BLOCK, &mask, &prev_mask);
      signal(SIGCHLD, child_handler);
      sigprocmask(SIG_UNBLOCK, &unmask, NULL);
      pause();
      printf("\nChild process2 is killed by parent!!\n");
      return 0;
    }
  } else {
    // TODO：子进程 1
    sigprocmask(SIG_BLOCK, &mask, &prev_mask);
    signal(SIGSTKFLT, stkfel_handler);
    sigprocmask(SIG_UNBLOCK, &unmask, NULL);
    pause();
    printf("\nChild process1 is killed by parent!!\n");
    return 0;
  }
  return 0;
}
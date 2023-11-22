#include <capstone/capstone.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

void print_adress(char *reg, const struct user_regs_struct *regs) {
  if (strcmp(reg, "ax") == 0) printf("%llx\n", regs->rax);
  if (strcmp(reg, "bx") == 0) printf("%llx\n", regs->rbx);
  if (strcmp(reg, "cx") == 0) printf("%llx\n", regs->rcx);
  if (strcmp(reg, "dx") == 0) printf("%llx\n", regs->rdx);
  if (strcmp(reg, "bp") == 0) printf("%llx\n", regs->rbp);
  if (strcmp(reg, "ip") == 0) printf("%llx\n", regs->rip);
  if (strcmp(reg, "sp") == 0) printf("%llx\n", regs->rsp);
  if (strcmp(reg, "15") == 0) printf("%llx\n", regs->r15);
  if (strcmp(reg, "14") == 0) printf("%llx\n", regs->r14);
  if (strcmp(reg, "13") == 0) printf("%llx\n", regs->r13);
  if (strcmp(reg, "12") == 0) printf("%llx\n", regs->r12);
  if (strcmp(reg, "11") == 0) printf("%llx\n", regs->r11);
  if (strcmp(reg, "10") == 0) printf("%llx\n", regs->r10);
  if (strcmp(reg, "9") == 0) printf("%llx\n", regs->r9);
  if (strcmp(reg, "8") == 0) printf("%llx\n", regs->r8);
}

void check_memory_access(cs_insn *insn, const struct user_regs_struct *regs) {
  if (strncmp(insn->mnemonic, "push", 4) == 0 ||
      strncmp(insn->mnemonic, "pop", 3) == 0) {
    printf("%llx\n", regs->rsp);
  }
  char *s = insn->op_str;
  for (; *s != '\0'; s++) {
    if (*s == '(') {
      s += 3;
      char reg[3] = {0};
      strncpy(reg, s, 2);
      print_adress(reg, regs);
    }
  }
}

int main() {
  pid_t pid;
  if ((pid = fork()) == 0) {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execl("./tracee", "tracee", NULL);
  }
  struct user_regs_struct regs;
  ptrace(PTRACE_ATTACH, pid, NULL, NULL);
  int val;
  uint64_t adress = 0x0;
  while (1) {
    csh handle;
    cs_insn *insn;
    size_t count;

    wait(&val);
    if (WIFEXITED(val)) {
      return 0;
    }

    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    unsigned long instruction = ptrace(PTRACE_PEEKTEXT, pid, regs.rip, NULL);
    uint8_t instruction_array[8] = {0};
    for (int i = 0; instruction; i++) {
      instruction_array[i] = (uint8_t)(instruction & 0xff);
      instruction = instruction >> 8;
    }

    printf("%llx\n", regs.rip);
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle)) {
      printf("ERROR: Failed to initialize engine!\n");
      return -1;
    }
    cs_option(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);

    count = cs_disasm(handle, instruction_array, sizeof(instruction_array), 0,
                      0, &insn);
    if (count) {
      // printf("%s\t\t%s\n", insn->mnemonic, insn->op_str);
      check_memory_access(insn, &regs);
    }

    ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
  }
}

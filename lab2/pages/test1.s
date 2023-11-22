	.file	"test1.c"
	.text
	.section	.rodata
.LC0:
	.string	"process%d access %lu success\n"
.LC1:
	.string	"process%d access %lu faild\n"
	.text
	.globl	rand_input
	.type	rand_input, @function
rand_input:
.LFB6:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	$0, %edi
	call	time@PLT
	movl	%eax, %edi
	call	srand@PLT
	movl	$0, -16(%rbp)
	jmp	.L2
.L5:
	call	rand@PLT
	cltd
	idivl	-20(%rbp)
	movl	%edx, -12(%rbp)
	call	rand@PLT
	movslq	%eax, %rcx
	movq	%rcx, %rax
	shrq	$12, %rax
	movabsq	$737869762948382228, %rdx
	mulq	%rdx
	movq	%rdx, %rax
	salq	$2, %rax
	addq	%rdx, %rax
	leaq	0(,%rax,4), %rdx
	addq	%rdx, %rax
	salq	$12, %rax
	subq	%rax, %rcx
	movq	%rcx, %rdx
	movq	%rdx, -8(%rbp)
	movq	-8(%rbp), %rdx
	movl	-12(%rbp), %eax
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	access@PLT
	testl	%eax, %eax
	je	.L3
	movq	-8(%rbp), %rdx
	movl	-12(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
	jmp	.L4
.L3:
	movq	-8(%rbp), %rdx
	movl	-12(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC1(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L4:
	addl	$1, -16(%rbp)
.L2:
	cmpl	$999999, -16(%rbp)
	jle	.L5
	nop
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	rand_input, .-rand_input
	.section	.rodata
	.align 8
.LC2:
	.string	"please input process and adress: "
.LC3:
	.string	"e.g 1 0xfffa01"
.LC4:
	.string	"%d %lu"
	.text
	.globl	man_input
	.type	man_input, @function
man_input:
.LFB7:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	%edi, -36(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	.LC2(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	leaq	.LC3(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
.L11:
	leaq	-16(%rbp), %rdx
	leaq	-20(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC4(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_scanf@PLT
	movl	-20(%rbp), %eax
	cmpl	$-1, %eax
	je	.L14
	movq	-16(%rbp), %rdx
	movl	-20(%rbp), %eax
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	access@PLT
	testl	%eax, %eax
	je	.L9
	movq	-16(%rbp), %rdx
	movl	-20(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
	jmp	.L11
.L9:
	movq	-16(%rbp), %rdx
	movl	-20(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC1(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
	jmp	.L11
.L14:
	nop
	movq	-8(%rbp), %rax
	subq	%fs:40, %rax
	je	.L12
	call	__stack_chk_fail@PLT
.L12:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	man_input, .-man_input
	.section	.rodata
.LC5:
	.string	"input mode:"
.LC6:
	.string	"1. manual"
.LC7:
	.string	"2. auto"
.LC8:
	.string	"%d"
	.text
	.globl	choise_input_mode
	.type	choise_input_mode, @function
choise_input_mode:
.LFB8:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	.LC5(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	leaq	.LC6(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	leaq	.LC7(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	leaq	-12(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC8(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_scanf@PLT
	movl	-12(%rbp), %eax
	movq	-8(%rbp), %rdx
	subq	%fs:40, %rdx
	je	.L17
	call	__stack_chk_fail@PLT
.L17:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	choise_input_mode, .-choise_input_mode
	.section	.rodata
	.align 8
.LC9:
	.string	"input the maximum number of processes:"
	.text
	.globl	set_max_processes
	.type	set_max_processes, @function
set_max_processes:
.LFB9:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	.LC9(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	leaq	-12(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC8(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_scanf@PLT
	movl	-12(%rbp), %eax
	movq	-8(%rbp), %rdx
	subq	%fs:40, %rdx
	je	.L20
	call	__stack_chk_fail@PLT
.L20:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	set_max_processes, .-set_max_processes
	.section	.rodata
.LC10:
	.string	"input the algorithm:"
.LC11:
	.string	"1. FIFO\n2. LRU"
	.text
	.globl	set_algo
	.type	set_algo, @function
set_algo:
.LFB10:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	.LC10(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	leaq	.LC11(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	leaq	-12(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC8(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_scanf@PLT
	movl	-12(%rbp), %eax
	movl	%eax, %edi
	call	set_algorithm@PLT
	nop
	movq	-8(%rbp), %rax
	subq	%fs:40, %rax
	je	.L22
	call	__stack_chk_fail@PLT
.L22:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	set_algo, .-set_algo
	.globl	main
	.type	main, @function
main:
.LFB11:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$0, %eax
	call	choise_input_mode
	movl	%eax, -8(%rbp)
	movl	$0, %eax
	call	set_max_processes
	movl	%eax, -4(%rbp)
	movl	$0, %eax
	call	set_algo
	cmpl	$1, -8(%rbp)
	je	.L24
	cmpl	$2, -8(%rbp)
	je	.L25
	jmp	.L26
.L24:
	movl	-4(%rbp), %eax
	movl	%eax, %edi
	call	man_input
	jmp	.L26
.L25:
	movl	-4(%rbp), %eax
	movl	%eax, %edi
	call	rand_input
.L26:
	movl	$0, %eax
	call	summarize@PLT
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:

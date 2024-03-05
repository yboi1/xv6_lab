	# qemu -kernel loads the kernel at 0x80000000
        # and causes each CPU to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.      栈的初始化代码
.section .text
_entry:
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
	csrr a1, mhartid 
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0
	# jump to start() in    start.c     row 21
        call start
spin:
        j spin

# csrr a1, mhartid：

# 这行代码使用 CSR（Control and Status Register）指令来读取当前硬件线程的 ID，并
# 将其存储到寄存器 a1 中。mhartid 是一个特权级 CSR，用于读取当前硬件线程的硬件线程 ID。
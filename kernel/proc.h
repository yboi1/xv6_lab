// Saved registers for kernel context switches.
struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null. 该CPU上运行的进程
  struct context context;     // swtch() here to enter scheduler(). 保存上下文消息用于调换
  int noff;                   // Depth of push_off() nesting. 内核中关闭中断，避免竞争
  int intena;                 // Were interrupts enabled before push_off()? 记录中断的状态
};

extern struct cpu cpus[NCPU];

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// the sscratch register points here.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {
  /*   0 */ uint64 kernel_satp;   // 内核页表的地址
  /*   8 */ uint64 kernel_sp;     // 进程的内核栈的顶部地址
  /*  16 */ uint64 kernel_trap;   // usertrap() 处理器陷入内核模式的位置， 用于用户态和内核态的过渡函数
  /*  24 */ uint64 epc;           // 保存用户程序的程序计数器
  /*  32 */ uint64 kernel_hartid; // 保存处理器的硬件线程ID
  /*  40 */ uint64 ra;            // 下面的寄存器为通用寄存器，用于保存处理器的上下文信息
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
};

enum procstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  struct spinlock lock;   // 自旋锁保证睡眠锁的正常进行

  // p->lock must be held when using these:
  enum procstate state;        // 进程的状态
  struct proc *parent;         // 父进程
  void *chan;                  // 如果不为零表示正在等一个特定的通道
  int killed;                  // 如果不为零， 表示已经被杀死
  int xstate;                  // 返回给父进程的等待状态
  int pid;                     // 进程ID

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // 内核栈虚拟地址，用于上下文切换
  uint64 sz;                   // 进程内存的大小（bytes）
  pagetable_t pagetable;       // 用户页表
  struct trapframe *trapframe; // 保存进程的中断帧
  struct context context;      // 用于上下文切换
  struct file *ofile[NOFILE];  // 记录已经打开的文件
  struct inode *cwd;           // 当前工作目录的指针
  char name[16];               // 进程的名称， debug使用
};

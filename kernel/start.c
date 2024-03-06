#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// scratch area for timer interrupt, one per CPU.
uint64 mscratch0[NCPU * 32];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// entry.S jumps here in machine mode on stack0.
void
start()
{
  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  w_mstatus(x);

  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany
  w_mepc((uint64)main);

  // disable paging for now.  
  w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  w_medeleg(0xffff);
  w_mideleg(0xffff);    // 外部中断（SEIE）、定时器中断（STIE）和软件中断（SSIE）
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

  // ask for clock interrupts.    对时钟芯片进行编程以产生计时器中断
  timerinit();

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);   // tp 为线程ID

  // switch to supervisor mode and jump to main().
  asm volatile("mret");
}   // To proc.c 212

// set up to receive timer interrupts in machine mode,
// which arrive at timervec in kernelvec.S,
// which turns them into software interrupts for
// devintr() in trap.c.   接受定时器中断
void
timerinit()
{
  // each CPU has a separate source of timer interrupts.
  int id = r_mhartid();

  // ask the CLINT for a timer interrupt.   
  int interval = 1000000; // cycles; about 1/10th second in qemu.   约相当于 qemu 中 1/10 秒的时间大
  *(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval;    // 针对CLINT编程， 在特定的延迟后生成中断

  // prepare information in scratch[] for timervec.   设置一个scratch区域， 用于保存寄存器现场和CLINT寄存器的地址
  // scratch[0..3] : space for timervec to save registers.
  // scratch[4] : address of CLINT MTIMECMP register.
  // scratch[5] : desired interval (in cycles) between timer interrupts.
  uint64 *scratch = &mscratch0[32 * id];
  scratch[4] = CLINT_MTIMECMP(id);    // 将 CLINT MTIMECMP 寄存器的地址和设置的时间间隔存储到寄存器保存区域中。
  scratch[5] = interval;
  w_mscratch((uint64)scratch);    // 存储到 CPU 的 mscratch 寄存器

  // set the machine-mode trap handler.   设置机器模式（Machine Mode）下的中断处理程序的入口地址
  w_mtvec((uint64)timervec);      // 使定时器可以中断

  // enable machine-mode interrupts.    MIE位
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE);
}

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
// RUNTIME_SBI defines the stack in entry_sbi.S for better control of location
#ifndef RUNTIME_SBI
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];
#endif

// entry.S jumps here in machine mode on stack0.
void
start()
{
#ifdef RUNTIME_SBI
  // skip all the machine mode initialization
  
  // dont need SEIE until we fool with the UART
  //w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
  w_sie(r_sie() | SIE_STIE | SIE_SSIE);
  // ask for clock interrupts.
  timerinit();
#else
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
  w_mideleg(0xffff);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

  // configure Physical Memory Protection to give supervisor mode
  // access to all of physical memory.
  w_pmpaddr0(0x3fffffffffffffull);
  w_pmpcfg0(0xf);

  // ask for clock interrupts.
  timerinit();

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);

  // switch to supervisor mode and jump to main().
  asm volatile("mret");
#endif
}

// ask each hart to generate timer interrupts.
void
timerinit()
{
#ifdef RUNTIME_SBI
  sbi_set_timer(r_time() + 1000000);
#else
  // enable supervisor-mode timer interrupts.
  w_mie(r_mie() | MIE_STIE);
  
  // enable the sstc extension (i.e. stimecmp).
  w_menvcfg(r_menvcfg() | (1L << 63)); 
  
  // allow supervisor to use stimecmp and time.
  w_mcounteren(r_mcounteren() | 2);
  
  // ask for the very first timer interrupt.
  w_stimecmp(r_time() + 1000000);
#endif
}

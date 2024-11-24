#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

#ifdef RUNTIME_SBI
int boothartid = -1;
int started = -1;
extern void *_entry_sbi;
#else
volatile static int started = 0;
#endif

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
#ifdef RUNTIME_SBI
  // uboot sends in one hart, its up to us to tell SBI to send in more
  if (started < 0) {
    // Only the uboot specified random (!) hart runs this code, so save its id
    // so we know which other cores to start for SMP.
    boothartid = r_tp();
#else
  if(cpuid() == 0){
#endif
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    kinit();         // physical page allocator
    kvminit();       // create kernel page table
    kvminithart();   // turn on paging
    procinit();      // process table
    trapinit();      // trap vectors
    trapinithart();  // install kernel trap vector
#ifndef RUNTIME_SBI
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
#endif
    binit();         // buffer cache
    iinit();         // inode table
    fileinit();      // file table
#ifndef RUNTIME_SBI
    virtio_disk_init(); // emulated hard disk
#endif
    userinit();      // first user process
    __sync_synchronize();
    // newly started harts must not come down same path as boot hart
    started = 1;
#ifdef RUNTIME_SBI
#ifdef BOARD_BPIF3
    // harts 0-7 (inclusive)
    for (int i = 0; i < NCPU; i++)
#else
    // harts 1-4 (inclusive)
    for (int i = 1; i <= NCPU; i++)
#endif
    {
      if (i != boothartid)
      {
        sbi_hart_start(i, (unsigned long) &_entry_sbi);
      }
    }
#endif
  } else {
#ifdef RUNTIME_SBI
   while (started < 0)
     ;
#else
    while(started == 0)
      ;
#endif
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
#ifndef RUNTIME_SBI
    plicinithart();   // ask PLIC for device interrupts
#endif
  }

  scheduler();        
}

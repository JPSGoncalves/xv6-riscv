#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

#ifdef RUNTIME_SBI
int boothartid = -1;
int started = -1;
extern void *_entry_hart1;
extern void *_entry_hart2;
extern void *_entry_hart3;
extern void *_entry_hart4;
extern void *_entry_mistake;
unsigned long harts_entry[NCPU];
#else
volatile static int started = 0;
#endif

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
#ifdef RUNTIME_SBI
    // uboot sends in one hart, its up to us to tell SBI to send in more
    if (started < 0)
    {
      // Only the uboot specified random (!) hart runs this code, so save its id
      // so we know which other cores to start for SMP. Range is 1-4 (not 0)
      boothartid = r_tp();
      // uboot identified its chosen hart in tp for us
      // But we need to uniquely init tp when starting harts via opensbi
      // no need to set up this array again after init
      harts_entry[0] = (unsigned long) &_entry_mistake;
      harts_entry[1] = (unsigned long) &_entry_hart1;
      harts_entry[2] = (unsigned long) &_entry_hart2;
      harts_entry[3] = (unsigned long) &_entry_hart3;
      harts_entry[4] = (unsigned long) &_entry_hart4;
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
    started = 1;
#ifdef RUNTIME_SBI
      for (int i = 1; i < NCPU; i++)
      {
        if (i != boothartid)
        {
          sbi_hart_start(i, harts_entry[i]);
        }
      }
#endif
  } else {
#ifdef RUNTIME_SBI
   // This technically shouldnt happen because of where we set 'started'
   // (other cores were penned back in opensbi before then)
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

#include "param.h"

        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.        

.section .text

.global _entry
.globl _entry_hart1
.globl _entry_hart2
.globl _entry_hart3
.globl _entry_hart4
#if defined(BOARD_BPIF3)
.globl _entry_hart5
.globl _entry_hart6
.globl _entry_hart7
.globl _entry_hart8
#endif
.globl _entry_mistake
.extern __bss_start
.extern __bss_end

_entry:
        # for openboot initiated launch, tp has the hart id in tp already
        # and in the general case can be any hart
        # 0 is reserved for siFive RT cores (unmatched, VF2) and will not enter here in tp
#if defined(BOARD_BPIF3)
        # processor 0 is usable on BPIF3 board (spacemit k1) and code expects tp from 1..NCPU for stack
        # we observe processor 0 always selected for this board but presumably can range from 0-7
        addi tp, tp, 1
#endif
        call clear_bss
        j entry_run
        # xv6/opensbi initiated harts do not self identify hart id so when we
        # start them in main() we direct their start address appropriately
        # logic there prevents a request to send in the same hart as uboot did
        # and already range from 1..max
_entry_hart1:
        li tp,1
        j entry_run
_entry_hart2:
        li tp,2
        j entry_run
_entry_hart3:
        li tp,3
        j entry_run
_entry_hart4:
        li tp,4
#if defined(BOARD_BPIF3)
        # BPI F3 has spacemit chip with 8 usable cores
        j entry_run
_entry_hart5:
        li tp,5
        j entry_run
_entry_hart6:
        li tp,6
        j entry_run
_entry_hart7:
        li tp,7
        j entry_run
_entry_hart8:
        li tp,8
#endif
entry_run:

        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
        # csrr a1, mhartid
        mv a1,tp
        # addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0
        # jump to start() in start.c

        call start
        call main
spin:
_entry_mistake:
        wfi
        j spin

clear_bss:
        la t0, __bss_start
        la t1, __bss_end

1:
        bge t0, t1, done
        sw zero, 0(t0)
        addi t0, t0, 4
        j 1b

done:
        ret

# the "calling" hart will be stopped and held in stop state within sbi
# this function does not return -- need to call sbi_hart_start(hard id)
park:
        li a6, 1
        lui a7, 0x485
        add a7, a7, 0x34d   # 48534d
        ecall
        # should not get here..
        j spin


.section .bss.stack
.align 12
.globl stack0
stack0:
    .space 4096 * 4 * (NCPU+1)
    .globl stack0_top
stack0_top:



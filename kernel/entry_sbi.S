#include "param.h"

        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.        

.section .text

.global _entry
.globl _entry_sbi

.globl _entry_mistake
.extern __bss_start
.extern __bss_end

_entry:
        # for uboot initiated launch, the hart id is in tp already
        # and is chosen randomly in in the general case

        # 0 is reserved for siFive RT cores (unmatched, VF2) and wont be selected
        # 0 is used on bpif3, however

        call clear_bss
        j entry_run

        # opensbi initiated harts identify their hart in a0
        # xv6 needs it in tp
_entry_sbi:
        mv tp,a0
entry_run:

        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
        # csrr a1, mhartid
        mv a1,tp
#if defined(BOARD_BPIF3)
        # processor 0 is usable on BPIF3 board (spacemit k1) so we need to adjust for the stack calculation
        # unmatched and SV2 enables 1..4 and uboot+main() leaves 0 inside SBI (since its not a RV64G processor)
        addi a1, a1, 1
#endif
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



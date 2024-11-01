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
.globl _entry_mistake
.extern __bss_start
.extern __bss_end

_entry:
        // uboot identifies the "chosen" hart in tp
        // not sure if thats a spec though. hart0 stays dormant in opensbi
        // (we can change that if we want as long as we have something
        //  productive to do outside of xv6, e.g. expand _entry_mistake.
        //  I'm thinking of some kind of raw memory peeker runing its own debugger.
        // this is because it does not have supervisor support
        // Only the 1st hart can clear bss
        // ideally this would happen on initial code load but it doesnt
        call clear_bss
        j entry_run
        // opensbi initiated harts do not self identify hart id so when we
        // start them in main() we direct their start address appropriately
        // logic there prevents a request to send in the same hart as uboot did
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
entry_run:
        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
        # csrr a1, mhartid
        # for unmatched openboot launch, tp has the hart id in tp already
        # but its range is 1-4 not 0-4. 0 is left in reserve since its an S7 core
        mv a1,tp
        # in effect, a1 has already been incremented by 1 due to range 1-4
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

# assumes NPROC = 5 (0 is not used on unreached but we reserve space anyway)
.section .bss.stack
.align 12
.globl stack0
stack0:
    .space 4096 * 4 * 5
    .globl stack0_top
stack0_top:



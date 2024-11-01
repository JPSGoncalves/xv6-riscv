/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2015 Regents of the University of California
 */

#ifndef _ASM_RISCV_SBI_H
#define _ASM_RISCV_SBI_H
#include "types.h"

#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2
#define SBI_CLEAR_IPI 3
#define SBI_SEND_IPI 4
#define SBI_REMOTE_FENCE_I 5
#define SBI_REMOTE_SFENCE_VMA 6
#define SBI_REMOTE_SFENCE_VMA_ASID 7
#define SBI_SHUTDOWN 8

#define SBI_CALL(which, arg0, arg1, arg2, arg3) ({        \
	register uintptr_t a0 asm("a0") = (uintptr_t)(arg0);  \
	register uintptr_t a1 asm("a1") = (uintptr_t)(arg1);  \
	register uintptr_t a2 asm("a2") = (uintptr_t)(arg2);  \
	register uintptr_t a3 asm("a3") = (uintptr_t)(arg3);  \
	register uintptr_t a7 asm("a7") = (uintptr_t)(which); \
	asm volatile("ecall"                                  \
				 : "+r"(a0)                               \
				 : "r"(a1), "r"(a2), "r"(a3), "r"(a7)     \
				 : "memory");                             \
	a0;                                                   \
})

/* Lazy implementations until SBI is finalized */
#define SBI_CALL_0(which) SBI_CALL(which, 0, 0, 0, 0)
#define SBI_CALL_1(which, arg0) SBI_CALL(which, arg0, 0, 0, 0)
#define SBI_CALL_2(which, arg0, arg1) SBI_CALL(which, arg0, arg1, 0, 0)
#define SBI_CALL_3(which, arg0, arg1, arg2) \
	SBI_CALL(which, arg0, arg1, arg2, 0)
#define SBI_CALL_4(which, arg0, arg1, arg2, arg3) \
	SBI_CALL(which, arg0, arg1, arg2, arg3)

void sbi_console_putchar(int ch)
{
	SBI_CALL_1(SBI_CONSOLE_PUTCHAR, ch);
}

int sbi_console_getchar(void)
{
	return SBI_CALL_0(SBI_CONSOLE_GETCHAR);
}

void sbi_set_timer(uint64 stime_value)
{
	SBI_CALL_1(SBI_SET_TIMER, stime_value);
}

void sbi_shutdown(void)
{
	SBI_CALL_0(SBI_SHUTDOWN);
}

void sbi_clear_ipi(void)
{
	SBI_CALL_0(SBI_CLEAR_IPI);
}

void sbi_send_ipi(const unsigned long *hart_mask)
{
	SBI_CALL_1(SBI_SEND_IPI, hart_mask);
}

void sbi_remote_fence_i(const unsigned long *hart_mask)
{
	SBI_CALL_1(SBI_REMOTE_FENCE_I, hart_mask);
}

long sbi_remote_sfence_vma(const unsigned long *hart_mask,
						   unsigned long start,
						   unsigned long size)
{
	long ret;
	SBI_CALL_3(SBI_REMOTE_SFENCE_VMA, hart_mask, start, size);
	asm volatile("mv %0, a0" : "=r"(ret));
	return ret;
}

void sbi_remote_sfence_vma_asid(const unsigned long *hart_mask,
								unsigned long start,
								unsigned long size,
								unsigned long asid)
{
	SBI_CALL_4(SBI_REMOTE_SFENCE_VMA_ASID, hart_mask, start, size, asid);
}

void sbi_set_extern_interrupt(unsigned long func_pointer)
{
	asm volatile("mv a6, %0" : : "r"(0x210));
	SBI_CALL_1(0x0A000004, func_pointer);
}

void sbi_set_mie(void)
{
	SBI_CALL_0(0x0A000005);
}

void sbi_hart_start(unsigned long hart, unsigned long addr)
{
	asm volatile("mv a6, %0" : : "r"(0x0));
	SBI_CALL_3(0x48534D, hart, addr, 0x4711);
}

unsigned long sbi_hart_get_status(unsigned long hart)
{
	unsigned long ret;
	asm volatile("mv a6, %0" : : "r"(0x1));
	SBI_CALL_1(0x48534D, hart);
	asm volatile("mv %0, a0" : "=r"(ret));
	return ret;
}

#endif

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	plat_qti_cpu_hold_park

	/* -----------------------------------------------------------------
	 * void plat_qti_cpu_hold_park(uintptr_t go, uintptr_t stack_base,
	 *			       size_t stack_size, uintptr_t cpu_data,
	 *			       size_t cpu_data_size)
	 *
	 * Park this core with the MMU and data cache off until *go is set,
	 * then enter the warm boot path, which runs with them off until it
	 * enables the MMU. The core stays powered and coherent, and its caches
	 * keep what it wrote with them on, so first write back and invalidate
	 * what is accessed with them off: the stack and per-CPU data of the
	 * warm boot path, and go. That is done by address, to the point of
	 * coherency, and with the data cache already off so that nothing is
	 * allocated again, and with no memory writes in between.
	 * -----------------------------------------------------------------
	 */
func plat_qti_cpu_hold_park
	mov	x19, x0
	mov	x20, x1
	mov	x21, x2
	mov	x22, x3
	mov	x23, x4

	mov_imm	x1, (SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	mrs	x0, sctlr_el3
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb

	mov	x0, x20
	mov	x1, x21
	bl	flush_dcache_range
	mov	x0, x22
	mov	x1, x23
	bl	flush_dcache_range
	mov	x0, x19
	mov	x1, #8
	bl	flush_dcache_range

1:	ldr	x0, [x19]
	cbnz	x0, 2f
	wfe
	b	1b
2:
	b	bl31_warm_entrypoint
endfunc plat_qti_cpu_hold_park

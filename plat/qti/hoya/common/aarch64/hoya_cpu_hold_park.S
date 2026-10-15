/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	plat_qti_cpu_hold_park

	/* -----------------------------------------------------------------
	 * void plat_qti_cpu_hold_park(uintptr_t go)
	 *
	 * Park this core with the MMU and data cache off until *go is set,
	 * then enter the warm boot path. The core stays powered and coherent.
	 * go is polled with the data cache off, so its line, which this core
	 * wrote with the cache on, is first written back and invalidated by
	 * address to the point of coherency, once the cache is off so that it
	 * is not allocated again.
	 * -----------------------------------------------------------------
	 */
func plat_qti_cpu_hold_park
	mov	x19, x0

	mov_imm	x1, (SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	mrs	x0, sctlr_el3
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb

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

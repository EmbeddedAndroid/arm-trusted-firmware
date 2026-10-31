/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

/* Cortex-A520/A720 CPUPWRCTLR_EL1 */
#define CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CPUPWRCTLR_EL1_CORE_PWRDN_BIT	(1 << 0)

	.globl	plat_qti_cpu_hold_park
	.globl	plat_qti_cpu_hold_wait

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

	/* -----------------------------------------------------------------
	 * void plat_qti_cpu_hold_wait(void)
	 *
	 * Hold this suspended core in WFI with the MMU and data cache off,
	 * then enter the warm boot path on its first wake-up event. The core
	 * stays powered and coherent. The CPU power-down hook set
	 * CPUPWRCTLR_EL1.CORE_PWRDN_EN, which turns that WFI into a power-down
	 * request nothing completes: clear it first. The non-secure interrupts
	 * are routed to EL3 so that they end the WFI; they stay masked here and
	 * the warm boot path restores the routing. Unlike after a power-down,
	 * the caches still hold valid lines when the warm boot path runs with
	 * the MMU off, so that path must not store to memory before it turns
	 * the MMU on.
	 * -----------------------------------------------------------------
	 */
func plat_qti_cpu_hold_wait
	mrs	x0, CPUPWRCTLR_EL1
	bic	x0, x0, #CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CPUPWRCTLR_EL1, x0

	mrs	x0, scr_el3
	orr	x0, x0, #(SCR_IRQ_BIT | SCR_FIQ_BIT)
	msr	scr_el3, x0
	isb

	mov_imm	x1, (SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	mrs	x0, sctlr_el3
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb

	dsb	sy
	wfi
	b	bl31_warm_entrypoint
endfunc plat_qti_cpu_hold_wait

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cortex_a520.h>
#include <cortex_a720.h>

/*
 * The hold skips the assembly part of bl31_warm_entrypoint, which records
 * the runtime instrumentation timestamps.
 */
#if ENABLE_RUNTIME_INSTRUMENTATION
#error "eliza_cpu_hold_wait does not support ENABLE_RUNTIME_INSTRUMENTATION"
#endif

#if (CORTEX_A520_CPUPWRCTLR_EL1_CORE_PWRDN_BIT != \
	CORTEX_A720_CPUPWRCTLR_EL1_CORE_PWRDN_BIT)
#error "Cortex-A520 and Cortex-A720 CPUPWRCTLR_EL1 layouts differ"
#endif

	.globl	eliza_cpu_hold_wait

	/* -----------------------------------------------------------------
	 * void eliza_cpu_hold_wait(void)
	 *
	 * Hold a core suspended to a power-down state in WFI and resume it on
	 * its first wake-up event as the warm boot path does once the MMU is
	 * on: the core stays powered, coherent and with the MMU and caches on,
	 * so nothing before that point of the warm boot path is needed. The
	 * CPU power-down hook set CORE_PWRDN_EN, which would turn the WFI into
	 * a power-down request: clear it. Non-secure interrupts are routed to
	 * EL3 so that they end the WFI; they stay masked here, and the exit to
	 * the normal world restores the routing from its saved context.
	 * -----------------------------------------------------------------
	 */
func eliza_cpu_hold_wait
	mrs	x0, CORTEX_A720_CPUPWRCTLR_EL1
	bic	x0, x0, #CORTEX_A720_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_A720_CPUPWRCTLR_EL1, x0

	mrs	x0, scr_el3
	orr	x0, x0, #(SCR_IRQ_BIT | SCR_FIQ_BIT)
	msr	scr_el3, x0
	isb

	dsb	sy
	wfi

	bl	plat_set_my_stack
	bl	bl31_warmboot
	b	el3_exit
endfunc eliza_cpu_hold_wait

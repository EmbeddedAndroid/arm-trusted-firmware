/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * With neither CPUCP nor the PCU sequences running, nothing completes the
 * P-channel power-down request a core raises at CPU_OFF, and powering the
 * core up again over that pending request hangs the SoC. A core turned off is
 * instead parked here, powered and with no P-channel request, until CPU_ON
 * releases it to the warm boot entrypoint.
 */
#define CPU_HOLD_NONE		0U
#define CPU_HOLD_WAIT		1U
#define CPU_HOLD_GO		2U

/* CPUPWRCTLR_EL1 of the Cortex-A55 and Cortex-A76 based Kryo 4xx cores. */
#define CPUPWRCTLR_EL1_CORE_PWRDN_EN	BIT_64(0)

struct cpu_hold {
	uint64_t state;
} __aligned(CACHE_WRITEBACK_GRANULE);

static struct cpu_hold cpu_hold[PLATFORM_CORE_COUNT];

void __dead2 plat_qti_pwr_domain_pwr_down(const psci_power_state_t *target_state)
{
	struct cpu_hold *hold = &cpu_hold[plat_my_core_pos()];
	void (*warm_entry)(void) = bl31_warm_entrypoint;
	uint64_t val;

	(void)target_state;

	/* Withdraw the power-down request the CPU_OFF sequence armed. */
	__asm__ volatile("mrs %0, S3_0_C15_C2_7" : "=r" (val));
	val &= ~CPUPWRCTLR_EL1_CORE_PWRDN_EN;
	__asm__ volatile("msr S3_0_C15_C2_7, %0" : : "r" (val));
	isb();

	hold->state = CPU_HOLD_WAIT;
	flush_dcache_range((uintptr_t)hold, sizeof(*hold));

	/* Leave coherency: the pen is polled with the MMU and caches off. */
	dcsw_op_louis(DCCISW);
	disable_mmu_icache_el3();

	while (mmio_read_64((uintptr_t)&hold->state) != CPU_HOLD_GO) {
		wfe();
	}
	mmio_write_64((uintptr_t)&hold->state, CPU_HOLD_NONE);
	dsbsy();

	warm_entry();
	panic();
}

/* Release a core parked by plat_qti_pwr_domain_pwr_down(), if it is. */
bool plat_qti_cpu_hold_release(int core_pos)
{
	struct cpu_hold *hold = &cpu_hold[core_pos];

	inv_dcache_range((uintptr_t)hold, sizeof(*hold));
	if (hold->state != CPU_HOLD_WAIT) {
		return false;
	}

	hold->state = CPU_HOLD_GO;
	flush_dcache_range((uintptr_t)hold, sizeof(*hold));
	dsbsy();
	sev();

	return true;
}

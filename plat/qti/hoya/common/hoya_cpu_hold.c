/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * On SoCs where nothing completes the P-channel power-down request a core
 * raises at CPU_OFF, powering the core up again over that pending request
 * hangs the SoC. A core turned off is instead parked here, powered and with
 * no power-down request, until CPU_ON releases it to the warm boot
 * entrypoint. Parked cores stay powered, so no power level above the core
 * may be powered down while any core is parked.
 *
 * A core suspended to a power-down state is held the same way, in WFI, and
 * resumes through the warm boot entrypoint on its first wake-up event, as
 * after a power-down: the OS restores its context as it would there.
 *
 * Each field has one writer at a time:
 * - retiring is written by the core itself, set in its pwr_domain_off()
 *   and cleared on a suspend. It tells CPU_ON a released core will reach
 *   the pen, as opposed to a core that has never run, which needs a cold
 *   power-up.
 * - go is cleared by the core in pwr_domain_off() and set by CPU_ON. The
 *   parked core only reads it, so a release that comes before the core
 *   reaches the pen is kept.
 * Both writes of the core happen before PSCI publishes AFF_STATE_OFF, and
 * CPU_ON only reaches the platform for a core it has seen OFF.
 */
struct cpu_hold {
	uint64_t retiring;
	uint64_t go;
} __aligned(CACHE_WRITEBACK_GRANULE);

static struct cpu_hold cpu_hold[PLATFORM_CORE_COUNT];

void __dead2 plat_qti_cpu_hold_park(uintptr_t go);
void __dead2 plat_qti_cpu_hold_wait(void);

bool plat_qti_cpu_hold_retire(unsigned int core_pos)
{
	struct cpu_hold *hold = &cpu_hold[core_pos];

	hold->go = 0U;
	hold->retiring = 1U;
	/* Visible before PSCI marks this core OFF. */
	dsbish();

	return true;
}

bool plat_qti_cpu_hold_suspend(unsigned int core_pos)
{
	cpu_hold[core_pos].retiring = 0U;

	return true;
}

void plat_qti_pwr_domain_pwr_down(const psci_power_state_t *target_state)
{
	struct cpu_hold *hold = &cpu_hold[plat_my_core_pos()];

	(void)target_state;

	/*
	 * PSCI calls this only for CPU_OFF, which sets retiring, and for a
	 * suspend to a power-down state, which clears it.
	 */
	if (hold->retiring == 0U) {
		plat_qti_cpu_hold_wait();
	}

	plat_qti_cpu_hold_park((uintptr_t)&hold->go);
}

bool plat_qti_cpu_hold_release(unsigned int core_pos)
{
	struct cpu_hold *hold = &cpu_hold[core_pos];

	/* Read retiring after the AFF_STATE_OFF the caller has seen. */
	dmbish();
	if (hold->retiring == 0U) {
		return false;
	}

	/* The parked core reads go with its caches off. */
	hold->go = 1U;
	flush_dcache_range((uintptr_t)&hold->go, sizeof(hold->go));
	sev();

	return true;
}

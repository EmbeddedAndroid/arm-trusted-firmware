/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/cpucp/cpucp.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_cpu_pwr.h>
#include <qti_plat.h>

/*
 * SM7750 core power: one DynamIQ cluster, the Cortex-A520 cores 0 and 1 in
 * the complex the boot firmware powers with core 0, core 2 alone in a second
 * complex, and the Cortex-A720 cores 3 to 7 on the APC1 rail, which is off at
 * cold boot. Each core has an IPM block (APSS_ALIAS_n) and a core PPU on the
 * DSU utility bus.
 */
#define CPU_IPM_BASE(core)		(QTI_APSS_HM_BASE + 0x800000U + \
					 ((core) * 0x10000U))
#define COMPLEX1_IPM_BASE		(QTI_APSS_HM_BASE + 0x890000U)
#define COMPLEX1_CORE			2U

#define IPM_HEAD_SWITCH_PWRDUP		BIT_32(10)

#define CPU_PPU_BASE(core)		(QTI_APSS_HM_BASE + 0xe80000U + \
					 ((core) * 0x100000U))
#define PPU_PWPR(core)			(CPU_PPU_BASE(core) + 0x0U)
#define PPU_PWSR(core)			(CPU_PPU_BASE(core) + 0x8U)

#define PPU_PWPR_PWR_POLICY_MASK	GENMASK_32(3, 0)
#define PPU_PWPR_PWR_POLICY_ON		0x8U
#define PPU_PWPR_PWR_DYN_EN		BIT_32(8)
#define PPU_PWSR_PWR_DYN_STATUS		BIT_32(8)

#define PPU_TIMEOUT_US			1000U

/* APC1 rail: SAW4, boot voltage and the memory repair of its rail sequencer. */
#define CL_IPM_BASE			(QTI_APSS_HM_BASE + 0x8b0000U)
#define APC1_SAW4_BASE			(QTI_APSS_HM_BASE + 0x901000U)
#define APC1_FIRST_CORE			3U
#define APC1_BOOT_MV			824U
#define APC1_REPAIR_EN			(CL_IPM_BASE + 0x180U)
#define APC1_REPAIR_VAL			(CL_IPM_BASE + 0x184U)
#define APC1_REPAIR_STS			(CL_IPM_BASE + 0x190U)
#define APC1_REPAIR_BIT			BIT_32(8)
#define APC1_REPAIR_DONE		BIT_32(4)

#define MEM_REPAIR_TIMEOUT_US		10000U

static bool apc1_repair_memories(void)
{
	uint64_t timeout;
	bool done = true;

	mmio_setbits_32(APC1_REPAIR_EN, APC1_REPAIR_BIT);
	mmio_setbits_32(APC1_REPAIR_VAL, APC1_REPAIR_BIT);

	timeout = timeout_init_us(MEM_REPAIR_TIMEOUT_US);
	while ((mmio_read_32(APC1_REPAIR_STS) & APC1_REPAIR_DONE) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("APC1: memory repair timed out\n");
			done = false;
			break;
		}
	}

	mmio_clrbits_32(APC1_REPAIR_VAL, APC1_REPAIR_BIT);
	mmio_clrbits_32(APC1_REPAIR_EN, APC1_REPAIR_BIT);

	return done;
}

/*
 * Bring up the APC1 rail before its first core: false if that fails, then
 * and for every later core, without another attempt.
 */
static bool apc1_power_on(void)
{
	static spinlock_t apc1_lock;
	static bool apc1_on;
	static bool apc1_failed;
	bool ok;

	spin_lock(&apc1_lock);
	if (!apc1_on && !apc1_failed) {
		apc1_on = qti_saw4_rail_on(APC1_SAW4_BASE, APC1_BOOT_MV) &&
			  apc1_repair_memories();
		apc1_failed = !apc1_on;
	}
	ok = apc1_on;
	spin_unlock(&apc1_lock);

	return ok;
}

/* Power up the complex of core 2 if the boot firmware left it off. */
static void complex1_power_on(void)
{
	static spinlock_t complex1_lock;

	spin_lock(&complex1_lock);
	if ((mmio_read_32(COMPLEX1_IPM_BASE + QTI_IPM_HEAD_SWITCH_CTL) &
	     IPM_HEAD_SWITCH_PWRDUP) == 0U) {
		qti_ipm_power_up(COMPLEX1_IPM_BASE, false);
	}
	spin_unlock(&complex1_lock);
}

/*
 * The core PPU must allow the core to power up: enable dynamic transitions
 * with ON as the minimum power state.
 */
static bool cpu_ppu_on(unsigned int core)
{
	uint64_t timeout;

	mmio_setbits_32(PPU_PWPR(core), PPU_PWPR_PWR_DYN_EN);

	timeout = timeout_init_us(PPU_TIMEOUT_US);
	while ((mmio_read_32(PPU_PWSR(core)) & PPU_PWSR_PWR_DYN_STATUS) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("CPU%u: PPU dynamic mode timed out\n", core);
			return false;
		}
	}

	mmio_clrsetbits_32(PPU_PWPR(core), PPU_PWPR_PWR_POLICY_MASK,
			   PPU_PWPR_PWR_POLICY_ON);

	return true;
}

static bool cpu_turned_off[PLATFORM_CORE_COUNT];

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	qti_cpucp_init();

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	unsigned int core = (unsigned int)core_pos;

	(void)mpidr;

	/*
	 * No core power-collapse sequence is set up, so a core turned off is
	 * left in WFI with its power-down request pending; powering it up over
	 * that request is not safe.
	 */
	if (cpu_turned_off[core]) {
		ERROR("CPU%u: CPU_ON after CPU_OFF is not supported\n", core);
		return PSCI_E_INTERN_FAIL;
	}

	if ((core >= APC1_FIRST_CORE) && !apc1_power_on()) {
		return PSCI_E_INTERN_FAIL;
	}

	if (core == COMPLEX1_CORE) {
		complex1_power_on();
	}

	qti_ipm_power_up(CPU_IPM_BASE(core), false);

	return cpu_ppu_on(core) ? PSCI_E_SUCCESS : PSCI_E_INTERN_FAIL;
}

void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
	cpucp_clkdom_init();
}

void plat_qti_pwr_domain_off(const uint8_t *states)
{
	(void)states;

	cpu_turned_off[plat_my_core_pos()] = true;
}

void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	(void)states;
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	(void)states;
}

/* Only CPU standby (WFI) is offered as an idle state. */
const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		0,
	};

	return idle_states;
}

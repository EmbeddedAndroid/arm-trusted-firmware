/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <hoya_cpu_pwr.h>
#include <hoya_dsu_cfg.h>
#include <platform_def.h>
#include <qti_plat.h>

/* Per-core IPM blocks (APSS_ALIAS_n); complex IPMs have the same layout. */
#define CPU_IPM_BASE(core)		(QTI_APSS_HM_BASE + 0x800000U + \
					 ((core) * 0x10000U))

#define IPM_HEAD_SWITCH_PWRDUP		BIT_32(10)

/* Core PPUs on the DSU utility bus. */
#define CPU_PPU_BASE(core)		(QTI_APSS_HM_BASE + 0xe80000U + \
					 ((core) * 0x100000U))
#define PPU_PWPR(core)			(CPU_PPU_BASE(core) + 0x0U)
#define PPU_PWSR(core)			(CPU_PPU_BASE(core) + 0x8U)

#define PPU_PWPR_PWR_POLICY_MASK	GENMASK_32(3, 0)
#define PPU_PWPR_PWR_POLICY_ON		0x8U
#define PPU_PWPR_PWR_DYN_EN		BIT_32(8)
#define PPU_PWSR_PWR_DYN_STATUS		BIT_32(8)

#define PPU_TIMEOUT_US			1000U

#define MEM_REPAIR_TIMEOUT_US		10000U

static spinlock_t gold_rail_lock;

static bool gold_rail_repair_memories(const struct hoya_gold_rail *rail)
{
	uint64_t timeout;
	bool done = true;

	mmio_setbits_32(rail->repair_en, rail->repair_bit);
	mmio_setbits_32(rail->repair_val, rail->repair_bit);

	timeout = timeout_init_us(MEM_REPAIR_TIMEOUT_US);
	while ((mmio_read_32(rail->repair_sts) & rail->repair_done) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("SAW4 0x%lx: memory repair timed out\n",
			      rail->saw4);
			done = false;
			break;
		}
	}

	mmio_clrbits_32(rail->repair_val, rail->repair_bit);
	mmio_clrbits_32(rail->repair_en, rail->repair_bit);

	return done;
}

/* Bring up the rail of @core if it is off: false if that fails. */
static bool gold_rail_power_on(unsigned int core)
{
	bool ok = true;

	spin_lock(&gold_rail_lock);

	for (unsigned int i = 0U; i < ARRAY_SIZE(hoya_gold_rails); i++) {
		struct hoya_gold_rail *rail = &hoya_gold_rails[i];

		if ((core < rail->first_core) || (core > rail->last_core) ||
		    rail->on) {
			continue;
		}

		ok = hoya_saw4_rail_on(rail->saw4, HOYA_GOLD_RAIL_BOOT_MV) &&
		     gold_rail_repair_memories(rail);
		rail->on = ok;
	}

	spin_unlock(&gold_rail_lock);

	return ok;
}

/*
 * The core PPU must allow the core to power up: enable dynamic transitions
 * with ON as the minimum power state.
 */
static void cpu_ppu_on(unsigned int core)
{
	uint64_t timeout;

	mmio_setbits_32(PPU_PWPR(core), PPU_PWPR_PWR_DYN_EN);

	timeout = timeout_init_us(PPU_TIMEOUT_US);
	while ((mmio_read_32(PPU_PWSR(core)) & PPU_PWSR_PWR_DYN_STATUS) == 0U) {
		if (timeout_elapsed(timeout)) {
			WARN("CPU%u: PPU dynamic mode timed out\n", core);
			break;
		}
	}

	mmio_clrsetbits_32(PPU_PWPR(core), PPU_PWPR_PWR_POLICY_MASK,
			   PPU_PWPR_PWR_POLICY_ON);
}

static spinlock_t complex_lock;

static bool cpu_turned_off[PLATFORM_CORE_COUNT];

/* Power up the complex of @core if the boot firmware left it off. */
static void complex_power_on(unsigned int core)
{
	if (ARRAY_SIZE(hoya_complexes) == 0U) {
		return;
	}

	spin_lock(&complex_lock);

	for (unsigned int i = 0U; i < ARRAY_SIZE(hoya_complexes); i++) {
		const struct hoya_complex *cplx = &hoya_complexes[i];

		if ((core >= cplx->first_core) && (core <= cplx->last_core) &&
		    ((mmio_read_32(cplx->ipm + HOYA_IPM_HEAD_SWITCH_CTL) &
		      IPM_HEAD_SWITCH_PWRDUP) == 0U)) {
			hoya_ipm_power_up(cplx->ipm, false);
		}
	}

	spin_unlock(&complex_lock);
}

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	return PSCI_E_SUCCESS;
}

/*
 * Cold boot of a secondary core: the first core on a gold rail also brings
 * the rail up, and a silver core whose complex is off powers it first.
 */
int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	unsigned int core = (unsigned int)core_pos;

	(void)mpidr;

	/*
	 * No core power-collapse sequence is ported, so a core turned off is
	 * left in WFI with its power-down request pending; powering it up over
	 * that request is not safe.
	 */
	if (cpu_turned_off[core]) {
		ERROR("CPU%u: CPU_ON after CPU_OFF is not supported\n", core);
		return PSCI_E_INTERN_FAIL;
	}

	if (!gold_rail_power_on(core)) {
		return PSCI_E_INTERN_FAIL;
	}

	complex_power_on(core);

	hoya_ipm_power_up(CPU_IPM_BASE(core), false);

	cpu_ppu_on(core);

	return PSCI_E_SUCCESS;
}

void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
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

void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)id;
	(void)handle;
}

/* Only CPU standby (WFI) is offered: no power-collapse sequence is ported. */
const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		0,
	};

	return idle_states;
}

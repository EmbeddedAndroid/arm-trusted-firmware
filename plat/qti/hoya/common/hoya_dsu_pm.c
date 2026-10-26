/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/watchdog/watchdog.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>

#include <hoya_dsu_cfg.h>
#include <platform_def.h>
#include <qti_plat.h>

/* Per-core IPM blocks (APSS_ALIAS_n); complex IPMs have the same layout. */
#define CPU_IPM_BASE(core)		(QTI_APSS_HM_BASE + 0x800000U + \
					 ((core) * 0x10000U))
#define IPM_HEAD_SWITCH_CTL		0x08U
#define IPM_SEQ_FORCE_PWR_CTL_EN	0x1cU
#define IPM_SEQ_FORCE_PWR_CTL_VAL	0x20U

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

/* SAW4 rail controller registers, from the SAW4 base. */
#define SAW4_VCTL			0x900U
#define SAW4_AVS_CTL			0x904U
#define SAW4_PMIC_STS			0xc18U

#define SAW4_AVS_CTL_EN			BIT_32(0)
#define SAW4_AVS_CTL_CTL_SEL		BIT_32(4)

#define SAW4_PMIC_STS_STATE_MASK	GENMASK_32(17, 16)
#define SAW4_PMIC_STS_CURR_DATA_MASK	GENMASK_32(15, 0)

/*
 * SAW4_VCTL holds SIZE[20], ADR_IDX[18:16] and PMIC_DATA[15:0]: the rail
 * enable is an 8-bit write of 0x80 to address index 3, the voltage a 16-bit
 * write of the millivolts to address index 0.
 */
#define SAW4_VCTL_ENABLE		0x30080U
#define SAW4_VCTL_SET_MV(mv)		(BIT_32(20) | (mv))

#define SAW4_PMIC_WRITE_RETRY		200U

#define MEM_REPAIR_TIMEOUT_US		10000U

static spinlock_t gold_rail_lock;

static bool saw4_pmic_wait(uintptr_t saw4, uint32_t data, bool check_data)
{
	for (unsigned int i = 0U; i < SAW4_PMIC_WRITE_RETRY; i++) {
		uint32_t sts = mmio_read_32(saw4 + SAW4_PMIC_STS);

		if (((sts & SAW4_PMIC_STS_STATE_MASK) == 0U) &&
		    (!check_data ||
		     ((sts & SAW4_PMIC_STS_CURR_DATA_MASK) == data))) {
			return true;
		}

		udelay(2);
	}

	return false;
}

static bool gold_rail_enable(const struct hoya_gold_rail *rail)
{
	mmio_setbits_32(rail->saw4 + SAW4_AVS_CTL, SAW4_AVS_CTL_EN);
	mmio_setbits_32(rail->saw4 + SAW4_AVS_CTL, SAW4_AVS_CTL_CTL_SEL);

	mmio_write_32(rail->saw4 + SAW4_VCTL, SAW4_VCTL_ENABLE);
	if (!saw4_pmic_wait(rail->saw4, 0U, false)) {
		ERROR("SAW4 0x%lx: rail enable timed out\n", rail->saw4);
		return false;
	}
	udelay(90);

	mmio_write_32(rail->saw4 + SAW4_VCTL,
		      SAW4_VCTL_SET_MV(HOYA_GOLD_RAIL_BOOT_MV));
	if (!saw4_pmic_wait(rail->saw4, HOYA_GOLD_RAIL_BOOT_MV, true)) {
		ERROR("SAW4 0x%lx: rail voltage timed out\n", rail->saw4);
		return false;
	}

	return true;
}

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

		ok = gold_rail_enable(rail) && gold_rail_repair_memories(rail);
		rail->on = ok;
	}

	spin_unlock(&gold_rail_lock);

	return ok;
}

/*
 * The core PPU must allow the core to power up: enable dynamic transitions
 * with ON as the minimum power state, which also keeps a core parked in BL31
 * at CPU_OFF powered.
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

/* The IPM power-up sequence of a core or a complex. */
static void ipm_power_up(uintptr_t ipm)
{
	/* Skew between the head switch enables, 40 XO cycles. */
	mmio_write_32(ipm + IPM_HEAD_SWITCH_CTL, 0x28U);
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_EN, 0x0U);

	/* Close the head switch, then drop the memory and logic clamps. */
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x642U);
	udelay(2);
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x402U);

	/* Wake the memories: slp_nret_n, then slp_ret_n. */
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x40aU);
	udelay(4);
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x40eU);
	udelay(4);

	/* Pulse wl_en_clk. */
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x50eU);
	udelay(2);
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x40eU);

	/* Ungate the clock, release reset and mark the block active. */
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x40cU);
	udelay(2);
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x43cU);
	mmio_write_32(ipm + IPM_SEQ_FORCE_PWR_CTL_VAL, 0x443cU);

	/* Assert PWRDUP. */
	mmio_write_32(ipm + IPM_HEAD_SWITCH_CTL, 0x428U);
}

static spinlock_t complex_lock;

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
		    ((mmio_read_32(cplx->ipm + IPM_HEAD_SWITCH_CTL) &
		      IPM_HEAD_SWITCH_PWRDUP) == 0U)) {
			ipm_power_up(cplx->ipm);
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

	if (plat_qti_cpu_hold_release(core_pos)) {
		return PSCI_E_SUCCESS;
	}

	if (!gold_rail_power_on(core)) {
		return PSCI_E_INTERN_FAIL;
	}

	complex_power_on(core);

	ipm_power_up(CPU_IPM_BASE(core));

	cpu_ppu_on(core);

	return PSCI_E_SUCCESS;
}

void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
	qti_watchdog_set_target(QTI_WATCHDOG_CPU_WAKEUP);
}

/* Hand the watchdog bark to an online core before this one is parked. */
void plat_qti_pwr_domain_off(const uint8_t *states)
{
	(void)states;

	qti_watchdog_set_target(QTI_WATCHDOG_CPU_HOTPLUG);
}

void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	(void)states;
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	(void)states;
}

void plat_qti_cpu_pwrdn_request(void)
{
	qti_cpu_pwrdn_en();
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

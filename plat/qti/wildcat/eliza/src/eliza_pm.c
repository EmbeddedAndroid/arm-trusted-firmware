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
#define COMPLEX0_IPM_BASE		(QTI_APSS_HM_BASE + 0x880000U)
#define COMPLEX1_IPM_BASE		(QTI_APSS_HM_BASE + 0x890000U)
#define COMPLEX1_CORE			2U

#define IPM_HEAD_SWITCH_PWRDUP		BIT_32(10)

#define IPM_SEQ_CTL			0x24U
#define IPM_SEQ_BRANCH_START(n)		(0x28U + ((n) * 4U))

#define SEQ_CTL_EN			BIT_32(0)
#define SEQ_CTL_PWR_STATE_IDX(idx)	((uint32_t)(idx) << 8)
#define SEQ_CTL_PWR_STATE_IDX_EN	BIT_32(12)
#define SEQ_CTL_START_ADDR(addr)	((uint32_t)(addr) << 16)

/* IPM sequencer program memory, four command bytes per word. */
#define APSS_SEQ_MEM(n)			(QTI_APSS_HM_BASE + 0x8c0000U + \
					 ((n) * 4U))

#define CPU_PPU_BASE(core)		(QTI_APSS_HM_BASE + 0xe80000U + \
					 ((core) * 0x100000U))
#define PPU_PWPR(core)			(CPU_PPU_BASE(core) + 0x0U)
#define PPU_PWSR(core)			(CPU_PPU_BASE(core) + 0x8U)

#define PPU_PWPR_PWR_POLICY_MASK	GENMASK_32(3, 0)
#define PPU_PWPR_PWR_POLICY_OFF		0x0U
#define PPU_PWPR_PWR_POLICY_ON		0x8U
#define PPU_PWPR_PWR_DYN_EN		BIT_32(8)
#define PPU_PWSR_PWR_DYN_STATUS		BIT_32(8)
#define PPU_PWSR_PWR_STATUS_MASK	GENMASK_32(3, 0)
#define PPU_PWSR_PWR_STATUS_OFF		0x0U

#define PPU_TIMEOUT_US			1000U
#define CPU_OFF_TIMEOUT_US		10000U

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

/* Bring up the APC1 rail before its first core: false if that fails. */
static bool apc1_power_on(void)
{
	static spinlock_t apc1_lock;
	static bool apc1_on;
	bool ok;

	spin_lock(&apc1_lock);
	if (!apc1_on) {
		apc1_on = qti_saw4_rail_on(APC1_SAW4_BASE, APC1_BOOT_MV) &&
			  apc1_repair_memories();
	}
	ok = apc1_on;
	spin_unlock(&apc1_lock);

	return ok;
}

/*
 * Power-control programs of the IPM sequencers, the ones the SM7750 boot
 * binaries' secure image uses. The core program collapses a core when its
 * PPU asks for OFF and powers it up again on a wake-up event; the complex
 * program does the same for a Cortex-A520 complex when its last core goes.
 * Their entries are at the byte offsets in the branch start registers 0 to 3.
 */
#define CPU_PWR_SEQ_START		0U
/* The complex program follows the 31 words of the core program. */
#define COMPLEX_PWR_SEQ_START		124U

static const uint32_t cpu_pwr_seq[] = {
	0x5a50555f, 0xa4a5b702, 0x029784e0, 0xb70220a0, 0x9784e0a4, 0xa720a002,
	0xa2a1e402, 0xb787e502, 0xe284e0a4, 0xe402a797, 0xa8e50282, 0x03e68802,
	0x20a08781, 0xa1e402a7, 0xe502a2ad, 0xa9e502a3, 0xa6b0b2b1, 0xaee302aa,
	0x028ee802, 0xa4b787e8, 0x039784e0, 0xe402a7e2, 0x8ee802ae, 0x0606e802,
	0x028a0606, 0x89818de3, 0x90869192, 0x82e50283, 0x02a8e502, 0x8703e688,
	0x000020a0,
};

static const uint32_t cpu_pwr_seq_branch[] = { 52U, 23U, 14U, 4U };

static const uint32_t complex_pwr_seq[] = {
	0x5a50555f, 0xe0b3b702, 0xa0029793, 0xb7022003, 0x9793e0b3, 0x2003a002,
	0xa1e402a7, 0x87e502a2, 0x93e0b3b7, 0x02a7e297, 0xe50282e4, 0xe68802a8,
	0x878103a0, 0xe402a720, 0xe502a2a1, 0xa9e502a3, 0xa6b0b1b2, 0x87e302aa,
	0x93e0b3b7, 0x02a7e297, 0x028a06e4, 0x918981e3, 0x83908692, 0x0282e502,
	0x8802a8e5, 0x8703a0e6, 0x00000020,
};

static const uint32_t complex_pwr_seq_branch[] = { 53U, 24U, 14U, 4U };

static void pwr_seq_load(const uint32_t *seq, size_t len, uint32_t start)
{
	for (size_t i = 0U; i < len; i++) {
		mmio_write_32(APSS_SEQ_MEM(start / 4U + i), seq[i]);
	}
}

static void pwr_seq_set_branches(uintptr_t ipm, const uint32_t *branch,
				 size_t len, uint32_t start)
{
	for (size_t n = 0U; n < len; n++) {
		mmio_write_32(ipm + IPM_SEQ_BRANCH_START(n), start + branch[n]);
	}
}

/*
 * Arm the sequencer of a complex for good: the hardware has it collapse the
 * complex only once all its cores are down, and power it up with the first.
 */
static void complex_seq_arm(uintptr_t ipm)
{
	pwr_seq_set_branches(ipm, complex_pwr_seq_branch,
			     ARRAY_SIZE(complex_pwr_seq_branch),
			     COMPLEX_PWR_SEQ_START);
	mmio_write_32(ipm + IPM_SEQ_CTL,
		      SEQ_CTL_START_ADDR(COMPLEX_PWR_SEQ_START) |
		      SEQ_CTL_PWR_STATE_IDX_EN | SEQ_CTL_EN);
}

/* Power up the complex of core 2 if the boot firmware left it off. */
static void complex1_power_on(void)
{
	static spinlock_t complex1_lock;

	spin_lock(&complex1_lock);
	if ((mmio_read_32(COMPLEX1_IPM_BASE + QTI_IPM_HEAD_SWITCH_CTL) &
	     IPM_HEAD_SWITCH_PWRDUP) == 0U) {
		qti_ipm_power_up(COMPLEX1_IPM_BASE, false);
		complex_seq_arm(COMPLEX1_IPM_BASE);
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
static uint32_t cpu_seq_ctl[PLATFORM_CORE_COUNT];

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	qti_cpucp_init();

	pwr_seq_load(cpu_pwr_seq, ARRAY_SIZE(cpu_pwr_seq), CPU_PWR_SEQ_START);
	pwr_seq_load(complex_pwr_seq, ARRAY_SIZE(complex_pwr_seq),
		     COMPLEX_PWR_SEQ_START);

	for (unsigned int core = 0U; core < PLATFORM_CORE_COUNT; core++) {
		pwr_seq_set_branches(CPU_IPM_BASE(core), cpu_pwr_seq_branch,
				     ARRAY_SIZE(cpu_pwr_seq_branch),
				     CPU_PWR_SEQ_START);
	}
	complex_seq_arm(COMPLEX0_IPM_BASE);

	return PSCI_E_SUCCESS;
}

/*
 * Wake a core its sequencer collapsed at CPU_OFF: the program powers it up
 * on the wake-up event. CPU_ON can arrive before the core has reached its
 * last WFI, so wait for the collapse first.
 */
static int cpu_wake(u_register_t mpidr, unsigned int core)
{
	uint64_t timeout = timeout_init_us(CPU_OFF_TIMEOUT_US);

	while ((mmio_read_32(PPU_PWSR(core)) & PPU_PWSR_PWR_STATUS_MASK) !=
	       PPU_PWSR_PWR_STATUS_OFF) {
		if (timeout_elapsed(timeout)) {
			ERROR("CPU%u did not power down\n", core);
			return PSCI_E_INTERN_FAIL;
		}
	}

	plat_ic_raise_el3_sgi(PLAT_INT_ID_CPU_WAKEUP_SGI, mpidr);

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	unsigned int core = (unsigned int)core_pos;

	if (cpu_turned_off[core]) {
		return cpu_wake(mpidr, core);
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
	unsigned int core = (unsigned int)core_pos;

	(void)states;

	plat_qti_gic_pcpu_init();

	if (cpu_turned_off[core]) {
		/* The wake-up SGI has done its job: do not take it. */
		plat_ic_clear_interrupt_pending(PLAT_INT_ID_CPU_WAKEUP_SGI);
		mmio_write_32(CPU_IPM_BASE(core) + IPM_SEQ_CTL, cpu_seq_ctl[core]);
		mmio_clrsetbits_32(PPU_PWPR(core), PPU_PWPR_PWR_POLICY_MASK,
				   PPU_PWPR_PWR_POLICY_ON);
		cpu_turned_off[core] = false;
	}

	cpucp_clkdom_init();
}

/*
 * Have the core sequencer collapse the core: arm it with the power-down entry
 * of the program and let the core PPU go to OFF on the power-down request the
 * core raises at its last WFI. CPU_ON wakes it up again.
 */
void plat_qti_pwr_domain_off(const uint8_t *states)
{
	unsigned int core = plat_my_core_pos();
	uintptr_t seq_ctl = CPU_IPM_BASE(core) + IPM_SEQ_CTL;

	(void)states;

	cpu_turned_off[core] = true;
	cpu_seq_ctl[core] = mmio_read_32(seq_ctl);

	mmio_write_32(seq_ctl, SEQ_CTL_START_ADDR(CPU_PWR_SEQ_START) |
			       SEQ_CTL_PWR_STATE_IDX(QTI_LOCAL_STATE_DEEPOFF) |
			       SEQ_CTL_PWR_STATE_IDX_EN | SEQ_CTL_EN);
	mmio_clrsetbits_32(PPU_PWPR(core), PPU_PWPR_PWR_POLICY_MASK,
			   PPU_PWPR_PWR_POLICY_OFF);
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

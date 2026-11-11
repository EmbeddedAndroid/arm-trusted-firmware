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
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>

/* Per-cluster NCC_ARCH registers, one bit (or bit group) per core. */
#define NCC_BOOT_CFG1				0x260U
#define NCC_BOOT_CFG1_CPUFETCHFWBLOCK(c)	BIT_64(16U + (c))
#define NCC_BOOT_CFG1_CORRVBARMASKWAKERCVD(c)	BIT_64(33U + (c))
#define NCC_PWR_CTL1				0x278U
#define NCC_PWR_CTL1_COREPWRUPREQ(c)		BIT_64(c)
#define NCC_PWR_CFG1				0x290U
#define NCC_PWR_CFG1_OK2ENTERC3(c)		BIT_64(c)
#define NCC_PWR_CFG1_OK2ENTERC4(c)		BIT_64(8U + (c))
#define NCC_PWR_STS1				0x2b0U
#define NCC_PWR_STS1_CORDOMISON(c)		BIT_64(c)
#define NCC_PWR_STS1_CORSPRINITDONE(c)		BIT_64(8U + (c))
#define NCC_RVBAR				0x328U
#define NCC_PWR_CTL3				0x410U
#define NCC_PWR_CTL3_CORFRCCPURVBAR(c)		BIT_64(c)
#define NCC_PWR_CTL3_CORFRCCPURVBAR_ALL		GENMASK_64(7, 0)

#define NCC_POWERUP_POLL			2000000U

/* GIC sleep sequencer of each cluster and the APSS power control bits. */
#define APSS_GIC_SEQ_BASE			0x18878000U
#define APSS_GIC_SEQ_EN(cl)			(APSS_GIC_SEQ_BASE + 0x40U + (4U * (cl)))
#define APSS_GIC_SEQ_BYPASS(cl)			(APSS_GIC_SEQ_BASE + 0x80U + (4U * (cl)))
#define APSS_GICD_PWR_CTL			0x17750800U
#define APSS_GICD_PWR_CTL_BIT2			BIT_32(2)
#define APSS_CPUSS_LPM_CTL			0x188a0088U

static uintptr_t warm_entry;

/* Serialises the read-modify-writes of the NCC registers a cluster shares. */
static spinlock_t ncc_lock;

/*
 * Per core: started is set once the core has run (it is only written and
 * read by CPU_ON callers, with caches on); release is set by CPU_ON for a
 * core parked by CPU_OFF and read by that core with its MMU off.
 */
struct nord_cpu_slot {
	uint32_t started;
	uint32_t release;
} __aligned(CACHE_WRITEBACK_GRANULE);

static struct nord_cpu_slot nord_cpu[PLATFORM_CORE_COUNT];

static uintptr_t ncc_base(unsigned int cluster)
{
	return PLAT_NCC_ARCH_BASE + (cluster * PLAT_NCC_CLUSTER_STRIDE);
}

static void ncc_clrbits_64(uintptr_t addr, uint64_t clr)
{
	mmio_write_64(addr, mmio_read_64(addr) & ~clr);
}

/*
 * Before the first CPU_ON: clear the forced RVBAR vectoring of every
 * cluster so each core starts from a known state, enable the GIC sleep
 * sequencers and set the APSS power controls the cores need to come up.
 */
static void ncc_lpm_init(void)
{
	unsigned int cl;

	for (cl = 0U; cl < PLAT_CLUSTER_COUNT; cl++) {
		ncc_clrbits_64(ncc_base(cl) + NCC_PWR_CTL3,
			       NCC_PWR_CTL3_CORFRCCPURVBAR_ALL);
	}
	dsbsy();

	for (cl = 0U; cl < PLAT_CLUSTER_COUNT; cl++) {
		mmio_write_32(APSS_GIC_SEQ_EN(cl), 1U);
		mmio_write_32(APSS_GIC_SEQ_BYPASS(cl), 0U);
	}
	mmio_clrbits_32(APSS_GICD_PWR_CTL, APSS_GICD_PWR_CTL_BIT2);
	mmio_setbits_32(APSS_CPUSS_LPM_CTL, BIT_32(0));
	dsbsy();
	isb();
}

/*
 * Power a core up through its cluster's NCC and let it fetch from NCC_RVBAR.
 * The wake-received mask is set and cleared around the power-up request:
 * its 1 to 0 transition is what vectors a cold core to RVBAR. The forced
 * RVBAR vectoring is dropped once the core runs, so that a core woken from
 * a low power state resumes where it stopped.
 */
static int ncc_power_on_core(unsigned int cluster, unsigned int core)
{
	uintptr_t base = ncc_base(cluster);
	unsigned int to;

	spin_lock(&ncc_lock);

	mmio_write_64(base + NCC_RVBAR, (uint64_t)warm_entry);
	mmio_setbits_64(base + NCC_PWR_CTL3, NCC_PWR_CTL3_CORFRCCPURVBAR(core));
	mmio_setbits_64(base + NCC_BOOT_CFG1,
			NCC_BOOT_CFG1_CORRVBARMASKWAKERCVD(core));

	ncc_clrbits_64(base + NCC_PWR_CFG1, NCC_PWR_CFG1_OK2ENTERC3(core) |
					    NCC_PWR_CFG1_OK2ENTERC4(core));
	mmio_setbits_64(base + NCC_BOOT_CFG1, NCC_BOOT_CFG1_CPUFETCHFWBLOCK(core));
	ncc_clrbits_64(base + NCC_BOOT_CFG1,
		       NCC_BOOT_CFG1_CORRVBARMASKWAKERCVD(core));
	mmio_setbits_64(base + NCC_PWR_CTL1, NCC_PWR_CTL1_COREPWRUPREQ(core));

	for (to = 0U; to < NCC_POWERUP_POLL; to++) {
		if ((mmio_read_64(base + NCC_PWR_STS1) &
		     NCC_PWR_STS1_CORSPRINITDONE(core)) != 0U) {
			break;
		}
	}
	for (to = 0U; to < NCC_POWERUP_POLL; to++) {
		if ((mmio_read_64(base + NCC_PWR_STS1) &
		     NCC_PWR_STS1_CORDOMISON(core)) != 0U) {
			break;
		}
	}
	if (to == NCC_POWERUP_POLL) {
		spin_unlock(&ncc_lock);
		return PSCI_E_INTERN_FAIL;
	}

	ncc_clrbits_64(base + NCC_BOOT_CFG1, NCC_BOOT_CFG1_CPUFETCHFWBLOCK(core));
	/* Wake-ups from the low power states resume the core after its WFI. */
	ncc_clrbits_64(base + NCC_PWR_CTL3, NCC_PWR_CTL3_CORFRCCPURVBAR(core));
	dsbsy();
	isb();

	spin_unlock(&ncc_lock);

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	warm_entry = warmboot_entry;
	nord_cpu[plat_my_core_pos()].started = 1U;

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	static bool lpm_init_done;
	int ret;
	unsigned int core = (unsigned int)MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cluster = (unsigned int)MPIDR_AFFLVL2_VAL(mpidr);

	if ((cluster >= PLAT_CLUSTER_COUNT) ||
	    (core >= PLAT_CORE_COUNT_PER_CLUSTER)) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (!lpm_init_done) {
		ncc_lpm_init();
		lpm_init_done = true;
	}

	/*
	 * gicv3_cpuif_enable() on the new core polls GICR_WAKER.ChildrenAsleep,
	 * which never clears while its redistributor is powered down.
	 */
	gicv3_rdistif_on((unsigned int)core_pos);

	/*
	 * The NCC power-up request only vectors a core that is powered down;
	 * a core parked by CPU_OFF is still on and is released instead.
	 */
	if (nord_cpu[core_pos].started != 0U) {
		nord_cpu[core_pos].release = 1U;
		flush_dcache_range((uintptr_t)&nord_cpu[core_pos],
				   sizeof(nord_cpu[core_pos]));
		dsbsy();
		sev();
		return PSCI_E_SUCCESS;
	}

	ret = ncc_power_on_core(cluster, core);
	if (ret == PSCI_E_SUCCESS) {
		nord_cpu[core_pos].started = 1U;
	}

	return ret;
}

void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
}

const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_C4,
				       PSTATE_TYPE_STANDBY),
		0,
	};

	return idle_states;
}

/*
 * C4: with its ok2EnterC3 and ok2EnterC4 bits set in NCC_PWR_CFG1, the
 * cluster NCC saves the core's context and powers it down once it is in
 * WFI, and powers it up and resumes it after the WFI on the next interrupt
 * for the core. The bits are only set around the WFI of a C4 request, so a
 * WFI standby stays a WFI.
 */
static void ncc_core_c4_allow(bool allow)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int core = (unsigned int)MPIDR_AFFLVL1_VAL(mpidr);
	uintptr_t cfg1 = ncc_base((unsigned int)MPIDR_AFFLVL2_VAL(mpidr)) +
			 NCC_PWR_CFG1;
	uint64_t bits = NCC_PWR_CFG1_OK2ENTERC3(core) |
			NCC_PWR_CFG1_OK2ENTERC4(core);

	spin_lock(&ncc_lock);
	if (allow) {
		mmio_write_64(cfg1, mmio_read_64(cfg1) | bits);
	} else {
		ncc_clrbits_64(cfg1, bits);
	}
	spin_unlock(&ncc_lock);
}

void plat_qti_cpu_retention_enter(plat_local_state_t cpu_state)
{
	if (cpu_state == QTI_LOCAL_STATE_C4) {
		ncc_core_c4_allow(true);
	}
}

void plat_qti_cpu_retention_exit(plat_local_state_t cpu_state)
{
	if (cpu_state == QTI_LOCAL_STATE_C4) {
		ncc_core_c4_allow(false);
	}
}

void plat_qti_pwr_domain_off(const uint8_t *states)
{
	(void)states;
}

/*
 * The NCC does not collapse a core that executes WFI after CPU_OFF, and
 * powering it up again does not reset it, so the core is parked in BL31:
 * PSCI has already disabled its GIC CPU interface and, with hardware
 * assisted coherency, leaves its caches to the hardware. It waits with the
 * MMU off for CPU_ON to release it and then takes the warm boot path as if
 * it had been powered up. The release flag is read and cleared with the
 * data cache off, so the releasing core cleans it to the PoC.
 */
void __dead2 plat_qti_cpu_park(const psci_power_state_t *target_state)
{
	struct nord_cpu_slot *slot = &nord_cpu[plat_my_core_pos()];

	(void)target_state;

	disable_mmu_icache_el3();
	while (mmio_read_32((uintptr_t)&slot->release) == 0U) {
		wfe();
	}
	mmio_write_32((uintptr_t)&slot->release, 0U);
	dsbsy();

	bl31_warm_entrypoint();
	panic();
}

void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	(void)states;
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	(void)states;

	plat_qti_gic_pcpu_init();
}

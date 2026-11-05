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
#include <drivers/delay_timer.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/cpucp/cpucp.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>

/* Per-core bits in the cluster's NCC_ARCH block. */
#define NCC_PWR_CFG1			0x290U
#define NCC_PWR_STS1			0x2b0U
#define NCC_PWR_CFG1_OK2ENTERC3(c)	BIT_64(c)
#define NCC_PWR_CFG1_OK2ENTERC4(c)	BIT_64(8U + (c))
#define NCC_PWR_STS1_CORDOMISON(c)	BIT_64(c)

#define CPU_OFF_TIMEOUT_US		10000U

/*
 * SM8750 core power: CPUCP powers the Oryon cores. A core is started with
 * an MPIDR-addressed POWER_STATE_SET, after which it fetches from its
 * cluster's NCC_RVBAR (qti_sec_core_remap()). CPUCP starts a core only
 * the first time; one turned off since then is woken as below.
 */
static bool cpu_turned_off[PLATFORM_CORE_COUNT];

/* Set by CPU_ON once the core is down; the core reads it with its MMU off. */
struct cpu_wake {
	uint32_t req;
} __aligned(CACHE_WRITEBACK_GRANULE);

static struct cpu_wake cpu_wake_req[PLATFORM_CORE_COUNT];

static void cpu_wake_req_set(unsigned int core_pos, uint32_t req)
{
	cpu_wake_req[core_pos].req = req;
	flush_dcache_range((uintptr_t)&cpu_wake_req[core_pos],
			   sizeof(cpu_wake_req[core_pos]));
}

/* NCC_PWR_CFG1 holds the bits of every core of the cluster. */
static spinlock_t ncc_lock;

static uintptr_t ncc_base(u_register_t mpidr)
{
	return PLAT_NCC_ARCH_BASE +
	       (MPIDR_AFFLVL2_VAL(mpidr) * PLAT_NCC_CLUSTER_STRIDE);
}

static void ncc_c4_allow(u_register_t mpidr, bool allow)
{
	unsigned int core = (unsigned int)MPIDR_AFFLVL1_VAL(mpidr);
	uint64_t bits = NCC_PWR_CFG1_OK2ENTERC4(core) |
			NCC_PWR_CFG1_OK2ENTERC3(core);
	uintptr_t cfg = ncc_base(mpidr) + NCC_PWR_CFG1;

	spin_lock(&ncc_lock);
	if (allow) {
		mmio_setbits_64(cfg, bits);
	} else {
		mmio_write_64(cfg, mmio_read_64(cfg) & ~bits);
	}
	spin_unlock(&ncc_lock);
}

/*
 * Wake a core the NCC powered down at CPU_OFF: its wake-up SGI powers it up
 * and the core continues after its WFI (plat_qti_cpu_off_wfi()). CPU_ON can
 * arrive before the core has reached its last WFI, and a pending SGI keeps
 * it out of C4, so wait for its power domain to go off first.
 */
static int cpu_wake(u_register_t mpidr, unsigned int core_pos)
{
	unsigned int core = (unsigned int)MPIDR_AFFLVL1_VAL(mpidr);
	uint64_t timeout = timeout_init_us(CPU_OFF_TIMEOUT_US);

	while ((mmio_read_64(ncc_base(mpidr) + NCC_PWR_STS1) &
		NCC_PWR_STS1_CORDOMISON(core)) != 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("CPU%u did not power down\n", core_pos);
			return PSCI_E_INTERN_FAIL;
		}
	}

	cpu_wake_req_set(core_pos, 1U);
	gicv3_rdistif_on(core_pos);
	plat_ic_raise_el3_sgi(PLAT_INT_ID_CPU_WAKEUP_SGI, mpidr);

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	unsigned int core = (unsigned int)MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cluster = (unsigned int)MPIDR_AFFLVL2_VAL(mpidr);
	int ret;

	/*
	 * The core position ignores Aff0 and Aff3 and strides by six cores
	 * per cluster, so it also accepts MPIDRs no core has (0x600 is core
	 * position 6, core 0x10000).
	 */
	if ((MPIDR_AFFLVL0_VAL(mpidr) != 0U) ||
	    (MPIDR_AFFLVL3_VAL(mpidr) != 0U) ||
	    (core >= ((cluster == 0U) ? PLAT_CLUSTER0_CORE_COUNT :
					PLAT_CLUSTER1_CORE_COUNT)) ||
	    ((chipinfo_get_disabled_cpus(cluster) & BIT_32(core)) != 0U)) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (cpu_turned_off[core_pos]) {
		return cpu_wake(mpidr, (unsigned int)core_pos);
	}

	/* The core's warm boot brings up its CPU interface. */
	gicv3_rdistif_on((unsigned int)core_pos);

	ret = cpucp_core_power_set(mpidr, true);
	if (ret != 0) {
		ERROR("CPUCP: power-on of 0x%lx failed: %d\n", mpidr, ret);
		return PSCI_E_INTERN_FAIL;
	}

	return PSCI_E_SUCCESS;
}

void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)states;

	plat_qti_gic_pcpu_init();

	if (cpu_turned_off[core_pos]) {
		/* The wake-up SGI has done its job: do not take it. */
		plat_ic_clear_interrupt_pending(PLAT_INT_ID_CPU_WAKEUP_SGI);
		ncc_c4_allow(read_mpidr_el1(), false);
		cpu_wake_req_set((unsigned int)core_pos, 0U);
		cpu_turned_off[core_pos] = false;
	}
}

/*
 * Have the NCC power the core down: with the core's ok2EnterC4 and
 * ok2EnterC3 bits set in NCC_PWR_CFG1, the NCC collapses the core once it
 * is in its last WFI. Any pending SGI or PPI other than the wake-up SGI
 * keeps the core out of C4, so only that one stays enabled; CPU_ON sends
 * it once the core is down.
 */
void plat_qti_pwr_domain_off(const uint8_t *states)
{
	unsigned int core_pos = plat_my_core_pos();
	unsigned int id;

	(void)states;

	for (id = 0U; id < MIN_SPI_ID; id++) {
		if (id != PLAT_INT_ID_CPU_WAKEUP_SGI) {
			gicv3_disable_interrupt(id, core_pos);
		}
	}
	gicv3_enable_interrupt(PLAT_INT_ID_CPU_WAKEUP_SGI, core_pos);

	cpu_turned_off[core_pos] = true;
	ncc_c4_allow(read_mpidr_el1(), true);
}

/*
 * The core's last WFI of CPU_OFF. On the wake-up SGI the NCC resumes the core
 * after the WFI, not from its reset address, so enter the warm boot path
 * here once CPU_ON has asked for it, with the MMU and caches off as at reset.
 * Any other wake-up is spurious: go back to WFI.
 */
void __dead2 plat_qti_cpu_off_wfi(const psci_power_state_t *target_state)
{
	uintptr_t req = (uintptr_t)&cpu_wake_req[plat_my_core_pos()].req;

	(void)target_state;

	dsbsy();
	disable_mmu_icache_el3();
	while (mmio_read_32(req) == 0U) {
		wfi();
	}

	bl31_warm_entrypoint();
	panic();
}

void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	(void)states;
}

/* Every idle state is a retention state: the redistributor keeps its setup. */
void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	(void)states;
}

/*
 * The idle states of the SM8750 device tree: CPU C4, cluster CL5 and the
 * system state, all entered with a WFI and coordinated by PSCI. Deeper
 * hardware retention is not programmed.
 */
const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		qti_make_pwrstate_lvl1(QTI_LOCAL_STATE_CL5,
				       QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		qti_make_pwrstate_lvl3(QTI_LOCAL_STATE_PDC_RET,
				       QTI_LOCAL_STATE_RSC_RET,
				       QTI_LOCAL_STATE_CL5,
				       QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		0,
	};

	return idle_states;
}

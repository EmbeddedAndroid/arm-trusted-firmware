/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
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
 * its 1 to 0 transition is what vectors a cold core to RVBAR.
 */
static int ncc_power_on_core(unsigned int cluster, unsigned int core)
{
	uintptr_t base = ncc_base(cluster);
	unsigned int to;

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
		return PSCI_E_INTERN_FAIL;
	}

	ncc_clrbits_64(base + NCC_BOOT_CFG1, NCC_BOOT_CFG1_CPUFETCHFWBLOCK(core));
	dsbsy();
	isb();

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	warm_entry = warmboot_entry;

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	static bool lpm_init_done;
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

	return ncc_power_on_core(cluster, core);
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
		0,
	};

	return idle_states;
}

void plat_qti_pwr_domain_off(const uint8_t *states)
{
	(void)states;
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

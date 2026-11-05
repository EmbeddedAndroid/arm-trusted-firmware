/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/cpucp/cpucp.h>
#include <lib/psci/psci.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * SM8750 core power: CPUCP powers the Oryon cores. A core is started with
 * an MPIDR-addressed POWER_STATE_SET, after which it fetches from its
 * cluster's NCC_RVBAR (qti_sec_core_remap()). The boot core was started
 * before BL2.
 */
static bool cpu_started[PLATFORM_CORE_COUNT] = {
	[0] = true,
};

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

	/* No core power-down is ported: a core turned off stays in WFI. */
	if (cpu_started[core_pos]) {
		return PSCI_E_INTERN_FAIL;
	}

	/* The core's warm boot brings up its CPU interface. */
	gicv3_rdistif_on((unsigned int)core_pos);

	ret = cpucp_core_power_set(mpidr, true);
	if (ret != 0) {
		ERROR("CPUCP: power-on of 0x%lx failed: %d\n", mpidr, ret);
		return PSCI_E_INTERN_FAIL;
	}
	cpu_started[core_pos] = true;

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

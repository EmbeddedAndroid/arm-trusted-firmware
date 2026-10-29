/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/psci/psci.h>

#include <platform_def.h>
#include <qti_plat.h>

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	return PSCI_E_SUCCESS;
}

/*
 * Secondary cores are not powered on yet: the SM7750 core power-on sequence
 * and warm boot vector are not ported, so CPU_ON fails and the OS runs on the
 * boot core only.
 */
int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	(void)core_pos;

	WARN("CPU_ON 0x%lx: not supported on SM7750 yet\n", mpidr);

	return PSCI_E_INTERN_FAIL;
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

/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>

#include <bl31qtilib_defs.h>
#include <bl31qtilib_interface.h>

/*
 * Build variant string
 */
const char bl31qtilib_build_variant[] = "stub";

static uint32_t boot_cpu_num;

void bl31qtilib_set_boot_cpu_num(uint32_t boot_cpu)
{
	boot_cpu_num = boot_cpu;
}

uint32_t bl31qtilib_get_boot_cpu_num(void)
{
	return boot_cpu_num;
}

bool bl31qtilib_is_quick_boot(void)
{
	return false;
}

void bl31qtilib_delay_timer_init(void)
{
}

void bl31qtilib_bl31_platform_early_setup(void)
{
	ERROR("Please use bl31qtilib_PATH while building TF-A\n");
	ERROR("Please refer docs/plat/qti.rst for more details.\n");
}

void bl31qtilib_bl31_platform_setup(void)
{
	ERROR("Please use bl31qtilib_PATH while building TF-A\n");
	ERROR("Please refer docs/plat/qti.rst for more details.\n");
}

void bl31qtilib_invoke_isr(uint32_t irq, void *handle)
{
	(void)irq;
	(void)handle;
}

void bl31qtilib_panic(void)
{
}

int bl31qtilib_psci_init(uintptr_t warmboot_entry, uint32_t mpidr)
{
	(void)warmboot_entry;
	(void)mpidr;

	return 0;
}

int32_t bl31qtilib_psci_power_domain_on(uint32_t mpidr)
{
	(void)mpidr;

	return 0;
}

void bl31qtilib_psci_power_domain_on_finish(uint32_t mpidr,
					    const uint8_t *pwr_states)
{
	(void)mpidr;
	(void)pwr_states;
}

void bl31qtilib_psci_cpu_pwr_down(uint32_t mpidr)
{
	(void)mpidr;
}

int32_t bl31qtilib_psci_suspend(uint32_t mpidr, const uint8_t *pwr_states)
{
	(void)mpidr;
	(void)pwr_states;

	return 0;
}

int32_t bl31qtilib_psci_resume(uint32_t mpidr, const uint8_t *pwr_states)
{
	(void)mpidr;
	(void)pwr_states;

	return 0;
}

uint64_t bl31qtilib_psci_get_suspend_state(void)
{
	return 0U;
}

int32_t bl31qtilib_psci_validate_power_state(unsigned int power_state,
					     uint8_t *req_state)
{
	(void)power_state;
	(void)req_state;

	return 0;
}

uint8_t bl31qtilib_psci_get_target_pwr_state(unsigned int lvl,
					     const uint8_t *states,
					     unsigned int ncpu, uint32_t mpidr)
{
	(void)lvl;
	(void)states;
	(void)ncpu;
	(void)mpidr;

	return 0U;
}

void bl31qtilib_psci_system_off(void)
{
	ERROR("System off not implemented in stub\n");
}

void bl31qtilib_psci_system_reset(void)
{
	ERROR("System reset not implemented in stub\n");
}

void bl31qtilib_psci_system_reset2(int reset_type, uint64_t cookie)
{
	(void)reset_type;
	(void)cookie;

	ERROR("System reset2 not implemented in stub\n");
}

void bl31qtilib_bl31_platform_post_coldboot_setup(void)
{
}

uint32_t bl31qtilib_get_chip_id(void)
{
	return 0;
}

uint32_t bl31qtilib_get_soc_revision(void)
{
	return 0;
}

int bl31qtilib_psci_warm_reset(void)
{
	return -1;
}

int bl31qtilib_config_hw_for_offline_ram_dump(uint32_t disable_wd_dbg,
					      uint32_t boot_partition_sel)
{
	(void)disable_wd_dbg;
	(void)boot_partition_sel;

	return -1;
}

int bl31qtilib_validate_ns_address(uintptr_t address, size_t size)
{
	(void)address;
	(void)size;

	return -1;
}

int bl31qtilib_secure_io_read(uintptr_t addr, struct smc_rsp_s *rsp)
{
	(void)addr;
	(void)rsp;

	return -1;
}

int bl31qtilib_secure_io_write(uintptr_t addr, uint32_t value)
{
	(void)addr;
	(void)value;

	return -1;
}

int bl31qtilib_get_subsystem_debug_options(uint32_t subsys_id,
					    struct smc_rsp_s *rsp)
{
	(void)subsys_id;
	(void)rsp;

	return -1;
}

int sysdbg_reset_check(void)
{
	return 0;
}

void sysdbg_entry_handler(void)
{
}

/*
 * Stub: invasive and non-invasive debug are both disabled by default.
 */
bool bl31qtilib_is_invasive_debug_enabled(int security_state)
{
	(void)security_state;

	return false;
}

bool bl31qtilib_is_non_invasive_debug_enabled(int security_state)
{
	(void)security_state;

	return false;
}

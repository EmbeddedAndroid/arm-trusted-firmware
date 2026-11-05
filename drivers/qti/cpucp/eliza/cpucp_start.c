/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <cpucp.h>
#include <cpucp_hwio.h>
#include <lib/mmio.h>

/*
 * The boot firmware loads CPUCP and releases it from reset just before it
 * loads the secure world, with the EPSS open to the normal world for the
 * kernel's frequency votes. The host interface is version 1: a version word
 * and the SLT part flag. It has no ready indication.
 */
#define CPUCP_SHARED_VERSION		1U
#define CPUCP_SHARED_VERSION_OFF	0x0U
#define CPUCP_SHARED_IS_SLT_PLAT_OFF	0x10U

void qti_cpucp_init(void)
{
	uint32_t version = mmio_read_32(CPUCP_SHARED_DATA_BASE +
					CPUCP_SHARED_VERSION_OFF);

	if (version != CPUCP_SHARED_VERSION) {
		ERROR("CPUCP: unsupported firmware interface %u\n", version);
		return;
	}

	/* SLT status unknown: there is no platform info service. */
	mmio_write_32(CPUCP_SHARED_DATA_BASE + CPUCP_SHARED_IS_SLT_PLAT_OFF,
		      UINT32_MAX);

	mmio_setbits_32(EPSSTOP_GLOBAL_ENABLE, 1U);
	mmio_setbits_32(EPSSTOP_MUC_CLK_CTRL, EPSSTOP_MUC_CLK_CTRL_CORE_CLK_EN);

	/*
	 * CPUCP is still initialising when BL31 gets here, and a request it
	 * answers then leaves it unable to bring up the gold clock domains
	 * later. Nothing is sent from the boot core: the silver domain is
	 * enabled with the other domains, by the first of its cores to come
	 * up, long after CPUCP has initialised.
	 */
}

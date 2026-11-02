/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <cpucp.h>
#include <cpucp_hwio.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

/*
 * The boot firmware loads CPUCP and releases it from reset just before it
 * loads the secure world, with the EPSS open to the normal world for the
 * kernel's frequency votes. The host interface is version 1: a version word,
 * a pointer, the packed 3-byte device configuration and the SLT part flag.
 * It has no ready indication.
 */
#define CPUCP_SHARED_VERSION		1U
#define CPUCP_SHARED_VERSION_OFF	0x0U
#define CPUCP_SHARED_DEVCFG_OFF		0x10U
#define CPUCP_SHARED_IS_SLT_PLAT_OFF	0x14U

#define CPUCP_DEVCFG_VERSION		1U
#define CPUCP_DEVCFG_SIZE		3U

/*
 * SAW4 SRM of the silver/L3 (APC0) and gold/prime (APC1) rails: CPUCP sets
 * the rail voltages through it, using the PMIC command words held in
 * PMIC_DATA. The boot firmware leaves them zero.
 */
#define SAW4_SRM_BASE(rail)		(0x17900000U + ((rail) * 0x1000U))
#define SAW4_SRM_CTL(rail)		(SAW4_SRM_BASE(rail) + 0x0U)
#define SAW4_CFG(rail)			(SAW4_SRM_BASE(rail) + 0xcU)
#define SAW4_SRM_PMIC_DATA(rail, n)	(SAW4_SRM_BASE(rail) + 0x40U + \
					 ((n) * 4U))

#define SAW4_SRM_CTL_EN			0x3U
#define SAW4_CFG_CLK_DIV_MASK		0x1fU
#define SAW4_CFG_CLK_DIV		0x14U
#define SAW4_SRM_PMIC_DATA_VCTL		0x03030080U
#define SAW4_SRM_PMIC_DATA_EN		0x00030000U

static void cpucp_saw4_srm_init(void)
{
	for (unsigned int rail = 0U; rail < 2U; rail++) {
		mmio_write_32(SAW4_SRM_CTL(rail), SAW4_SRM_CTL_EN);
		mmio_clrsetbits_32(SAW4_CFG(rail), SAW4_CFG_CLK_DIV_MASK,
				   SAW4_CFG_CLK_DIV);
		mmio_write_32(SAW4_SRM_PMIC_DATA(rail, 0U),
			      SAW4_SRM_PMIC_DATA_VCTL);
		mmio_write_32(SAW4_SRM_PMIC_DATA(rail, 1U),
			      SAW4_SRM_PMIC_DATA_EN);
	}
}

void qti_cpucp_init(void)
{
	uintptr_t devcfg = CPUCP_SHARED_DATA_BASE + CPUCP_SHARED_DEVCFG_OFF;
	uint32_t version = mmio_read_32(CPUCP_SHARED_DATA_BASE +
					CPUCP_SHARED_VERSION_OFF);

	if (version != CPUCP_SHARED_VERSION) {
		ERROR("CPUCP: unsupported firmware interface %u\n", version);
		return;
	}

	/* Per-core DCVS off, as the stock device configuration has it. */
	mmio_write_8(devcfg, CPUCP_DEVCFG_VERSION);
	mmio_write_8(devcfg + 1U, CPUCP_DEVCFG_SIZE);
	mmio_write_8(devcfg + 2U, 0U);

	/* SLT status unknown: there is no platform info service. */
	mmio_write_32(CPUCP_SHARED_DATA_BASE + CPUCP_SHARED_IS_SLT_PLAT_OFF,
		      UINT32_MAX);

	cpucp_saw4_srm_init();

	mmio_setbits_32(EPSSTOP_GLOBAL_ENABLE, 1U);
	mmio_setbits_32(EPSSTOP_MUC_CLK_CTRL, EPSSTOP_MUC_CLK_CTRL_CORE_CLK_EN);

	/*
	 * Nothing is sent from here: a request this early in CPUCP's own
	 * initialisation breaks the later domain enables, and the version 1
	 * interface has no ready indication. The silver domain is enabled from
	 * the boot core when the OS starts the first secondary core (asked by
	 * core 1 itself, CPUCP answers and core 1 never reaches the normal
	 * world); every other domain by the first of its cores to come up.
	 */
}

/*
 * CPUCP answers the enable before it has the domain under its control, and
 * a core of the domain powered up, or a further request sent, before then
 * fails. CPUCP publishes the domain's performance state once it is done.
 */
void cpucp_plat_clkdom_wait(unsigned int clkdom)
{
	uint64_t timeout = timeout_init_us(CPUCP_CLKDOM_STATE_TIMEOUT_US);

	while (mmio_read_32(EPSS_CLKDOM_STATE(clkdom)) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("CPUCP: clock domain %u did not come up\n",
			      clkdom);
			return;
		}
	}
}

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HOYA_DSU_CFG_H
#define HOYA_DSU_CFG_H

#include <lib/utils_def.h>

#include <hoya_dsu_pm.h>
#include <lanai_def.h>

/* Nominal voltage of the APC1 and APC2 rails until CPUCP scales them. */
#define HOYA_GOLD_RAIL_BOOT_MV		824U

/*
 * The Cortex-A720 cores 2 to 4 sit on the APC2 rail, the Cortex-A720 cores
 * 5 and 6 and the Cortex-X4 core 7 on APC1. The second Cortex-A520 shares the
 * complex the boot firmware powers with core 0.
 */
static struct hoya_gold_rail hoya_gold_rails[] = {
	{
		/* APC2 */
		.first_core = 2U,
		.last_core = 4U,
		.saw4 = QTI_APSS_HM_BASE + 0x902000U,
		.repair_en = HOYA_CL_IPM_BASE + 0x278U,
		.repair_val = HOYA_CL_IPM_BASE + 0x27cU,
		.repair_sts = HOYA_CL_IPM_BASE + 0x288U,
		.repair_bit = BIT_32(17),
		.repair_done = BIT_32(16),
	},
	{
		/* APC1 */
		.first_core = 5U,
		.last_core = 7U,
		.saw4 = QTI_APSS_HM_BASE + 0x901000U,
		.repair_en = HOYA_CL_IPM_BASE + 0x180U,
		.repair_val = HOYA_CL_IPM_BASE + 0x184U,
		.repair_sts = HOYA_CL_IPM_BASE + 0x190U,
		.repair_bit = BIT_32(8),
		.repair_done = BIT_32(4),
	},
};

#endif /* HOYA_DSU_CFG_H */

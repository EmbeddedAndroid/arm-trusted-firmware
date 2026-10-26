/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HOYA_DSU_CFG_H
#define HOYA_DSU_CFG_H

#include <lib/utils_def.h>

#include <hoya_dsu_pm.h>
#include <kailua_def.h>

/* Nominal voltage of the APC1 rail until CPUCP scales it. */
#define HOYA_GOLD_RAIL_BOOT_MV		824U

/* The Cortex-A715, Cortex-A710 and Cortex-X3 cores 3 to 7 sit on APC1. */
static struct hoya_gold_rail hoya_gold_rails[] = {
	{
		/* APC1 */
		.first_core = 3U,
		.last_core = 7U,
		.saw4 = QTI_APSS_HM_BASE + 0x901000U,
		.repair_en = HOYA_CL_IPM_BASE + 0x180U,
		.repair_val = HOYA_CL_IPM_BASE + 0x184U,
		.repair_sts = HOYA_CL_IPM_BASE + 0x190U,
		.repair_bit = BIT_32(8),
		.repair_done = BIT_32(4),
	},
};

/*
 * The Cortex-A510 cores 1 and 2 share complex 1; complex 0 holds the boot
 * core alone.
 */
static const struct hoya_complex hoya_complexes[] = {
	{
		.first_core = 1U,
		.last_core = 2U,
		.ipm = QTI_APSS_HM_BASE + 0x890000U,
	},
};

#endif /* HOYA_DSU_CFG_H */

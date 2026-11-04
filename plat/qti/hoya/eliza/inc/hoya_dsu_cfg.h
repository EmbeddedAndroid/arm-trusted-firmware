/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HOYA_DSU_CFG_H
#define HOYA_DSU_CFG_H

#include <lib/utils_def.h>

#include <eliza_def.h>
#include <hoya_dsu_pm.h>

/* Cluster IPM block, home of the rail sequencers. */
#define ELIZA_CL_IPM_BASE		(QTI_APSS_HM_BASE + 0x8b0000U)

/* Nominal voltage of the APC1 rail until the OS scales it. */
#define HOYA_GOLD_RAIL_BOOT_MV		824U

/* The Cortex-A720 cores 3 to 7 sit on APC1, off at cold boot. */
static struct hoya_gold_rail hoya_gold_rails[] = {
	{
		/* APC1 */
		.first_core = 3U,
		.last_core = 7U,
		.saw4 = QTI_APSS_HM_BASE + 0x901000U,
		.repair_en = ELIZA_CL_IPM_BASE + 0x180U,
		.repair_val = ELIZA_CL_IPM_BASE + 0x184U,
		.repair_sts = ELIZA_CL_IPM_BASE + 0x190U,
		.repair_bit = BIT_32(8),
		.repair_done = BIT_32(4),
	},
};

/*
 * Cores 0 and 1 share the Cortex-A520 complex the boot firmware powers with
 * core 0; core 2 is alone in a second complex, off at cold boot.
 */
static const struct hoya_complex hoya_complexes[] = {
	{
		.first_core = 2U,
		.last_core = 2U,
		.ipm = QTI_APSS_HM_BASE + 0x890000U,
	},
};

#endif /* HOYA_DSU_CFG_H */

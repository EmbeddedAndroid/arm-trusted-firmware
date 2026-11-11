/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_PLATFORM_PM_H
#define QTI_PLATFORM_PM_H

#include <arch.h>

#define QTI_PWR_LVL0				MPIDR_AFFLVL0
#define QTI_PWR_LVL1				MPIDR_AFFLVL1

/* The device tree idle states use the State IDs of the boot firmware. */
#define PLAT_QTI_VALIDATE_POWER_STATE		plat_qti_validate_power_state
/* Suspended cores are held in BL31 instead of being powered down. */
#define PLAT_QTI_PWR_DOMAIN_PWR_DOWN		plat_qti_pwr_domain_pwr_down

#ifndef __ASSEMBLER__
#include <cdefs.h>

void __dead2 eliza_cpu_hold_wait(void);
#endif

/* Local power states, in the State-ID encoding of the SM7750 device tree. */
#define QTI_LOCAL_STATE_STB			1
#define QTI_LOCAL_STATE_RET			2
#define QTI_LOCAL_STATE_OFF			3
#define QTI_LOCAL_STATE_DEEPOFF			4

#define PLAT_MAX_RET_STATE			QTI_LOCAL_STATE_RET
#define PLAT_MAX_OFF_STATE			QTI_LOCAL_STATE_DEEPOFF

/*
 * One rail, one DynamIQ cluster with the 3 Cortex-A520 and 5 Cortex-A720
 * cores. MPIDR has MT set: Aff1 is the core.
 */
#define PLAT_CLUSTER_COUNT			1
#define PLAT_CORE_COUNT_PER_CLUSTER		8
#define PLATFORM_CORE_COUNT			8
#define PLAT_MAX_PWR_LVL			2

#define PLAT_TREE_DESC				\
{						\
	1, /* CX rail */			\
	PLAT_CLUSTER_COUNT,			\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
}

#define PLAT_CLUSTER_COUNT_IDX			(1U)
#define PLAT_CLUSTER_CORE_COUNT_START_IDX	(2U)

#endif /* QTI_PLATFORM_PM_H */

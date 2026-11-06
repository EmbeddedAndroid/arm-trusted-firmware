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
#define QTI_PWR_LVL2				MPIDR_AFFLVL2
#define QTI_PWR_LVL3				MPIDR_AFFLVL3

/*
 * Local power states. The native back-end offers CPU standby (WFI) only:
 * cores are never power collapsed, so there is no off state but the one
 * CPU_OFF reports to PSCI.
 */
#define QTI_LOCAL_STATE_RUN			0
#define QTI_LOCAL_STATE_STB			1
#define QTI_LOCAL_STATE_RET			2
#define QTI_LOCAL_STATE_OFF			3
#define QTI_LOCAL_STATE_DEEPOFF			4

#define PLAT_MAX_RET_STATE			QTI_LOCAL_STATE_RET
#define PLAT_MAX_OFF_STATE			QTI_LOCAL_STATE_DEEPOFF

/*
 * CX rail, then three Oryon clusters of six cores. MPIDR Aff2 is the
 * cluster and Aff1 the core.
 */
#define PLAT_CLUSTER_COUNT			3
#define PLAT_CORE_COUNT_PER_CLUSTER		6
#define PLATFORM_CORE_COUNT			(PLAT_CLUSTER_COUNT * \
						 PLAT_CORE_COUNT_PER_CLUSTER)
#define PLAT_MAX_PWR_LVL			2

#define PLAT_TREE_DESC				\
{						\
	1, /* CX rail */			\
	PLAT_CLUSTER_COUNT,			\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
}

#define PLAT_RAIL_COUNT_IDX			(0U)
#define PLAT_CLUSTER_COUNT_IDX			(1U)
#define PLAT_CLUSTER_CORE_COUNT_START_IDX	(2U)

#endif /* QTI_PLATFORM_PM_H */

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
 * Local power states. STB is a WFI. C4 is the core state in which the
 * cluster NCC powers the core down and keeps its context: it resumes after
 * its WFI, so to PSCI it is a retention state. CL4 is the cluster LPM, which
 * keeps the cores' context too, and SYS the subsystem state, in which only
 * the clusters idle. CL4 is as deep as C4 so that PSCI accepts it above cores
 * in C4; the device tree passes it as 5 (see plat_qti_map_pwr_state()).
 * CPU_OFF uses the off states.
 */
#define QTI_LOCAL_STATE_RUN			0
#define QTI_LOCAL_STATE_STB			1
#define QTI_LOCAL_STATE_SYS			1
#define QTI_LOCAL_STATE_C4			3
#define QTI_LOCAL_STATE_CL4			3
#define QTI_LOCAL_STATE_RET			3
#define QTI_LOCAL_STATE_OFF			6
#define QTI_LOCAL_STATE_DEEPOFF			7

#define PLAT_MAX_RET_STATE			QTI_LOCAL_STATE_RET
#define PLAT_MAX_OFF_STATE			QTI_LOCAL_STATE_DEEPOFF

/*
 * In OS-initiated mode the device tree passes the last level in bits
 * [27:24] of the State-ID (cluster 0x01000053, system 0x02000153).
 */
#define QTI_LAST_AT_PLVL_SHIFT			24

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

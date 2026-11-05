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
 * Local power states, numbered after the State IDs of the SM8750 device
 * tree (one nibble per level, from the core up): CPU C4 0x00000004, cluster
 * CL5 0x01000054 and the system state 0x0200c354 (RSC 3, PDC 0xc). All are
 * standby states. The power-down states are only used by CPU_OFF.
 */
#define QTI_LOCAL_STATE_STB			0x4
#define QTI_LOCAL_STATE_CL5			0x5
#define QTI_LOCAL_STATE_RSC_RET			0x3
#define QTI_LOCAL_STATE_PDC_RET			0xc
#define QTI_LOCAL_STATE_OFF			0xd
#define QTI_LOCAL_STATE_DEEPOFF			0xe

#define PLAT_MAX_RET_STATE			QTI_LOCAL_STATE_PDC_RET
#define PLAT_MAX_OFF_STATE			QTI_LOCAL_STATE_DEEPOFF

/* The NCC resumes a core after its CPU_OFF WFI: plat_qti_cpu_off_wfi(). */
#define PLAT_QTI_CPU_OFF_WFI

/*
 * PDC, RSC, two Oryon clusters of six and two cores. MPIDR Aff2 is the
 * cluster and Aff1 the core; the core position strides by six per cluster.
 */
#define PLAT_CLUSTER_COUNT			2
#define PLAT_CLUSTER0_CORE_COUNT		6
#define PLAT_CLUSTER1_CORE_COUNT		2
#define PLAT_CORE_COUNT_PER_CLUSTER		6
#define PLATFORM_CORE_COUNT			8
#define PLAT_MAX_PWR_LVL			3

#define PLAT_TREE_DESC				\
{						\
	1, /* PDC */				\
	1, /* RSC */				\
	PLAT_CLUSTER_COUNT,			\
	PLAT_CLUSTER0_CORE_COUNT,		\
	PLAT_CLUSTER1_CORE_COUNT,		\
}

#define PLAT_CLUSTER_COUNT_IDX			(2U)
#define PLAT_CLUSTER_CORE_COUNT_START_IDX	(3U)

#endif /* QTI_PLATFORM_PM_H */

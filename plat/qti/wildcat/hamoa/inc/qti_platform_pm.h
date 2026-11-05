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
 * Local power states, numbered after the State IDs of the X1E80100 device
 * tree (one nibble per level, from the core up): CPU C4 0x00000004, cluster
 * CL4 0x01000044 and CL5 0x01000054, and the system state 0x0200c354
 * (RSC 3, PDC 0xc). All are standby states. The power-down states are only
 * used by CPU_OFF.
 */
#define QTI_LOCAL_STATE_STB			0x4
#define QTI_LOCAL_STATE_CL4			0x4
#define QTI_LOCAL_STATE_CL5			0x5
#define QTI_LOCAL_STATE_RSC_RET			0x3
#define QTI_LOCAL_STATE_PDC_RET			0xc
#define QTI_LOCAL_STATE_OFF			0xd
#define QTI_LOCAL_STATE_DEEPOFF			0xe

/* The device tree State IDs keep the last-at-level field in bits [27:24]. */
#define QTI_LAST_AT_PLVL_SHIFT			24

#define PLAT_MAX_RET_STATE			QTI_LOCAL_STATE_PDC_RET
#define PLAT_MAX_OFF_STATE			QTI_LOCAL_STATE_DEEPOFF

/*
 * PDC, RSC, then Oryon clusters of four cores: three on X1E80100, the first
 * two on X1P42100 (HAMOA_CLUSTER_COUNT). MPIDR Aff2 is the cluster and Aff1
 * the core.
 */
#define PLAT_CLUSTER_COUNT			HAMOA_CLUSTER_COUNT
#if (PLAT_CLUSTER_COUNT != 2) && (PLAT_CLUSTER_COUNT != 3)
#error "HAMOA_CLUSTER_COUNT must be 2 (X1P42100) or 3 (X1E80100)"
#endif
#define PLAT_CORE_COUNT_PER_CLUSTER		4
#define PLATFORM_CORE_COUNT			(PLAT_CLUSTER_COUNT * \
						 PLAT_CORE_COUNT_PER_CLUSTER)
#define PLAT_MAX_PWR_LVL			3

#if PLAT_CLUSTER_COUNT == 3
#define PLAT_TREE_DESC				\
{						\
	1, /* PDC */				\
	1, /* RSC */				\
	PLAT_CLUSTER_COUNT,			\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
}
#else
#define PLAT_TREE_DESC				\
{						\
	1, /* PDC */				\
	1, /* RSC */				\
	PLAT_CLUSTER_COUNT,			\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
	PLAT_CORE_COUNT_PER_CLUSTER,		\
}
#endif

#define PLAT_CLUSTER_COUNT_IDX			(2U)
#define PLAT_CLUSTER_CORE_COUNT_START_IDX	(3U)

#endif /* QTI_PLATFORM_PM_H */

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_HWIO_H
#define QTI_CPUCP_HWIO_H

#include <lib/utils_def.h>

/* Shared data and secure SCMI channel in the EPSS data memory. */
#define CPUCP_SHARED_DATA_BASE			(0x17d08000U)
#define CPUCP_IPC_SEC_BUF_BASE			(0x17d09000U)

/* APSS -> CPUCP doorbell. Writing the OSM_IPC bit raises the IRQ. */
#define APSS_SHARED_TZ_IPC_INTERRUPT_ADDR	(0x17400008U)
#define APSS_SHARED_TZ_IPC_INTERRUPT_OSM_IPC_SHFT	(28U)

#define EPSSTOP_MUC_CLK_CTRL			(0x17d98004U)
#define EPSSTOP_MUC_CLK_CTRL_CORE_CLK_EN	BIT_32(1)
#define EPSSTOP_GLOBAL_ENABLE			(0x17d98008U)

#endif /* QTI_CPUCP_HWIO_H */

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_HWIO_H
#define QTI_CPUCP_HWIO_H

/* Secure SCMI channel in the CPUCP data memory, 1 KB. */
#define CPUCP_IPC_SEC_BUF_BASE			(0x18b38000U)

/* APSS -> CPUCP doorbell. Writing the OSM_IPC bit raises the IRQ. */
#define APSS_SHARED_TZ_IPC_INTERRUPT_ADDR	(0x17440008U)
#define APSS_SHARED_TZ_IPC_INTERRUPT_OSM_IPC_SHFT	(28U)

/* CPUCP powers the cores, addressed by MPIDR. */
#define CPUCP_SCMI_PD_MPIDR

#endif /* QTI_CPUCP_HWIO_H */

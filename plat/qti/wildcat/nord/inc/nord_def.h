/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NORD_DEF_H
#define NORD_DEF_H

/* AOP command DB address space for mapping. */
#define QTI_AOP_CMD_DB_BASE			0x87148000
#define QTI_AOP_CMD_DB_SIZE			0x00020000

/* APSS HM registers. */
#define QTI_APSS_HM_BASE			0x17000000
#define QTI_APSS_HM_SIZE			0x01000000 /* TODO: confirm */

/* AOSS registers. */
#define QTI_AOSS_BASE				0x0b000000
#define QTI_AOSS_SIZE				0x04000000

/* CPU_OFF parks the core in BL31 (see plat_qti_cpu_park()). */
#define PLAT_QTI_CPU_PARK		1

/* No power down state for SYSTEM_SUSPEND (see plat_qti_pm_idle_states()). */
#define PLAT_QTI_SYSTEM_SUSPEND		0

/* Core C4 is a retention state (see plat_qti_cpu_retention_enter()). */
#define PLAT_QTI_CPU_RETENTION		1

#endif /* NORD_DEF_H */

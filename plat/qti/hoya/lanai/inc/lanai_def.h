/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef LANAI_DEF_H
#define LANAI_DEF_H

#include <hoya_def.h>

/*----------------------------------------------------------------------------*/
/* GIC-700 constants (8 redistributor frames, 0x40000 apart) */
/*----------------------------------------------------------------------------*/
#define QTI_GICD_BASE				0x17100000
#define QTI_GICR_BASE				0x17180000

/*----------------------------------------------------------------------------*/
/* UART related constants (QUPv3 wrapper 2 SE7, the QRD debug UART). */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x0089c000

/*----------------------------------------------------------------------------*/
/* Peripherals base addresses */
/*----------------------------------------------------------------------------*/
#define QTI_SEC_PRNG_BASE			0x010c0000

/*----------------------------------------------------------------------------*/
/* Device address space for mapping. Excluding starting 4K */
/*----------------------------------------------------------------------------*/
#define QTI_DEVICE_BASE				0x1000
#define QTI_DEVICE_SIZE				(0x1C000000 - QTI_DEVICE_BASE)

/*----------------------------------------------------------------------------*/
/* AOP CMD DB address space for mapping */
/*----------------------------------------------------------------------------*/
#define QTI_AOP_CMD_DB_BASE			0x81c60000
#define QTI_AOP_CMD_DB_SIZE			0x00020000

/*----------------------------------------------------------------------------*/
/* SMEM base address */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_BASE				0x81d00000
#define QTI_SMEM_SIZE				0x00200000

/*----------------------------------------------------------------------------*/
/*
 * PMIC arbiter v7 on bus 0 (the device tree core, chnls and cnfg regions):
 * APID map at core + 0x2000, read/write channels 4K apart, and the APID
 * ownership table. The APID count and TZ as EE 1 follow SM8750.
 */
/*----------------------------------------------------------------------------*/
#define QTI_SPMI_ARB_APID_MAP_BASE		0x0c402000U
#define QTI_SPMI_ARB_NUM_APID			896U
#define QTI_SPMI_ARB_CHNL_BASE			0x0c500000U
#define QTI_SPMI_ARB_CHNL_STRIDE		0x1000U
#define QTI_SPMI_ARB_OWNER_BASE			0x0c42d000U
#define QTI_SPMI_ARB_EE				1U

/*----------------------------------------------------------------------------*/
/* PMK8550 PON register offsets */
/*----------------------------------------------------------------------------*/
#define PON_PS_HOLD_RESET_CTL			0x852
#define PON_PS_HOLD_RESET_CTL2			0x853
/*
 * A PS_HOLD warm reset runs the system debug image first pass, which sets the
 * XBL download cookie, so XBL boots into RAM dump instead of rebooting (seen
 * on SM8750).
 */
#define QTI_PON_SYSTEM_RESET_TYPE		RESET_TYPE_HARD_RESET

/*----------------------------------------------------------------------------*/
/* QTIMER registers */
/*----------------------------------------------------------------------------*/
#define QTI_QTIMER_BASE				0x17420000

#endif /* LANAI_DEF_H */

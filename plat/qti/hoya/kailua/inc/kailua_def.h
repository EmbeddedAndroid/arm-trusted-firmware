/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef KAILUA_DEF_H
#define KAILUA_DEF_H

#include <hoya_def.h>

/*----------------------------------------------------------------------------*/
/* GIC-700 constants (8 redistributor frames, 0x40000 apart) */
/*----------------------------------------------------------------------------*/
#define QTI_GICD_BASE				0x17100000
#define QTI_GICR_BASE				0x17180000

/*----------------------------------------------------------------------------*/
/* UART related constants (QUPv3 wrapper 1 SE7, the HDK debug UART). */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x00a9c000

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
 * ownership table. Bus 0 has 768 peripherals; TZ is EE 1.
 */
/*----------------------------------------------------------------------------*/
#define QTI_SPMI_ARB_APID_MAP_BASE		0x0c402000U
#define QTI_SPMI_ARB_NUM_APID			768U
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
#define PON_PS_HOLD_SYSTEM_RESET_TYPE		PON_RESET_TYPE_HARD_RESET

/*----------------------------------------------------------------------------*/
/* APSS HM and AOSS registers */
/*----------------------------------------------------------------------------*/
#define QTI_APSS_HM_BASE			0x17000000
#define QTI_AOSS_BASE				0x0b000000

/*----------------------------------------------------------------------------*/
/* QTIMER registers */
/*----------------------------------------------------------------------------*/
#define QTI_QTIMER_BASE				0x17420000

#endif /* KAILUA_DEF_H */

/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2024, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCS615_DEF_H
#define QCS615_DEF_H

#include <hoya_def.h>

/*----------------------------------------------------------------------------*/
/* UART related constants. */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x880000

/*----------------------------------------------------------------------------*/
/* Peripherals base addresses */
/*----------------------------------------------------------------------------*/
#define QTI_SEC_PRNG_BASE			0x790000

/*----------------------------------------------------------------------------*/
/*
 * The two Kryo 4xx Gold cores (6 and 7) share the DSU and L3 with the silver
 * cores but sit on the APC1 rail, which XBL leaves collapsed.
 */
#define QTI_FIRST_GOLD_CORE			6
/* Gold (APC1) rail boot voltage, in mV. */
#define QTI_GOLD_RAIL_BOOT_MV			872U

/* APSS register space */
/*----------------------------------------------------------------------------*/
#define QTI_APSS_HM_BASE			0x17800000

/*----------------------------------------------------------------------------*/
/* QTIMER registers                                                           */
/*----------------------------------------------------------------------------*/
#define QTI_QTIMER_BASE				0x17C20000

/*----------------------------------------------------------------------------*/
/* AOSS registers                                                             */
/*----------------------------------------------------------------------------*/
#define QTI_AOSS_BASE				0x0b000000

/*----------------------------------------------------------------------------*/
/* AOP CMD DB  address space for mapping */
/*----------------------------------------------------------------------------*/
#define QTI_AOP_CMD_DB_BASE			0x85F20000
#define QTI_AOP_CMD_DB_SIZE			0x00020000
/*----------------------------------------------------------------------------*/
/* SMEM base address */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_BASE				0x86000000
#define QTI_SMEM_SIZE				0x00200000
/*----------------------------------------------------------------------------*/
/* LC PON register offsets */
/*----------------------------------------------------------------------------*/
#define PON_PS_HOLD_RESET_CTL			0x85a
#define PON_PS_HOLD_RESET_CTL2			0x85b

/*----------------------------------------------------------------------------*/
/* Chipset specific NOC error interrupt IDs.                                  */
/*----------------------------------------------------------------------------*/
#define PLAT_INT_ID_A1_NOC_ERROR		(0x18B)
#define PLAT_INT_ID_SYSTEM_NOC_ERROR		(0xC6)

#endif /* QCS615_DEF_H */

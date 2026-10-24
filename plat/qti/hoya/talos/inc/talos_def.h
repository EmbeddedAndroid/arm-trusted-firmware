/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TALOS_DEF_H
#define TALOS_DEF_H

/*
 * The two gold cores (6 and 7) share the DSU and L3 with the silver cores
 * but sit on the APC1 rail, which XBL leaves collapsed.
 */
#define QTI_FIRST_GOLD_CORE			6
#define QTI_GOLD_HAS_SEPARATE_DSU		0
/* Gold (APC1) rail boot voltage, in mV. */
#define QTI_GOLD_RAIL_BOOT_MV			872U

/*
 * Power-collapse sequences of the core IPM sequencers, run on the P-channel
 * OFF request a core raises at CPU_OFF: one for the Cortex-A55 cores, one for
 * the Cortex-A76 cores, which differs in its fifth command.
 */
#define QTI_CPU_OFF_SEQ_START			0x20U
#define QTI_CPU_OFF_SEQ \
{ \
	0xa1e402a7U, 0xe502a28dU, 0xa9e502a3U, 0xa6b0a5a4U, \
	0xaee302aaU, 0x028ee802U, 0xb7ac87e8U, 0xe802aee0U, \
	0xe2e8028eU, 0x0606068cU, 0xe402a706U, 0xe3028a97U, \
	0x8689818dU, 0xe5028390U, 0xa8e50282U, 0xa0e68802U, \
	0x80858403U, 0x00002087U, \
}
#define QTI_GOLD_CPU_OFF_SEQ_START		0x68U
#define QTI_GOLD_CPU_OFF_SEQ \
{ \
	0xa1e402a7U, 0xe502a2adU, 0xa9e502a3U, 0xa6b0a5a4U, \
	0xaee302aaU, 0x028ee802U, 0xb7ac87e8U, 0xe802aee0U, \
	0xe2e8028eU, 0x0606068cU, 0xe402a706U, 0xe3028a97U, \
	0x8689818dU, 0xe5028390U, 0xa8e50282U, 0xa0e68802U, \
	0x80858403U, 0x00002087U, \
}

#include <hoya_def.h>


/*----------------------------------------------------------------------------*/
/* UART related constants. */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x880000

/*----------------------------------------------------------------------------*/
/* Peripherals base addresses */
/*----------------------------------------------------------------------------*/
#define QTI_SEC_PRNG_BASE			0x790000
#define QTI_APSS_HM_BASE			0x17800000
#define QTI_QTIMER_BASE				0x17C20000
#define QTI_AOSS_BASE				0x0b000000

/*----------------------------------------------------------------------------*/
/* AOP CMD DB address space for mapping */
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
/* Chipset specific NOC error interrupt IDs. */
/*----------------------------------------------------------------------------*/
#define PLAT_INT_ID_A1_NOC_ERROR		(0x18B)
#define PLAT_INT_ID_SYSTEM_NOC_ERROR		(0xC6)

#endif /* TALOS_DEF_H */

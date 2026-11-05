/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HAMOA_DEF_H
#define HAMOA_DEF_H

#include <common_def.h>
#include <lib/utils_def.h>

#include <qti_board_def.h>
#include <qti_platform_pm.h>

/*----------------------------------------------------------------------------*/
/* Page tables and MMU */
/*----------------------------------------------------------------------------*/
#define MAX_MMAP_REGIONS			(PLAT_QTI_MMAP_ENTRIES)
#define MAX_XLAT_TABLES				12

#define PLAT_PHY_ADDR_SPACE_SIZE		(1ULL << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ULL << 36)

#define CACHE_WRITEBACK_SHIFT			6
#define CACHE_WRITEBACK_GRANULE			(1 << CACHE_WRITEBACK_SHIFT)

#define PLAT_PERCPU_BAKERY_LOCK_SIZE		(1 * CACHE_WRITEBACK_GRANULE)

/* Boot core: MPIDR Aff1 (core) and Aff2 (cluster) zero. */
#define MPIDR_PRIMARY_CPU			0x0000

/*----------------------------------------------------------------------------*/
/* GIC-700 */
/*----------------------------------------------------------------------------*/
#define QTI_GICD_BASE				0x17000000
#define QTI_GICR_BASE				0x17080000
/* No memory-mapped CPU interface: the crash report skips it. */
#define QTI_GICC_BASE				0x0

/*----------------------------------------------------------------------------*/
/* Debug UART (QUPv3 2 SE5, GENI), register spaces                           */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x00894000

#define GENI4_CFG				0x0
#define GENI4_IMAGE_REGS			0x100
#define GENI4_DATA				0x600

#define GENI_STATUS_REG				(GENI4_CFG + 0x00000040)
#define GENI_STATUS_M_GENI_CMD_ACTIVE_MASK	(0x1)
#define UART_TX_TRANS_LEN_REG			(GENI4_IMAGE_REGS + 0x00000170)
#define GENI_M_CMD0_REG				(GENI4_DATA + 0x00000000)
#define GENI_TX_FIFOn_REG			(GENI4_DATA + 0x00000100)

#define GENI_M_CMD_TX				(0x08000000)

/*----------------------------------------------------------------------------*/
/* Peripherals */
/*----------------------------------------------------------------------------*/
#define QTI_SEC_PRNG_BASE			0x010c0000

/* Device address space for mapping, excluding the first 4 KB. */
#define QTI_DEVICE_BASE				0x1000
#define QTI_DEVICE_SIZE				(0x1C000000 - QTI_DEVICE_BASE)

#define QTI_SMEM_BASE				0xffe00000
#define QTI_SMEM_SIZE				0x00200000

/* PMIC arbiter v7; bus 0, with the PMK8550 PON, has the first 896 channels. */
#define QTI_PMIC_ARB_BASE			0x0B000000UL
#define SPMI_ARB_NUM_CHANNELS			896U

#define QTI_APSS_HM_BASE			0x17000000

#define QTI_PS_HOLD_REG				0x0C264000

#define QTI_QTIMER_BASE				0x17800000

/* NCC_ARCH block of cluster 0; the others follow one stride apart. */
#define PLAT_NCC_ARCH_BASE			0x19880000U
#define PLAT_NCC_CLUSTER_STRIDE			0x1000000U

/*----------------------------------------------------------------------------*/
/* Secure interrupts */
/*----------------------------------------------------------------------------*/
#define PLAT_INT_ID_RESET_SGI			(0xf)
#define PLAT_INT_ID_CPU_WAKEUP_SGI		(0x8)

#endif /* HAMOA_DEF_H */

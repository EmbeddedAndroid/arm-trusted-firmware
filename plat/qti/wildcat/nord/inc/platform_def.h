/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <common_def.h>
#include <lib/utils_def.h>

#include <nord_def.h>
#include <qti_board_def.h>
#include <qti_platform_pm.h>

/*----------------------------------------------------------------------------*/
/* Page tables and MMU */
/*----------------------------------------------------------------------------*/
#define MAX_MMAP_REGIONS			(PLAT_QTI_MMAP_ENTRIES)
#define MAX_XLAT_TABLES				12

#define PLAT_PHY_ADDR_SPACE_SIZE		(1ULL << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ULL << 36)

#define ARM_CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE			(1 << ARM_CACHE_WRITEBACK_SHIFT)

#define PLAT_PERCPU_BAKERY_LOCK_SIZE		(1 * CACHE_WRITEBACK_GRANULE)

/* Boot core: MPIDR Aff1 (core) and Aff2 (cluster) zero. */
#define MPIDR_PRIMARY_CPU			0x0000

#define MAX_IO_HANDLES				2
#define MAX_IO_DEVICES				2
#define MAX_IO_BLOCK_DEVICES			U(1)

/*----------------------------------------------------------------------------*/
/* GIC-700 */
/*----------------------------------------------------------------------------*/
#define QTI_GICD_BASE				0x17000000
#define QTI_GICR_BASE				0x17080000
/* No memory-mapped CPU interface: the crash report skips it. */
#define QTI_GICC_BASE				0x0

/*----------------------------------------------------------------------------*/
/* Debug UART (QUPv3 2 SE1, GENI), register spaces                           */
/*----------------------------------------------------------------------------*/
#define UART_BASE_ADDR				0x00884000
#define PLAT_QTI_UART_BASE			UART_BASE_ADDR

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

/* XBL places the SMEM table of contents in the last 4 KB of the 4 MB. */
#define QTI_SMEM_BASE				0x89b00000
#define QTI_SMEM_SIZE				0x00400000

/* PMIC arbiter v7. */
#define QTI_PMIC_ARB_BASE			0x0B000000UL
#define SPMI_ARB_NUM_CHANNELS			1024U

#define QTI_PS_HOLD_REG				0x0C264000

#define QTI_CORE_TOP_CSR_BASE			0x01f00000

/* NCC_ARCH block of cluster 0; the others follow one stride apart. */
#define PLAT_NCC_ARCH_BASE			0x19880000U
#define PLAT_NCC_CLUSTER_STRIDE			0x1000000U

/*----------------------------------------------------------------------------*/
/* EL3 image memory map                                                       */
/*----------------------------------------------------------------------------*/
/*
 * BL2 is the reset entry image (RESET_TO_BL2=1) that XBL loads from the tz
 * partition and enters at 0x14694000 in system IMEM.
 *
 * XBL loads the FIP as the APPSBL (uefi partition) where the stock UEFI
 * image lives, 0xd8d00000, and BL2 reads it from there. The FIP holds BL31,
 * BL32 (OP-TEE) and BL33.
 *
 * BL31 and BL32 take the QTEE region of the DARE-TZ carve-out
 * (0xbc180000, 6.5 MB): BL31 the first 1 MB, OP-TEE the remaining 5.5 MB.
 * BL33 runs from non-secure DRAM below the carve-out.
 */
#define BL2_BASE				0x14694000
#define BL2_SIZE				0x40000
#define BL2_LIMIT				(BL2_BASE + BL2_SIZE)

#define PLAT_QTI_FIP_IOBASE			0xd8d00000
#define PLAT_QTI_FIP_MAXSIZE			0x01000000

#define BL31_BASE				0xbc180000
#define BL31_SIZE				0x00100000
#define BL31_LIMIT				(BL31_BASE + BL31_SIZE)

#define BL32_BASE				0xbc280000
#define BL32_SIZE				0x00580000
#define BL32_LIMIT				(BL32_BASE + BL32_SIZE)

#define BL33_BASE				0xa0200000
#define BL33_SIZE				0x00a00000

#endif /* PLATFORM_DEF_H */

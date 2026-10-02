/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <qcs615_def.h>

/* Device space ends where pIMEM, which holds BL2 and BL31, starts. */
#define QTI_DEVICE_BASE			0x1000
#define QTI_DEVICE_SIZE			(0x1C000000 - QTI_DEVICE_BASE)

#define MAX_IO_HANDLES			2
#define MAX_IO_DEVICES			2
#define MAX_IO_BLOCK_DEVICES		U(1)

/*
 * XBL enters the TZ image in the system IMEM window that QSEE uses; a TZ entry
 * point in pIMEM resets the SoC before the first instruction runs.
 */
#define BL2_BASE			0x14680000
#define BL2_SIZE			0x00019000
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

#define BL31_BASE			0x1c200000
#define BL31_SIZE			0x00100000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

/* OP-TEE runs from the DDR carve-out the QCS615 memory map gives TZ Apps. */
#define BL32_BASE			0x87a00000
#define BL32_SIZE			0x00200000
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

#define BL33_BASE			0x9f800000
#define BL33_SIZE			0x00400000

/* XBL loads the uefi partition image (the FIP in an ELF) here. */
#define PLAT_QTI_FIP_IOBASE		0x9fc00000
#define PLAT_QTI_FIP_MAXSIZE		0x00400000

#endif /* PLATFORM_DEF_H */

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <hamoa_def.h>

#define MAX_IO_HANDLES			2
#define MAX_IO_DEVICES			2
#define MAX_IO_BLOCK_DEVICES		U(1)

/*
 * X1E80100 EL3 image memory map.
 *
 * BL2 is the reset entry image (RESET_TO_BL2=1) that XBL loads from the tz
 * partition and enters at 0x14680000 in system IMEM. XBL accepts tz segments
 * only inside its IMEM TZ window (0x14680000, 100 KB) or in DDR, so BL2,
 * including its bss and translation tables, must fit that window.
 *
 * BL31 sits in the qtee carve-out of the X1E80100 device tree (0xd80e0000 +
 * 0x520000): 1 MB at 0xd8100000. BL32 (OP-TEE, 30 MB with its TA RAM) does
 * not fit in what is left there and takes the start of the trusted
 * application region the same device tree reserves (ta, 0xd8600000 +
 * 0x8a00000).
 *
 * XBL loads the FIP as the APPSBL (uefi partition, 7 MB) at 0xa7000000 and
 * BL2 reads it from there. BL33 runs from non-secure DRAM above the FIP
 * window.
 */
#define BL2_BASE			0x14680000
#define BL2_SIZE			0x19000
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

#define PLAT_QTI_FIP_IOBASE		0xa7000000
#define PLAT_QTI_FIP_MAXSIZE		0x00700000

#define BL31_BASE			0xd8100000
#define BL31_SIZE			0x00100000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#define BL32_BASE			0xd8600000
#define BL32_SIZE			0x01e00000
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

#define BL33_BASE			0xa7800000
#define BL33_SIZE			0x00800000

#endif /* PLATFORM_DEF_H */

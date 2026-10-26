/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <kailua_def.h>

#define MAX_IO_HANDLES			2
#define MAX_IO_DEVICES			2
#define MAX_IO_BLOCK_DEVICES		U(1)

/*
 * SM8550 HDK EL3 image memory map.
 *
 * BL2 is the reset entry image (RESET_TO_BL2=1) that XBL loads and enters as
 * the TZ image. XBL loads TZ image segments only into its TZ windows and
 * stops the boot on any other address, so BL2, including its bss and
 * translation tables, must fit there. BL2 takes the TZ region at the start of
 * the system IMEM, 0x14680000 + 0x19000; that XBL's IMEM window matches it is
 * not yet confirmed from the stock TZ image.
 *
 * BL31 takes the secure OS slot of the TZ carve-out (0xd8300000 + 5 MB, the
 * qtee region of the SM8550 device tree): XBL_SC data and the CPUCP firmware
 * sit below it, at 0xd8100000 and 0xd8140000. BL32 (OP-TEE, 30 MB with its
 * TA RAM) runs from the TA area above it, at 0xd8800000.
 *
 * XBL loads the FIP as the APPSBL (uefi partition) at 0xa7000000, the stock
 * UEFI load address, and BL2 reads it from there. BL33 runs from non-secure
 * DRAM above the FIP window.
 */
#define BL2_BASE			0x14680000
#define BL2_SIZE			0x19000
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

#define PLAT_QTI_FIP_IOBASE		0xa7000000
#define PLAT_QTI_FIP_MAXSIZE		0x00500000

#define BL31_BASE			0xd8300000
#define BL31_SIZE			0x00100000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#define BL32_BASE			0xd8800000
#define BL32_SIZE			0x01e00000
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

#define BL33_BASE			0xa7800000
#define BL33_SIZE			0x00800000

#endif /* PLATFORM_DEF_H */

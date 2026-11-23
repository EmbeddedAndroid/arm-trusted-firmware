/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <eliza_def.h>

#define MAX_IO_HANDLES			2
#define MAX_IO_DEVICES			2
#define MAX_IO_BLOCK_DEVICES		U(1)

/*
 * SM7750 MTP EL3 image memory map.
 *
 * BL2 is the reset entry image (RESET_TO_BL2=1) that XBL loads from the tz
 * partition and enters at 0x14693000 in system IMEM. XBL rejects a tz
 * segment at the start of the DARE-TZ QTEE slot (0xd80e0000), so BL2,
 * including its bss and translation tables, must fit the IMEM TZ window
 * (0x14693000, 80 KB).
 *
 * BL31 and BL32 take the place of QTEE in the DARE-TZ region (0xd8000000 +
 * 0x3800000), above the XBL_SC data XBL keeps at its base: BL31 1 MB at
 * 0xd8100000, BL32 (OP-TEE, 30 MB with its TA RAM) from 0xd8200000, running
 * on into the DARE-TZ TA region at 0xd8600000. BL31 restricts the QTEE slot
 * up to the end of BL32 (0xd80e0000-0xda000000) to the secure apps domain at
 * the LLCC MPU, with a resource group XBL leaves free.
 *
 * XBL loads the FIP as the APPSBL (uefi partition) at 0xa7000000 and BL2
 * reads it from there. BL33 runs from non-secure
 * DRAM above the FIP window.
 */
#define BL2_BASE			0x14693000
#define BL2_SIZE			0x14000
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

#define PLAT_QTI_FIP_IOBASE		0xa7000000
#define PLAT_QTI_FIP_MAXSIZE		0x00500000

#define BL31_BASE			0xd8100000
#define BL31_SIZE			0x00100000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#define BL32_BASE			0xd8200000
#define BL32_SIZE			0x01e00000
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

#define QTI_LLCC_MPU_TZ_RG		65U
#define QTI_LLCC_MPU_TZ_BASE		0xd80e0000
#define QTI_LLCC_MPU_TZ_LIMIT		BL32_LIMIT

#define BL33_BASE			0xa7800000
#define BL33_SIZE			0x00800000

#endif /* PLATFORM_DEF_H */

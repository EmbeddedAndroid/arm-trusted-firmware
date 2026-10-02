/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2024, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <qcs615_def.h>

/*----------------------------------------------------------------------------*/
/* Device address space for mapping. Excluding starting 4K */
/*----------------------------------------------------------------------------*/
#define QTI_DEVICE_BASE				0x1000
#define QTI_DEVICE_SIZE				(0x80000000 - QTI_DEVICE_BASE)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at DDR as per memory map. BL31_BASE is calculated using the
 * current BL31 debug size plus a little space for growth.
 */
#define BL31_BASE				0x86200000
#define BL31_SIZE				0x00100000
#define BL31_LIMIT				(BL31_BASE + BL31_SIZE)

#endif /* PLATFORM_DEF_H */

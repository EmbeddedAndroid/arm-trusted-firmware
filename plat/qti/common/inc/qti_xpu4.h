/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_XPU4_H
#define QTI_XPU4_H

#include <lib/utils_def.h>

/* XPU4 (MPU) registers; resource group n */
#define XPU4_IDR0				0x0U
#define XPU4_IDR0_NRG(v)			(((v) >> 16) & 0x3ffU)
#define XPU4_RGCR1(n)				(0x1004U + (0x40U * (n)))
#define XPU4_RGCSAR1(n)				(0x1008U + (0x40U * (n)))
#define XPU4_RGCSAR0(n)				(0x100cU + (0x40U * (n)))
#define XPU4_RGCEAR1(n)				(0x1010U + (0x40U * (n)))
#define XPU4_RGCEAR0(n)				(0x1014U + (0x40U * (n)))
#define XPU4_RGRDR(n)				(0x1018U + (0x40U * (n)))
#define XPU4_RGWRR(n)				(0x101cU + (0x40U * (n)))
#define XPU4_RGCR1_RGE				BIT_32(0)
/* Number of resource groups inside a mapping of size bytes from the base */
#define XPU4_NRG_MAPPED(size)			(((size) - 0x1000U) / 0x40U)

/* Access domains in RGRDR/RGWRR */
#define XPU4_QAD_APPS				BIT_32(0)
#define XPU4_QAD_NS				BIT_32(30)

#endif /* QTI_XPU4_H */

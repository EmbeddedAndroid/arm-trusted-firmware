/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SMMU_CFG_H
#define SMMU_CFG_H

#include <drivers/qti/smmu/smmu.h>

/* Apps SMMU */
#define APPS_SMMU_SACR				0x15000010U

/* SACR with CACHE_LOCK clear, so that the OS's ACTLR writes take effect */
#define SMMU_SACR_VAL				0x00000004U

/* GPU SMMU (Adreno SMMU-500) */
#define GPU_SMMU_BASE				0x03da0000U
#define GPU_SMMU_SACR				(GPU_SMMU_BASE + 0x0010U)
#define GPU_SMMU_CB0				0x03dc0000U
#define GPU_REG_OFFSET_MASK			0xfffffU

/* GPU SMMU aperture, in GPU CX */
#define GPU_SMMU_APERTURE_GC			0x03d60000U
#define GPU_SMMU_APERTURE_LPAC			0x03d60004U
#define GPU_SMMU_APERTURE_CTRL			0x03d60008U
#define GPU_SMMU_APERTURE_ECP_SNOOP		0x03d6000cU
#define GPU_SMMU_APERTURE_GMU_FW		0x03d60010U
#define GPU_SMMU_APERTURE_CTRL_EN		0x00000002U

#define GCC_GPU_CFG_AHB_CBCR			0x00171004U

#define GPU_CC_BASE				0x03d90000U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX		(GPU_CC_BASE + 0x4004U)
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK		(GPU_CC_BASE + 0x5000U)
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS		(GPU_CC_BASE + 0x5004U)
#define GPU_CC_CX_GDSCR				(GPU_CC_BASE + 0x9110U)
#define GPU_CC_CX_GMU_CBCR			(GPU_CC_BASE + 0x916cU)
#define GPU_CC_HUB_CX_INT_CBCR			(GPU_CC_BASE + 0x9180U)

#endif /* SMMU_CFG_H */

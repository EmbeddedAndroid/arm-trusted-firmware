/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/gpu_smmu/gpu_smmu.h>
#include <lib/utils_def.h>

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U

#define GPU_CC_BASE			0x05090000U
#define GPU_CC_CX_GDSCR			(GPU_CC_BASE + 0x106cU)
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	(GPU_CC_BASE + 0x3000U)
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	(GPU_CC_BASE + 0x3004U)

/* Context bank pages of the GPU SMMU at 0x050a0000, as offsets from 0x05000000 */
#define GPU_SMMU_APERTURE_S1CB0		0x05060000U
#define GPU_SMMU_APERTURE_S1CB1		0x05060004U
#define GPU_SMMU_APERTURE_CTL		0x05060008U
#define GPU_SMMU_CB0_OFFSET		0x000a8000U
#define GPU_SMMU_CB1_OFFSET		0x000a9000U

static const struct qti_gpu_smmu_reg gpu_smmu_regs[] = {
	{ GPU_SMMU_APERTURE_S1CB0, GPU_SMMU_CB0_OFFSET, true },
	{ GPU_SMMU_APERTURE_S1CB1, GPU_SMMU_CB1_OFFSET, true },
	{ GPU_SMMU_APERTURE_CTL, 0U, true },
};

const struct qti_gpu_smmu_cfg qti_gpu_smmu_cfg = {
	.cfg_ahb_cbcr = GCC_GPU_CFG_AHB_CBCR,
	.tz_vote_gds = GPU_CC_TZ_VOTE_GPU_SMMU_GDS,
	.tz_vote_clk = GPU_CC_TZ_VOTE_GPU_SMMU_CLK,
	.cx_gdscr = GPU_CC_CX_GDSCR,
	.cx_cbcrs = NULL,
	.num_cx_cbcrs = 0U,
	.regs = gpu_smmu_regs,
	.num_regs = ARRAY_SIZE(gpu_smmu_regs),
};

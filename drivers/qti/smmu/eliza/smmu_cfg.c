/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/smmu/eliza/smmu_cfg.h>
#include <lib/utils_def.h>

struct smmu_cfg_s qti_smmu_cfg[] = {
	{APPS_SMMU_SACR, SMMU_SACR_VAL},
};

const size_t qti_smmu_cfg_count = ARRAY_SIZE(qti_smmu_cfg);

static const uintptr_t gpu_cx_cbcrs[] = {
	GPU_CC_CX_GMU_CBCR,
	GPU_CC_HUB_CX_INT_CBCR,
};

static const struct smmu_cfg_s gpu_regs[] = {
	{GPU_SMMU_SACR, SMMU_SACR_VAL},
};

static const uintptr_t gpu_aperture[] = {
	GPU_SMMU_APERTURE_GC,
	GPU_SMMU_APERTURE_LPAC,
	GPU_SMMU_APERTURE_ECP_SNOOP,
	GPU_SMMU_APERTURE_GMU_FW,
};

const struct qti_smmu_gpu_cx qti_smmu_gpu_cx = {
	.cfg_ahb_cbcr = GCC_GPU_CFG_AHB_CBCR,
	.tz_vote_gds = GPU_CC_TZ_VOTE_GPU_SMMU_GDS,
	.tz_vote_clk = GPU_CC_TZ_VOTE_GPU_SMMU_CLK,
	.tz_gdsc_ctrl = GPU_CC_TZ_GDSC_CTRL_GPU_CX,
	.cx_gdscr = GPU_CC_CX_GDSCR,
	.cx_cbcrs = gpu_cx_cbcrs,
	.num_cx_cbcrs = ARRAY_SIZE(gpu_cx_cbcrs),
	.regs = gpu_regs,
	.num_regs = ARRAY_SIZE(gpu_regs),
	.aperture = gpu_aperture,
	.num_aperture = ARRAY_SIZE(gpu_aperture),
	.aperture_ctrl = GPU_SMMU_APERTURE_CTRL,
	.aperture_ctrl_en = GPU_SMMU_APERTURE_CTRL_EN,
	.cb0_offset = GPU_SMMU_CB0 & GPU_REG_OFFSET_MASK,
	/* GPU SMMU IDR1.NUMCB */
	.num_cbs = 25U,
};

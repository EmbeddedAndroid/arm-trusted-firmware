/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/qti/gpu_smmu/gpu_smmu.h>
#include <drivers/qti/smmu/kodiak/smmu_cfg.h>
#include <lib/utils_def.h>

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U

#define GPU_CC_BASE			0x03d90000U
#define GPU_CC_CX_GDSCR			(GPU_CC_BASE + 0x106cU)
#define GPU_CC_CX_GMU_CBCR		(GPU_CC_BASE + 0x1098U)
#define GPU_CC_HUB_CX_INT_CBCR		(GPU_CC_BASE + 0x1204U)
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	(GPU_CC_BASE + 0x3000U)
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	(GPU_CC_BASE + 0x3004U)

/* The GPU SMMU registers also need the CX GMU and hub interface clocks. */
static const uintptr_t gpu_cx_cbcrs[] = {
	GPU_CC_CX_GMU_CBCR,
	GPU_CC_HUB_CX_INT_CBCR,
};

static const struct qti_gpu_smmu_reg gpu_smmu_regs[] = {
	{ GPU_GFX_SMMU_CUSTOM_CFG_ADDR, GPU_GFX_SMMU_CUSTOM_CFG, false },
	{ GPU_GFX_SMMU_CUSTOM_CFG_SEC_ADDR, GPU_GFX_SMMU_CUSTOM_CFG_SEC, false },
	{ GPU_GFX_SMMU_ACR_ADDR, GPU_GFX_SMMU_ACR, false },
	{ GPU_GFX_SMMU_NSACR_ADDR, GPU_GFX_SMMU_NSACR, false },
	{ GPU_GFX_SMMU_SPARE_REG_1_SEC_ADDR, GPU_GFX_SMMU_SPARE_REG_1_SEC,
	  false },
	{ GPU_GFX_SMMU_CLOCK_GATING_SEC_ADDR, GPU_GFX_SMMU_CLOCK_GATING_SEC,
	  false },
	{ GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_0_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_0_SEC, false },
	{ GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_1_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_1_SEC, false },
	{ GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_0_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_0_SEC, false },
	{ GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_1_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_1_SEC, false },
	{ GPU_GFX_SMMU_TLBIS_CTRL_ADDR, GPU_GFX_SMMU_TLBIS_CTRL, false },
	{ GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_0_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_0_SEC, false },
	{ GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_1_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_1_SEC, false },
	/* Context banks 0, 1 and 2 at aperture slots 0, 1 and 6. */
	{ SMMU_GPU_AHB_4K_APERTURE_S1CB0_HWIO_ADDR,
	  SMMU_GPU_AHB_4K_APERTURE_S1CB0_REG_ADDR & GPU_REG_OFFSET_MASK, true },
	{ SMMU_GPU_AHB_4K_APERTURE_S1CB1_HWIO_ADDR,
	  SMMU_GPU_AHB_4K_APERTURE_S1CB1_REG_ADDR & GPU_REG_OFFSET_MASK, true },
	{ SMMU_GPU_AHB_4K_APERTURE_S1CB6_HWIO_ADDR,
	  SMMU_GPU_AHB_4K_APERTURE_S1CB2_REG_ADDR & GPU_REG_OFFSET_MASK, true },
	{ SMMU_GPU_AHB_4K_APERTURE_CTL_HWIO_ADDR,
	  SMMU_GPU_AHB_4K_APERTURE_CTL_REG_ADDR, true },
};

const struct qti_gpu_smmu_cfg qti_gpu_smmu_cfg = {
	.cfg_ahb_cbcr = GCC_GPU_CFG_AHB_CBCR,
	.tz_vote_gds = GPU_CC_TZ_VOTE_GPU_SMMU_GDS,
	.tz_vote_clk = GPU_CC_TZ_VOTE_GPU_SMMU_CLK,
	.cx_gdscr = GPU_CC_CX_GDSCR,
	.cx_cbcrs = gpu_cx_cbcrs,
	.num_cx_cbcrs = ARRAY_SIZE(gpu_cx_cbcrs),
	.regs = gpu_smmu_regs,
	.num_regs = ARRAY_SIZE(gpu_smmu_regs),
};

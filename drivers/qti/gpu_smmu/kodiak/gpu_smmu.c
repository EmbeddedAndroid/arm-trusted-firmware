/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/gpu_smmu/gpu_smmu.h>
#include <drivers/qti/smmu/smmu.h>
#include <drivers/qti/smmu/kodiak/smmu_cfg.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U
#define CBCR_CLK_ENABLE			BIT_32(0)
#define CBCR_CLK_OFF			BIT_32(31)

#define GPU_CC_BASE			0x03d90000U
#define GPU_CC_CX_GDSCR			(GPU_CC_BASE + 0x106cU)
#define GDSCR_RETAIN_FF_ENABLE		BIT_32(11)
#define GPU_CC_CX_GMU_CBCR		(GPU_CC_BASE + 0x1098U)
#define GPU_CC_HUB_CX_INT_CBCR		(GPU_CC_BASE + 0x1204U)
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	(GPU_CC_BASE + 0x3000U)
#define TZ_VOTE_CLK_ENABLE		BIT_32(0)
#define TZ_VOTE_CLK_OFF			BIT_32(31)
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	(GPU_CC_BASE + 0x3004U)
#define TZ_VOTE_GDS_SW_COLLAPSE		BIT_32(0)
#define TZ_VOTE_GDS_PWR_ON		BIT_32(31)

#define POLL_US				1000U

static const struct smmu_cfg_s gpu_smmu_cfg[] = {
	{ GPU_GFX_SMMU_CUSTOM_CFG_ADDR, GPU_GFX_SMMU_CUSTOM_CFG },
	{ GPU_GFX_SMMU_CUSTOM_CFG_SEC_ADDR, GPU_GFX_SMMU_CUSTOM_CFG_SEC },
	{ GPU_GFX_SMMU_ACR_ADDR, GPU_GFX_SMMU_ACR },
	{ GPU_GFX_SMMU_NSACR_ADDR, GPU_GFX_SMMU_NSACR },
	{ GPU_GFX_SMMU_SPARE_REG_1_SEC_ADDR, GPU_GFX_SMMU_SPARE_REG_1_SEC },
	{ GPU_GFX_SMMU_CLOCK_GATING_SEC_ADDR, GPU_GFX_SMMU_CLOCK_GATING_SEC },
	{ GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_0_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_0_SEC },
	{ GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_1_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_1_SEC },
	{ GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_0_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_0_SEC },
	{ GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_1_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_1_SEC },
	{ GPU_GFX_SMMU_TLBIS_CTRL_ADDR, GPU_GFX_SMMU_TLBIS_CTRL },
	{ GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_0_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_0_SEC },
	{ GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_1_SEC_ADDR,
	  GPU_GFX_SMMU_CLIENT_DBG_WUSER_GFX_1_SEC },
	{ SMMU_GPU_AHB_4K_APERTURE_S1CB0_HWIO_ADDR,
	  SMMU_GPU_AHB_4K_APERTURE_S1CB0_REG_ADDR & GPU_REG_OFFSET_MASK },
	{ SMMU_GPU_AHB_4K_APERTURE_S1CB1_HWIO_ADDR,
	  SMMU_GPU_AHB_4K_APERTURE_S1CB1_REG_ADDR & GPU_REG_OFFSET_MASK },
	{ SMMU_GPU_AHB_4K_APERTURE_S1CB6_HWIO_ADDR,
	  SMMU_GPU_AHB_4K_APERTURE_S1CB2_REG_ADDR & GPU_REG_OFFSET_MASK },
	{ SMMU_GPU_AHB_4K_APERTURE_CTL_HWIO_ADDR,
	  SMMU_GPU_AHB_4K_APERTURE_CTL_REG_ADDR },
};

static bool poll(uintptr_t addr, uint32_t mask, uint32_t want)
{
	for (uint32_t i = 0U; i < POLL_US; i++) {
		if ((mmio_read_32(addr) & mask) == want) {
			return true;
		}
		udelay(1U);
	}

	return false;
}

static bool clk_on(uintptr_t cbcr)
{
	mmio_setbits_32(cbcr, CBCR_CLK_ENABLE);

	return poll(cbcr, CBCR_CLK_OFF, 0U);
}

/*
 * The boot firmware leaves the GPU CX domain collapsed and its clocks off,
 * and writes to it are then lost. Vote CX and the GPU SMMU clock on, run the
 * CX interface clocks, keep the CX registers across collapse (the OS does not
 * set retention on this GDSC, and the aperture is lost without it), program
 * the GPU SMMU, and drop the votes.
 */
void qti_gpu_smmu_init(void)
{
	if (!clk_on(GCC_GPU_CFG_AHB_CBCR)) {
		ERROR("GPU SMMU: GPU config AHB clock stays off\n");
		return;
	}

	mmio_clrbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_GDS, TZ_VOTE_GDS_SW_COLLAPSE);
	if (!poll(GPU_CC_TZ_VOTE_GPU_SMMU_GDS, TZ_VOTE_GDS_PWR_ON,
		  TZ_VOTE_GDS_PWR_ON)) {
		ERROR("GPU SMMU: GPU CX stays off\n");
		goto gds_off;
	}

	mmio_setbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_CLK, TZ_VOTE_CLK_ENABLE);
	if (!poll(GPU_CC_TZ_VOTE_GPU_SMMU_CLK, TZ_VOTE_CLK_OFF, 0U) ||
	    !clk_on(GPU_CC_CX_GMU_CBCR) || !clk_on(GPU_CC_HUB_CX_INT_CBCR)) {
		ERROR("GPU SMMU: GPU CX clocks stay off\n");
		goto clk_off;
	}

	mmio_setbits_32(GPU_CC_CX_GDSCR, GDSCR_RETAIN_FF_ENABLE);
	for (size_t i = 0U; i < ARRAY_SIZE(gpu_smmu_cfg); i++) {
		mmio_write_32(gpu_smmu_cfg[i].addr, gpu_smmu_cfg[i].value);
	}
	VERBOSE("GPU SMMU aperture 0x%x 0x%x 0x%x ctl 0x%x\n",
		mmio_read_32(SMMU_GPU_AHB_4K_APERTURE_S1CB0_HWIO_ADDR),
		mmio_read_32(SMMU_GPU_AHB_4K_APERTURE_S1CB1_HWIO_ADDR),
		mmio_read_32(SMMU_GPU_AHB_4K_APERTURE_S1CB6_HWIO_ADDR),
		mmio_read_32(SMMU_GPU_AHB_4K_APERTURE_CTL_HWIO_ADDR));

clk_off:
	mmio_clrbits_32(GPU_CC_HUB_CX_INT_CBCR, CBCR_CLK_ENABLE);
	mmio_clrbits_32(GPU_CC_CX_GMU_CBCR, CBCR_CLK_ENABLE);
	mmio_clrbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_CLK, TZ_VOTE_CLK_ENABLE);
gds_off:
	mmio_setbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_GDS, TZ_VOTE_GDS_SW_COLLAPSE);
}

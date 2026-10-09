/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/gpu_smmu/gpu_smmu.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U
#define CBCR_CLK_ENABLE			BIT_32(0)
#define CBCR_CLK_OFF			BIT_32(31)

#define GPU_CC_BASE			0x05090000U
#define GPU_CC_CX_GDSCR			(GPU_CC_BASE + 0x106cU)
#define GDSCR_RETAIN_FF_ENABLE		BIT_32(11)
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	(GPU_CC_BASE + 0x3000U)
#define TZ_VOTE_CLK_ENABLE		BIT_32(0)
#define TZ_VOTE_CLK_OFF			BIT_32(31)
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	(GPU_CC_BASE + 0x3004U)
#define TZ_VOTE_GDS_SW_COLLAPSE		BIT_32(0)
#define TZ_VOTE_GDS_PWR_ON		BIT_32(31)

/* Context bank pages of the GPU SMMU at 0x050a0000, as offsets from 0x05000000 */
#define GPU_SMMU_APERTURE_S1CB0		0x05060000U
#define GPU_SMMU_APERTURE_S1CB1		0x05060004U
#define GPU_SMMU_APERTURE_CTL		0x05060008U
#define GPU_SMMU_CB0_OFFSET		0x000a8000U
#define GPU_SMMU_CB1_OFFSET		0x000a9000U

#define POLL_US				1000U

static bool poll(uintptr_t addr, uint32_t mask, uint32_t want)
{
	uint32_t i;

	for (i = 0U; i < POLL_US; i++) {
		if ((mmio_read_32(addr) & mask) == want) {
			return true;
		}
		udelay(1U);
	}

	return false;
}

/*
 * The aperture is in the GPU CX domain. Vote CX on and the GPU SMMU clock
 * for secure, keep the CX registers across collapse (the OS does not set
 * retention on this GDSC), program context banks 0 and 1, and drop the
 * votes.
 */
void qti_gpu_smmu_init(void)
{
	mmio_setbits_32(GCC_GPU_CFG_AHB_CBCR, CBCR_CLK_ENABLE);
	if (!poll(GCC_GPU_CFG_AHB_CBCR, CBCR_CLK_OFF, 0U)) {
		ERROR("GPU SMMU: GPU config AHB clock stays off\n");
		return;
	}

	mmio_clrbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_GDS, TZ_VOTE_GDS_SW_COLLAPSE);
	if (!poll(GPU_CC_TZ_VOTE_GPU_SMMU_GDS, TZ_VOTE_GDS_PWR_ON, TZ_VOTE_GDS_PWR_ON)) {
		ERROR("GPU SMMU: GPU CX stays off\n");
		goto gds_off;
	}
	mmio_setbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_CLK, TZ_VOTE_CLK_ENABLE);
	if (!poll(GPU_CC_TZ_VOTE_GPU_SMMU_CLK, TZ_VOTE_CLK_OFF, 0U)) {
		ERROR("GPU SMMU: GPU SMMU clock stays off\n");
		goto clk_off;
	}

	mmio_setbits_32(GPU_CC_CX_GDSCR, GDSCR_RETAIN_FF_ENABLE);
	mmio_write_32(GPU_SMMU_APERTURE_S1CB0, GPU_SMMU_CB0_OFFSET);
	mmio_write_32(GPU_SMMU_APERTURE_S1CB1, GPU_SMMU_CB1_OFFSET);
	mmio_write_32(GPU_SMMU_APERTURE_CTL, 0U);
	VERBOSE("GPU SMMU aperture 0x%x 0x%x ctl 0x%x\n",
		mmio_read_32(GPU_SMMU_APERTURE_S1CB0),
		mmio_read_32(GPU_SMMU_APERTURE_S1CB1),
		mmio_read_32(GPU_SMMU_APERTURE_CTL));

clk_off:
	mmio_clrbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_CLK, TZ_VOTE_CLK_ENABLE);
gds_off:
	mmio_setbits_32(GPU_CC_TZ_VOTE_GPU_SMMU_GDS, TZ_VOTE_GDS_SW_COLLAPSE);
}

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for kodiak (qcs6490): the secure UFS GDSC
 * retention vote, and GPU CX for the GPU SMMU setup.
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GCC_APCS_TZ_UFS_PHY_SGDSCR	0x00153008U
#define SGDSCR_RETAIN_FF_ENABLE		BIT_32(1)
#define SGDSCR_SW_OVERRIDE		BIT_32(0)

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U
#define GPU_CC_CX_GMU_CBCR		0x03d91098U
#define GPU_CC_HUB_CX_INT_CBCR		0x03d91204U
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	0x03d93000U
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	0x03d93004U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX	0x03d94004U

static struct clock_desc gpu_cfg_ahb = {
	.cbcr_addr = GCC_GPU_CFG_AHB_CBCR,
};

static struct clock_power_domain_desc gpu_cx = {
	.gdscr_addr = GPU_CC_TZ_VOTE_GPU_SMMU_GDS,
	.poll_pwr_on = true,
};

/* The TZ vote registers have the CBCR CLK_ENABLE and CLK_OFF fields. */
static struct clock_desc gpu_cx_clks[] = {
	{ .cbcr_addr = GPU_CC_TZ_VOTE_GPU_SMMU_CLK },
	{ .cbcr_addr = GPU_CC_CX_GMU_CBCR },
	{ .cbcr_addr = GPU_CC_HUB_CX_INT_CBCR },
};

static const struct hoya_gpu_cx gpu = {
	.cfg_ahb = &gpu_cfg_ahb,
	.cx = &gpu_cx,
	.clks = gpu_cx_clks,
	.num_clks = ARRAY_SIZE(gpu_cx_clks),
	.retain_ctl = GPU_CC_TZ_GDSC_CTRL_GPU_CX,
};

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	/*
	 * The OS collapses the UFS GDSC and resumes the link from hibern8,
	 * which needs the host state retained. SW_OVERRIDE stays clear, as
	 * in QTISECLIB.
	 */
	mmio_clrsetbits_32(GCC_APCS_TZ_UFS_PHY_SGDSCR, SGDSCR_SW_OVERRIDE,
			   SGDSCR_RETAIN_FF_ENABLE);

	if (chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		return 0;
	}

	return hoya_clock_gpu_cx_on(&gpu);
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	hoya_clock_gpu_cx_off(&gpu);

	return 0;
}

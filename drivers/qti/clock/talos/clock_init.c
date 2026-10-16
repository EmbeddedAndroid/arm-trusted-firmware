/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for talos: GPU CX for the GPU SMMU setup.
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/utils_def.h>

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U
#define GPU_CC_CX_GDSCR			0x0509106cU
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	0x05093000U
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	0x05093004U

static struct clock_desc gpu_cfg_ahb = {
	.cbcr_addr = GCC_GPU_CFG_AHB_CBCR,
};

static struct clock_power_domain_desc gpu_cx = {
	.gdscr_addr = GPU_CC_TZ_VOTE_GPU_SMMU_GDS,
	.poll_pwr_on = true,
};

static struct clock_desc gpu_cx_clks[] = {
	{ .cbcr_addr = GPU_CC_TZ_VOTE_GPU_SMMU_CLK },
};

/* The CX register retention control of Talos is in the CX GDSCR. */
static const struct hoya_gpu_cx gpu = {
	.cfg_ahb = &gpu_cfg_ahb,
	.cx = &gpu_cx,
	.clks = gpu_cx_clks,
	.num_clks = ARRAY_SIZE(gpu_cx_clks),
	.retain_ctl = GPU_CC_CX_GDSCR,
};

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

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

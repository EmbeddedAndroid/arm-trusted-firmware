/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for pakala (SM8750): GPU CX for the GPU
 * registers BL31 programs at cold boot.
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX	0x03d94004U
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	0x03d95000U
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	0x03d95004U
#define GPU_CC_CX_GMU_CBCR		0x03d990d4U
#define GPU_CC_HUB_CX_INT_CBCR		0x03d990e8U

/* Adreno 830 CX_MISC (cx_mem at 0x03d9e000) software feature fuses */
#define GPU_CX_MISC_SW_FUSE_VALUE	0x03d9f000U
#define SW_FUSE_FASTBLEND		BIT_32(0)
#define SW_FUSE_LPAC			BIT_32(1)
#define SW_FUSE_RAYTRACING		BIT_32(2)

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
	int ret;

	(void)drv_ctxt;

	if (chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		return 0;
	}

	ret = hoya_clock_gpu_cx_on(&gpu);
	if (ret != 0) {
		return ret;
	}

	/*
	 * Only the secure world may write the software feature fuses; Linux
	 * reads them back on the first GMU start. CX retention keeps them.
	 */
	mmio_write_32(GPU_CX_MISC_SW_FUSE_VALUE,
		      SW_FUSE_FASTBLEND | SW_FUSE_LPAC | SW_FUSE_RAYTRACING);

	return 0;
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	hoya_clock_gpu_cx_off(&gpu);

	return 0;
}

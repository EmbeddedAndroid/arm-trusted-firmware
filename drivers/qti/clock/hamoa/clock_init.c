/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for hamoa (X1E80100): the secure GPU GDSC
 * controls and GPU CX for the GPU SMMU setup.
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GCC_GPU_CFG_AHB_CBCR			0x00171004U
#define GPU_CC_TZ_GDSC_CTRL_GPU_GX		0x03d94000U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX		0x03d94004U
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK		0x03d95000U
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS		0x03d95004U
#define GPU_CC_TZ_VOTE_HUB_CX_INT_CLK		0x03d95008U
#define GPU_CC_TZ_VOTE_CX_GMU_CLK		0x03d9500cU
#define GPU_CC_CX_GMU_CBCR			0x03d9913cU

#define CBCR_FORCE_MEM_PERIPH_ON		BIT_32(13)

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
	{ .cbcr_addr = GPU_CC_TZ_VOTE_HUB_CX_INT_CLK },
	{ .cbcr_addr = GPU_CC_TZ_VOTE_CX_GMU_CLK },
};

static const struct hoya_gpu_cx gpu = {
	.cfg_ahb = &gpu_cfg_ahb,
	.cx = &gpu_cx,
	.clks = gpu_cx_clks,
	.num_clks = ARRAY_SIZE(gpu_cx_clks),
	.retain_ctl = GPU_CC_TZ_GDSC_CTRL_GPU_CX,
};

/*
 * XBL leaves the secure GPU GDSC controls at reset. Leave the CX and GX
 * GDSCs to the voting logic with flip-flop retention, and keep the CX GMU
 * memories powered, as the GMU expects.
 */
static void gpu_gdsc_init(void)
{
	hoya_clock_gdsc_retain(GPU_CC_TZ_GDSC_CTRL_GPU_CX);
	hoya_clock_gdsc_retain(GPU_CC_TZ_GDSC_CTRL_GPU_GX);
	mmio_setbits_32(GPU_CC_CX_GMU_CBCR, CBCR_FORCE_MEM_PERIPH_ON);
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	int ret;

	(void)drv_ctxt;

	if (chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		return 0;
	}

	ret = hoya_clock_gpu_cx_on(&gpu);
	if (ret == 0) {
		gpu_gdsc_init();
	}

	return ret;
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	hoya_clock_gpu_cx_off(&gpu);

	return 0;
}

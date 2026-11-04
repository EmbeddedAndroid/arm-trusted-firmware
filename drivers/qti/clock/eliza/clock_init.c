/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for eliza (SM7750): the secure GPU GDSC
 * controls, GPU CX for the GPU SMMU setup, and the LPASS setup the ADSP
 * expects at cold boot.
 */

#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GCC_GPU_CFG_AHB_CBCR		0x00171004U
#define GPU_CC_TZ_GDSC_CTRL_GPU_GX	0x03d94000U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX	0x03d94004U
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK	0x03d95000U
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS	0x03d95004U
#define GPU_CC_CX_GMU_CBCR		0x03d9916cU
#define GPU_CC_HUB_CX_INT_CBCR		0x03d99180U

#define CBCR_FORCE_MEM_PERIPH_ON	BIT_32(13)

#define GCC_LPASS_CFG_NOC_SWAY_CBCR		0x00147000U
#define GCC_LPASS_AXIS2_CBCR			0x00147014U
#define GCC_LPASS_AGGRE_NOC_DDRSS_SHUB_CBCR	0x00147030U
#define GCC_CONFIG_NOC_LPASS_BOOT_CBCR		0x0014717cU
#define LPASS_AUDIO_CC_SLIMBUS_CBCR		0x06bd5018U
#define LPASS_AUDIO_CC_SLIMBUS_CORE_CBCR	0x06bd502cU
#define LPASS_AON_CC_PDC_H_CBCR			0x06e1100cU
#define LPASS_AON_CC_AUDIO_HM_GDSCR		0x06e11090U
#define LPASS_AON_CC_PDC_HM_GDSCR		0x06e20090U
#define LPASS_AON_CC_LPI_NOC_HS_CBCR		0x06e21038U
#define LPASS_CORE_GDSCR_DEBUG_VOTE		0x07e02000U
#define TCSR_LPASS_BOOT_IMEM_DISABLE		0x01ff7000U

#define DEBUG_VOTE_RETENTION_ENABLE		BIT_32(0)

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

static struct clock_desc lpass_sway_clk = {
	.cbcr_addr = GCC_LPASS_CFG_NOC_SWAY_CBCR,
};

static struct clock_power_domain_desc lpass_gdscs[] = {
	{ .gdscr_addr = LPASS_AON_CC_AUDIO_HM_GDSCR, .poll_pwr_on = true },
	{ .gdscr_addr = LPASS_AON_CC_PDC_HM_GDSCR, .poll_pwr_on = true },
};

static struct clock_desc lpass_clks[] = {
	{ .cbcr_addr = GCC_LPASS_AGGRE_NOC_DDRSS_SHUB_CBCR },
	{ .cbcr_addr = LPASS_AUDIO_CC_SLIMBUS_CBCR },
	{ .cbcr_addr = LPASS_AUDIO_CC_SLIMBUS_CORE_CBCR },
	{ .cbcr_addr = LPASS_AON_CC_PDC_H_CBCR },
	{ .cbcr_addr = LPASS_AON_CC_LPI_NOC_HS_CBCR },
};

static struct clock_desc lpass_boot_clks[] = {
	{ .cbcr_addr = GCC_CONFIG_NOC_LPASS_BOOT_CBCR },
	{ .cbcr_addr = GCC_LPASS_AXIS2_CBCR },
};

static int lpass_clk_on(struct clock_desc *clk)
{
	int ret;

	clock_hal_set_clock(clk, true);
	ret = clock_hal_wait_for_clock_on(clk);
	if (ret != 0) {
		WARN("Clock: LPASS branch 0x%lx stays off\n", clk->cbcr_addr);
	}

	return ret;
}

/*
 * The ADSP expects, at cold boot, a retention vote on the LPASS core GDSC
 * made with the LPASS audio and PDC hardware modules powered, the LPASS boot
 * IMEM released and the LPASS boot NoC and AXIS2 clocks off. What this turns
 * on goes back to the state it was found in.
 */
static void lpass_init(void)
{
	size_t i;

	/* The LPASS registers cannot be reached without the sway clock. */
	if (lpass_clk_on(&lpass_sway_clk) != 0) {
		if (lpass_sway_clk.tfa_enabled) {
			clock_hal_set_clock(&lpass_sway_clk, false);
		}
		return;
	}

	for (i = 0U; i < ARRAY_SIZE(lpass_gdscs); i++) {
		/* A GDSC another voter holds on is left alone. */
		if ((mmio_read_32(lpass_gdscs[i].gdscr_addr) &
		     HAL_CLK_GDSCR_PWR_ON_FMSK) != 0U) {
			continue;
		}
		clock_hal_enable_power_domain(&lpass_gdscs[i]);
		if (clock_hal_wait_for_power_domain_on(&lpass_gdscs[i]) != 0) {
			WARN("Clock: LPASS GDSC 0x%lx stays off\n",
			     lpass_gdscs[i].gdscr_addr);
		}
	}
	for (i = 0U; i < ARRAY_SIZE(lpass_clks); i++) {
		(void)lpass_clk_on(&lpass_clks[i]);
	}

	mmio_setbits_32(LPASS_CORE_GDSCR_DEBUG_VOTE,
			DEBUG_VOTE_RETENTION_ENABLE);

	/* No line of the boot IMEM may be written back once it is gone. */
	dcsw_op_all(DCCISW);
	mmio_write_32(TCSR_LPASS_BOOT_IMEM_DISABLE, 1U);
	dsbsy();

	for (i = 0U; i < ARRAY_SIZE(lpass_boot_clks); i++) {
		clock_hal_set_clock(&lpass_boot_clks[i], false);
	}

	for (i = ARRAY_SIZE(lpass_clks); i > 0U; i--) {
		if (lpass_clks[i - 1U].tfa_enabled) {
			clock_hal_set_clock(&lpass_clks[i - 1U], false);
		}
	}
	for (i = ARRAY_SIZE(lpass_gdscs); i > 0U; i--) {
		if (lpass_gdscs[i - 1U].tfa_enabled) {
			clock_hal_disable_power_domain(&lpass_gdscs[i - 1U]);
			if (clock_hal_wait_for_power_domain_off(
				    &lpass_gdscs[i - 1U]) != 0) {
				WARN("Clock: LPASS GDSC 0x%lx stays on\n",
				     lpass_gdscs[i - 1U].gdscr_addr);
			}
		}
	}
	if (lpass_sway_clk.tfa_enabled) {
		clock_hal_set_clock(&lpass_sway_clk, false);
	}
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	int ret;

	(void)drv_ctxt;

	lpass_init();

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

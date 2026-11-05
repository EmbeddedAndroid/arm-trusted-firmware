/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for hamoa (X1E80100): the secure GPU GDSC
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

#define GCC_GPU_CFG_AHB_CBCR			0x00171004U
#define GPU_CC_TZ_GDSC_CTRL_GPU_GX		0x03d94000U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX		0x03d94004U
#define GPU_CC_TZ_VOTE_GPU_SMMU_CLK		0x03d95000U
#define GPU_CC_TZ_VOTE_GPU_SMMU_GDS		0x03d95004U
#define GPU_CC_TZ_VOTE_HUB_CX_INT_CLK		0x03d95008U
#define GPU_CC_TZ_VOTE_CX_GMU_CLK		0x03d9500cU
#define GPU_CC_CX_GMU_CBCR			0x03d9913cU

#define CBCR_FORCE_MEM_PERIPH_ON		BIT_32(13)

#define GCC_LPASS_CFG_NOC_SWAY_CBCR		0x00147000U
#define GCC_LPASS_AGGRE_NOC_DDRSS_SHUB_CBCR	0x00147024U
#define GCC_LPASS_AXIS2_CBCR			0x0014d000U
#define GCC_APCS_TZ_LPASS_QTB_SGDSCR		0x00153044U
#define GCC_LPASS_QTB_GDSCR			0x00199000U
#define LPASS_AUDIO_CC_XPU2_CLIENT_CBCR		0x06bd1000U
#define LPASS_AUDIO_CC_SLIMBUS_CBCR		0x06bd5018U
#define LPASS_AUDIO_CC_SLIMBUS_CORE_CBCR	0x06bd501cU
#define LPASS_AUDIO_CC_BUS_CBCR			0x06be7000U
#define LPASS_AON_CC_VA_XPU2_CLIENT_CBCR	0x06e09018U
#define LPASS_AON_CC_PDC_H_CBCR			0x06e1100cU
#define LPASS_AON_CC_VA_MEM0_CBCR		0x06e11060U
#define LPASS_AON_CC_AUDIO_HM_GDSCR		0x06e11090U
#define LPASS_AON_CC_LPI_NOC_LS_CBCR		0x06e11300U
#define LPASS_AON_CC_ENPU1_RETENTION_VOTE	0x06e1f000U
#define LPASS_AON_CC_PDC_HM_RETENTION_VOTE	0x06e1f004U
#define LPASS_AON_CC_AUDIO_HM_RETENTION_VOTE	0x06e1f008U
#define LPASS_AON_CC_ENPU2_RETENTION_VOTE	0x06e1f010U
#define LPASS_AON_CC_SSC_RETENTION_VOTE		0x06e1f014U
#define LPASS_AON_CC_PDC_HM_GDSCR		0x06e20090U
#define LPASS_AON_CC_LPI_NOC_HS_CBCR		0x06e21008U
#define LPASS_TOP_CC_AGGNOC_LS_CBCR		0x07e06000U
#define TCSR_BOOT_IMEM_DISABLE			0x01fd3300U

#define SGDSCR_SW_OVERRIDE			BIT_32(0)
#define SGDSCR_RETAIN_FF_ENABLE			BIT_32(1)
#define RETENTION_VOTE_SECURE			BIT_32(0)
#define RETENTION_VOTE_HW_CONTROL_OVERRIDE	BIT_32(1)

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

static struct clock_desc lpass_sway_clk = {
	.cbcr_addr = GCC_LPASS_CFG_NOC_SWAY_CBCR,
};

static struct clock_power_domain_desc lpass_gdscs[] = {
	{ .gdscr_addr = GCC_LPASS_QTB_GDSCR, .poll_pwr_on = true },
	{ .gdscr_addr = LPASS_AON_CC_AUDIO_HM_GDSCR, .poll_pwr_on = true },
	{ .gdscr_addr = LPASS_AON_CC_PDC_HM_GDSCR, .poll_pwr_on = true },
};

static struct clock_desc lpass_clks[] = {
	{ .cbcr_addr = LPASS_AON_CC_VA_MEM0_CBCR },
	{ .cbcr_addr = LPASS_AON_CC_VA_XPU2_CLIENT_CBCR },
	{ .cbcr_addr = LPASS_AUDIO_CC_SLIMBUS_CBCR },
	{ .cbcr_addr = LPASS_AUDIO_CC_SLIMBUS_CORE_CBCR },
	{ .cbcr_addr = LPASS_AON_CC_PDC_H_CBCR },
	{ .cbcr_addr = LPASS_AON_CC_LPI_NOC_LS_CBCR },
	{ .cbcr_addr = LPASS_AON_CC_LPI_NOC_HS_CBCR },
	{ .cbcr_addr = LPASS_AUDIO_CC_BUS_CBCR },
	{ .cbcr_addr = LPASS_AUDIO_CC_XPU2_CLIENT_CBCR },
	{ .cbcr_addr = GCC_LPASS_AGGRE_NOC_DDRSS_SHUB_CBCR },
	{ .cbcr_addr = LPASS_TOP_CC_AGGNOC_LS_CBCR },
};

static struct clock_desc lpass_axis2_clk = {
	.cbcr_addr = GCC_LPASS_AXIS2_CBCR,
};

/* Retention votes taken from hardware control; the first three are made. */
static const uintptr_t lpass_votes[] = {
	LPASS_AON_CC_SSC_RETENTION_VOTE,
	LPASS_AON_CC_PDC_HM_RETENTION_VOTE,
	LPASS_AON_CC_AUDIO_HM_RETENTION_VOTE,
	LPASS_AON_CC_ENPU2_RETENTION_VOTE,
	LPASS_AON_CC_ENPU1_RETENTION_VOTE,
};

#define LPASS_VOTES_MADE			3U

static void lpass_clk_on(struct clock_desc *clk)
{
	clock_hal_set_clock(clk, true);
	if (clock_hal_wait_for_clock_on(clk) != 0) {
		WARN("Clock: LPASS branch 0x%lx stays off\n", clk->cbcr_addr);
	}
}

/*
 * The ADSP expects, at cold boot, the LPASS retention votes of the SSC,
 * PDC_HM and AUDIO_HM domains taken from hardware control and made secure
 * with the LPASS QTB, audio and PDC hardware modules powered, the LPASS QTB
 * GDSC left to the voting logic with retention, the boot IMEM released
 * (the Q6SS boot IMEM disconnect fuse is not blown on these parts) and the
 * AXIS2 clock to it off. XBL leaves the boot IMEM connected. What this turns
 * on goes back to the state it was found in.
 */
static void lpass_init(void)
{
	size_t i;

	lpass_clk_on(&lpass_sway_clk);
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
		lpass_clk_on(&lpass_clks[i]);
	}

	/*
	 * Write every cached boot IMEM line back while AXIS2 still clocks the
	 * path to it: an eviction after that stalls the core.
	 */
	dcsw_op_all(DCCISW);
	clock_hal_set_clock(&lpass_axis2_clk, false);

	for (i = 0U; i < ARRAY_SIZE(lpass_votes); i++) {
		mmio_clrbits_32(lpass_votes[i],
				RETENTION_VOTE_HW_CONTROL_OVERRIDE);
	}
	for (i = 0U; i < LPASS_VOTES_MADE; i++) {
		mmio_setbits_32(lpass_votes[i], RETENTION_VOTE_SECURE);
	}

	mmio_write_32(TCSR_BOOT_IMEM_DISABLE, 1U);
	dsbsy();

	mmio_clrsetbits_32(GCC_APCS_TZ_LPASS_QTB_SGDSCR, SGDSCR_SW_OVERRIDE,
			   SGDSCR_RETAIN_FF_ENABLE);

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

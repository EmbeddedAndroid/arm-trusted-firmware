/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/gpu_smmu/gpu_smmu.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define CBCR_CLK_ENABLE			BIT_32(0)
#define CBCR_CLK_OFF			BIT_32(31)
#define TZ_VOTE_CLK_ENABLE		BIT_32(0)
#define TZ_VOTE_CLK_OFF			BIT_32(31)
#define TZ_VOTE_GDS_SW_COLLAPSE		BIT_32(0)
#define TZ_VOTE_GDS_PWR_ON		BIT_32(31)
#define GDSCR_RETAIN_FF_ENABLE		BIT_32(11)

/* GPU CX takes about 1.7 ms to power up on Kodiak. */
#define GPU_SMMU_TIMEOUT_US		10000U
#define GPU_SMMU_MAX_CX_CBCRS		4U

static bool wait_bits(uintptr_t addr, uint32_t mask, uint32_t want)
{
	uint64_t timeout = timeout_init_us(GPU_SMMU_TIMEOUT_US);

	while ((mmio_read_32(addr) & mask) != want) {
		if (timeout_elapsed(timeout)) {
			return (mmio_read_32(addr) & mask) == want;
		}
	}

	return true;
}

static bool branch_enabled(uintptr_t cbcr)
{
	return (mmio_read_32(cbcr) & CBCR_CLK_ENABLE) != 0U;
}

static bool branch_enable(uintptr_t cbcr)
{
	mmio_setbits_32(cbcr, CBCR_CLK_ENABLE);

	return wait_bits(cbcr, CBCR_CLK_OFF, 0U);
}

static int program_regs(const struct qti_gpu_smmu_cfg *cfg)
{
	for (size_t i = 0U; i < cfg->num_regs; i++) {
		const struct qti_gpu_smmu_reg *reg = &cfg->regs[i];
		uint32_t val;

		mmio_write_32(reg->addr, reg->value);
		if (!reg->verify) {
			continue;
		}

		val = mmio_read_32(reg->addr);
		if (val != reg->value) {
			ERROR("GPU SMMU: 0x%lx reads 0x%x, wrote 0x%x\n",
			      reg->addr, val, reg->value);
			return -EIO;
		}
	}

	return 0;
}

/*
 * Turn on what the GPU SMMU registers need, program them, and give back
 * every vote and clock to the state it was found in. Only the CX register
 * retention stays: the OS does not set it, and without it the registers are
 * lost at the first CX collapse.
 */
static int gpu_smmu_setup(const struct qti_gpu_smmu_cfg *cfg)
{
	bool cbcr_was_on[GPU_SMMU_MAX_CX_CBCRS];
	bool ahb_was_on, cx_was_on, clk_was_on;
	int ret = -ETIMEDOUT;
	size_t n = 0U;

	assert(cfg->num_cx_cbcrs <= GPU_SMMU_MAX_CX_CBCRS);

	ahb_was_on = branch_enabled(cfg->cfg_ahb_cbcr);
	if (!branch_enable(cfg->cfg_ahb_cbcr)) {
		ERROR("GPU SMMU: GPU config AHB clock stays off\n");
		goto ahb_off;
	}

	cx_was_on = (mmio_read_32(cfg->tz_vote_gds) &
		     TZ_VOTE_GDS_SW_COLLAPSE) == 0U;
	mmio_clrbits_32(cfg->tz_vote_gds, TZ_VOTE_GDS_SW_COLLAPSE);
	if (!wait_bits(cfg->tz_vote_gds, TZ_VOTE_GDS_PWR_ON,
		       TZ_VOTE_GDS_PWR_ON)) {
		ERROR("GPU SMMU: GPU CX stays off\n");
		goto cx_off;
	}

	clk_was_on = (mmio_read_32(cfg->tz_vote_clk) & TZ_VOTE_CLK_ENABLE) != 0U;
	mmio_setbits_32(cfg->tz_vote_clk, TZ_VOTE_CLK_ENABLE);
	if (!wait_bits(cfg->tz_vote_clk, TZ_VOTE_CLK_OFF, 0U)) {
		ERROR("GPU SMMU: GPU SMMU clock stays off\n");
		goto clk_off;
	}

	while (n < cfg->num_cx_cbcrs) {
		cbcr_was_on[n] = branch_enabled(cfg->cx_cbcrs[n]);
		if (!branch_enable(cfg->cx_cbcrs[n++])) {
			ERROR("GPU SMMU: GPU CX clock 0x%lx stays off\n",
			      cfg->cx_cbcrs[n - 1U]);
			goto cbcr_off;
		}
	}

	VERBOSE("GPU SMMU: found AHB %d CX %d SMMU clock %d\n",
		ahb_was_on, cx_was_on, clk_was_on);

	mmio_setbits_32(cfg->cx_gdscr, GDSCR_RETAIN_FF_ENABLE);
	ret = program_regs(cfg);

cbcr_off:
	while (n > 0U) {
		n--;
		if (!cbcr_was_on[n]) {
			mmio_clrbits_32(cfg->cx_cbcrs[n], CBCR_CLK_ENABLE);
		}
	}
clk_off:
	if (!clk_was_on) {
		mmio_clrbits_32(cfg->tz_vote_clk, TZ_VOTE_CLK_ENABLE);
	}
cx_off:
	if (!cx_was_on) {
		mmio_setbits_32(cfg->tz_vote_gds, TZ_VOTE_GDS_SW_COLLAPSE);
	}
ahb_off:
	if (!ahb_was_on) {
		mmio_clrbits_32(cfg->cfg_ahb_cbcr, CBCR_CLK_ENABLE);
	}

	return ret;
}

/*
 * The OS GPU driver cannot tell that the aperture is missing, and hangs the
 * GPU at its first page table switch, so a failed setup stops the boot.
 */
void qti_gpu_smmu_init(void)
{
	if (gpu_smmu_setup(&qti_gpu_smmu_cfg) != 0) {
		ERROR("GPU SMMU: setup failed\n");
		panic();
	}
}

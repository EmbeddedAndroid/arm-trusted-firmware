/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * GPU CX bring-up for the GPU SMMU setup on the hoya SoCs whose BL31 votes
 * GPU CX through its GPU_CC TZ vote instead of driving the CX GDSC.
 */

#include <errno.h>
#include <stddef.h>

#include <common/debug.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GDSC_CTRL_RETAIN_FF_ENABLE	BIT_32(11)

static int branch_on(struct clock_desc *clk)
{
	clock_hal_set_clock(clk, true);
	if (clock_hal_wait_for_clock_on(clk) != 0) {
		ERROR("Clock: GPU branch 0x%lx stays off\n", clk->cbcr_addr);
		return -ETIMEDOUT;
	}

	return 0;
}

/*
 * The boot firmware leaves GPU CX collapsed with its clocks off. Turn on the
 * GPU config interface, vote CX on, turn on the branches the GPU SMMU
 * registers need, and enable CX register retention so that what BL31 writes
 * survives the CX collapses that follow. On failure the caller still calls
 * hoya_clock_gpu_cx_off().
 */
int hoya_clock_gpu_cx_on(const struct hoya_gpu_cx *gpu)
{
	int ret = branch_on(gpu->cfg_ahb);

	if (ret != 0) {
		return ret;
	}

	clock_hal_enable_power_domain(gpu->cx);
	if (clock_hal_wait_for_power_domain_on(gpu->cx) != 0) {
		ERROR("Clock: GPU CX stays off\n");
		return -ETIMEDOUT;
	}

	for (size_t i = 0U; (ret == 0) && (i < gpu->num_clks); i++) {
		ret = branch_on(&gpu->clks[i]);
	}
	mmio_setbits_32(gpu->retain_ctl, GDSC_CTRL_RETAIN_FF_ENABLE);

	return ret;
}

/* Give back every vote and branch that hoya_clock_gpu_cx_on() turned on. */
void hoya_clock_gpu_cx_off(const struct hoya_gpu_cx *gpu)
{
	for (size_t i = gpu->num_clks; i > 0U; i--) {
		if (gpu->clks[i - 1U].tfa_enabled) {
			clock_hal_set_clock(&gpu->clks[i - 1U], false);
		}
	}
	if (gpu->cx->tfa_enabled) {
		clock_hal_disable_power_domain(gpu->cx);
	}
	if (gpu->cfg_ahb->tfa_enabled) {
		clock_hal_set_clock(gpu->cfg_ahb, false);
	}
}

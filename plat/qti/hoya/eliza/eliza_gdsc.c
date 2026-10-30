/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <qti_plat.h>

#define GCC_GPU_CFG_AHB_CBCR			0x00171004U

#define GPU_CC_TZ_GDSC_CTRL_GPU_GX		0x03d94000U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX		0x03d94004U
#define GPU_CC_GX_CFG2_GDSCR			0x03d99064U
#define GPU_CC_CX_GMU_CBCR			0x03d9916cU
#define GPU_CC_RCG_SRC_ACTIVE_CTL		0x03d99528U

#define CBCR_CLK_ENABLE				BIT_32(0)
#define CBCR_CLK_OFF				BIT_32(31)
#define CBCR_FORCE_MEM_PERIPH_ON		BIT_32(13)
#define TZ_GDSC_SW_OVERRIDE			BIT_32(2)
#define TZ_GDSC_RETAIN_FF_ENABLE		BIT_32(11)
#define GX_CFG2_MEM_PWR_DWN_TIMEOUT_MASK	0xfU

/* Turn a branch clock on; returns whether it was enabled before. */
static bool branch_on(uintptr_t cbcr)
{
	bool was_on = (mmio_read_32(cbcr) & CBCR_CLK_ENABLE) != 0U;
	unsigned int n;

	mmio_setbits_32(cbcr, CBCR_CLK_ENABLE);
	for (n = 0U; n < 500U; n++) {
		if ((mmio_read_32(cbcr) & CBCR_CLK_OFF) == 0U) {
			return was_on;
		}
		udelay(1U);
	}
	WARN("GDSC: clock 0x%lx did not turn on\n", cbcr);

	return was_on;
}

/*
 * GPU GDSC setup the Qualcomm TZ does at cold boot: the secure GDSC
 * controls of GPU CX and GX lose their software override, so the voting
 * logic stays in charge, and gain flip-flop retention; the CX GMU memories
 * are forced on, the GX memory power-down timeout is relaxed for XO
 * operation and the RCG source-active override is dropped.
 */
void plat_qti_gdsc_init(void)
{
	bool gpu_ahb_on = branch_on(GCC_GPU_CFG_AHB_CBCR);

	mmio_clrsetbits_32(GPU_CC_GX_CFG2_GDSCR,
			   GX_CFG2_MEM_PWR_DWN_TIMEOUT_MASK,
			   GX_CFG2_MEM_PWR_DWN_TIMEOUT_MASK);
	mmio_clrsetbits_32(GPU_CC_TZ_GDSC_CTRL_GPU_CX, TZ_GDSC_SW_OVERRIDE,
			   TZ_GDSC_RETAIN_FF_ENABLE);
	mmio_clrsetbits_32(GPU_CC_TZ_GDSC_CTRL_GPU_GX, TZ_GDSC_SW_OVERRIDE,
			   TZ_GDSC_RETAIN_FF_ENABLE);
	mmio_write_32(GPU_CC_RCG_SRC_ACTIVE_CTL, 0U);
	mmio_setbits_32(GPU_CC_CX_GMU_CBCR, CBCR_FORCE_MEM_PERIPH_ON);

	if (!gpu_ahb_on) {
		mmio_clrbits_32(GCC_GPU_CFG_AHB_CBCR, CBCR_CLK_ENABLE);
	}
}

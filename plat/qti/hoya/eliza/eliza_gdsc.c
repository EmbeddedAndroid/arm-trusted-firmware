/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/cmd_db/cmd_db.h>
#include <drivers/qti/pwr_utils/pwr_utils.h>
#include <drivers/qti/pwr_utils/voltage_level.h>
#include <drivers/qti/rpmh/rpmh_client.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <qti_plat.h>

#define GCC_GPU_CFG_AHB_CBCR			0x00171004U

#define GPU_CC_TZ_GDSC_CTRL_GPU_GX		0x03d94000U
#define GPU_CC_TZ_GDSC_CTRL_GPU_CX		0x03d94004U
#define GPU_CC_GX_BCR				0x03d99058U
#define GPU_CC_GX_GDSCR				0x03d9905cU
#define GPU_CC_GX_CFG_GDSCR			0x03d99060U
#define GPU_CC_GX_CFG2_GDSCR			0x03d99064U
#define GPU_CC_CX_GDSCR				0x03d99110U
#define GPU_CC_CX_CFG_GDSCR			0x03d99114U
#define GPU_CC_CX_GMU_CBCR			0x03d9916cU
#define GPU_CC_HUB_CX_INT_CBCR			0x03d99180U
#define GPU_CC_ACD_BCR				0x03d9939cU
#define GPU_CC_ACD_CXO_CBCR			0x03d993a0U
#define GPU_CC_ACD_AHB_CBCR			0x03d993a4U
#define GPU_CC_GX_DOMAIN_MISC3			0x03d99504U
#define GPU_CC_RCG_SRC_ACTIVE_CTL		0x03d99528U
#define GPU_CC_ACD_MISC				0x03d9958cU
#define GPU_CC_GX_DOMAIN_MISC5			0x03d995d8U

#define CBCR_CLK_ENABLE				BIT_32(0)
#define CBCR_FORCE_MEM_PERIPH_ON		BIT_32(13)
#define CBCR_CLK_DIS				BIT_32(22)
#define CBCR_CLK_OFF				BIT_32(31)
#define BCR_BLK_ARES				BIT_32(0)
#define ACD_MISC_IROOT_ARES			BIT_32(0)
#define GDSCR_SW_COLLAPSE			BIT_32(0)
#define CFG_GDSCR_POWER_DOWN_COMPLETE		BIT_32(15)
#define CFG_GDSCR_POWER_UP_COMPLETE		BIT_32(16)
#define MISC3_GX_RAIL_CLAMP_IO			BIT_32(0)
#define MISC3_GX_RAIL_RESET			BIT_32(4)
#define MISC5_SAVE_FF				BIT_32(0)
#define TZ_GDSC_SW_OVERRIDE			BIT_32(2)
#define TZ_GDSC_RETAIN_FF_ENABLE		BIT_32(11)
#define GX_CFG2_MEM_PWR_DWN_TIMEOUT_MASK	0xfU

#define GDSC_POLL_US				500U
#define GX_ARES_HOLD_US				150U
#define GX_RAIL_RESET_HOLD_US			5U

static bool poll_set(uintptr_t reg, uint32_t mask)
{
	unsigned int n;

	for (n = 0U; n < GDSC_POLL_US; n++) {
		if ((mmio_read_32(reg) & mask) != 0U) {
			return true;
		}
		udelay(1U);
	}

	return false;
}

/* Turn a branch clock on; returns whether it was enabled before. */
static bool branch_on(uintptr_t cbcr)
{
	bool was_on = (mmio_read_32(cbcr) & CBCR_CLK_ENABLE) != 0U;
	unsigned int n;

	mmio_setbits_32(cbcr, CBCR_CLK_ENABLE);
	for (n = 0U; n < GDSC_POLL_US; n++) {
		if ((mmio_read_32(cbcr) & CBCR_CLK_OFF) == 0U) {
			return was_on;
		}
		udelay(1U);
	}
	WARN("GPU: clock 0x%lx did not turn on\n", cbcr);

	return was_on;
}

static bool gfx_vote(struct rpmh_client *rpmh, uint32_t addr, int vlvl)
{
	int hlvl = 0;

	if (vlvl != 0) {
		hlvl = pwr_utils_hlvl_named_resource("gfx.lvl", vlvl, NULL);
		if (hlvl < 0) {
			return false;
		}
	}
	rpmh_barrier_single(rpmh, rpmh_issue_command(rpmh, RPMH_SET_ACTIVE,
						     true, addr, (uint32_t)hlvl));

	return true;
}

/*
 * GX power-up: reset the GX block, the ACD and the always-on logic on the
 * GX rail, release the rail clamp and power the GDSC.
 */
static bool gpu_gx_power_up(void)
{
	mmio_setbits_32(GPU_CC_ACD_AHB_CBCR, CBCR_CLK_DIS);
	mmio_setbits_32(GPU_CC_ACD_CXO_CBCR, CBCR_CLK_DIS);

	mmio_setbits_32(GPU_CC_GX_BCR, BCR_BLK_ARES);
	mmio_setbits_32(GPU_CC_ACD_BCR, BCR_BLK_ARES);
	mmio_setbits_32(GPU_CC_ACD_MISC, ACD_MISC_IROOT_ARES);
	dsb();
	udelay(GX_ARES_HOLD_US);
	mmio_clrbits_32(GPU_CC_GX_BCR, BCR_BLK_ARES);
	mmio_clrbits_32(GPU_CC_ACD_BCR, BCR_BLK_ARES);
	mmio_clrbits_32(GPU_CC_ACD_MISC, ACD_MISC_IROOT_ARES);
	dsb();

	mmio_setbits_32(GPU_CC_GX_DOMAIN_MISC3, MISC3_GX_RAIL_RESET);
	dsb();
	udelay(GX_RAIL_RESET_HOLD_US);
	mmio_clrbits_32(GPU_CC_GX_DOMAIN_MISC3, MISC3_GX_RAIL_RESET);
	dsb();
	mmio_clrbits_32(GPU_CC_GX_DOMAIN_MISC3, MISC3_GX_RAIL_CLAMP_IO);
	dsb();

	mmio_clrbits_32(GPU_CC_GX_GDSCR, GDSCR_SW_COLLAPSE);

	return poll_set(GPU_CC_GX_CFG_GDSCR, CFG_GDSCR_POWER_UP_COMPLETE);
}

/* GX power-down, leaving the rail clamped and its always-on logic in reset. */
static bool gpu_gx_power_down(void)
{
	bool down;

	mmio_setbits_32(GPU_CC_GX_GDSCR, GDSCR_SW_COLLAPSE);
	down = poll_set(GPU_CC_GX_CFG_GDSCR, CFG_GDSCR_POWER_DOWN_COMPLETE);

	mmio_setbits_32(GPU_CC_GX_BCR, BCR_BLK_ARES);
	dsb();
	mmio_setbits_32(GPU_CC_ACD_AHB_CBCR, CBCR_CLK_DIS);
	mmio_setbits_32(GPU_CC_ACD_CXO_CBCR, CBCR_CLK_DIS);
	dsb();
	mmio_setbits_32(GPU_CC_GX_DOMAIN_MISC5, MISC5_SAVE_FF);
	dsb();
	udelay(GX_RAIL_RESET_HOLD_US);
	mmio_clrbits_32(GPU_CC_GX_DOMAIN_MISC5, MISC5_SAVE_FF);
	dsb();
	mmio_setbits_32(GPU_CC_ACD_BCR, BCR_BLK_ARES);
	dsb();
	mmio_setbits_32(GPU_CC_ACD_MISC, ACD_MISC_IROOT_ARES);
	dsb();
	udelay(GX_ARES_HOLD_US);
	mmio_setbits_32(GPU_CC_GX_DOMAIN_MISC3, MISC3_GX_RAIL_RESET);
	dsb();
	mmio_setbits_32(GPU_CC_GX_DOMAIN_MISC3, MISC3_GX_RAIL_CLAMP_IO);
	dsb();

	return down;
}

/*
 * Cold-boot GPU setup with the GX rail voted to nominal: power GPU CX and GX
 * up once, hand the secure GDSC controls of CX and GX to the voting logic
 * with flip-flop retention, force the CX GMU memories on, then power GX and
 * CX back down and release the rail. The GMU powers GX itself afterwards.
 */
void plat_qti_clocked_boot_init(void)
{
	struct rpmh_client *rpmh;
	uint32_t gfx;
	bool gpu_ahb_on, gmu_on, hub_on, gx_up, gx_down;

	gfx = cmd_db_query_addr("gfx.lvl");
	rpmh = rpmh_create_handle(RSC_DRV_TZ, "gpu");
	if ((gfx == 0U) || (rpmh == NULL) ||
	    !gfx_vote(rpmh, gfx, RAIL_VOLTAGE_LEVEL_NOM)) {
		ERROR("GPU: cannot vote for the GX rail\n");
		return;
	}

	gpu_ahb_on = branch_on(GCC_GPU_CFG_AHB_CBCR);
	gmu_on = branch_on(GPU_CC_CX_GMU_CBCR);
	hub_on = branch_on(GPU_CC_HUB_CX_INT_CBCR);

	mmio_clrbits_32(GPU_CC_CX_GDSCR, GDSCR_SW_COLLAPSE);
	if (!poll_set(GPU_CC_CX_CFG_GDSCR, CFG_GDSCR_POWER_UP_COMPLETE)) {
		WARN("GPU: CX GDSC did not power up\n");
	}
	gx_up = gpu_gx_power_up();

	mmio_clrsetbits_32(GPU_CC_TZ_GDSC_CTRL_GPU_CX, TZ_GDSC_SW_OVERRIDE,
			   TZ_GDSC_RETAIN_FF_ENABLE);
	mmio_clrsetbits_32(GPU_CC_TZ_GDSC_CTRL_GPU_GX, TZ_GDSC_SW_OVERRIDE,
			   TZ_GDSC_RETAIN_FF_ENABLE);
	mmio_clrsetbits_32(GPU_CC_GX_CFG2_GDSCR,
			   GX_CFG2_MEM_PWR_DWN_TIMEOUT_MASK,
			   GX_CFG2_MEM_PWR_DWN_TIMEOUT_MASK);
	mmio_write_32(GPU_CC_RCG_SRC_ACTIVE_CTL, 0U);
	mmio_setbits_32(GPU_CC_CX_GMU_CBCR, CBCR_FORCE_MEM_PERIPH_ON);

	gx_down = gpu_gx_power_down();
	mmio_setbits_32(GPU_CC_CX_GDSCR, GDSCR_SW_COLLAPSE);

	if (!hub_on) {
		mmio_clrbits_32(GPU_CC_HUB_CX_INT_CBCR, CBCR_CLK_ENABLE);
	}
	if (!gmu_on) {
		mmio_clrbits_32(GPU_CC_CX_GMU_CBCR, CBCR_CLK_ENABLE);
	}
	if (!gpu_ahb_on) {
		mmio_clrbits_32(GCC_GPU_CFG_AHB_CBCR, CBCR_CLK_ENABLE);
	}

	(void)gfx_vote(rpmh, gfx, 0);

	if (!gx_up || !gx_down) {
		WARN("GPU: GX GDSC power %s did not complete\n",
		     gx_up ? "down" : "up");
	}
}

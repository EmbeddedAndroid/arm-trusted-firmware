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
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/smmu/smmu.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <qti_plat.h>

#define CBCR_CLK_ENABLE			BIT_32(0)
#define CBCR_CLK_OFF			BIT_32(31)
#define TZ_VOTE_CLK_ENABLE		BIT_32(0)
#define TZ_VOTE_CLK_OFF			BIT_32(31)
#define TZ_VOTE_GDS_SW_COLLAPSE		BIT_32(0)
#define TZ_VOTE_GDS_PWR_ON		BIT_32(31)
#define TZ_GDSC_CTRL_RETAIN_FF_ENABLE	BIT_32(11)
#define GDSCR_PWR_ON			BIT_32(31)

/* GPU CX takes about 1.7 ms to power up on Kodiak. */
#define GPU_CX_TIMEOUT_US		10000U
#define GPU_CX_MAX_CBCRS		4U

#define GPU_SMMU_CB_SIZE		0x1000U

/* What gpu_cx_on() found on, so that gpu_cx_restore() leaves it on. */
struct gpu_cx_state {
	bool ahb_was_on;
	bool cx_was_on;
	bool clk_was_on;
	bool cbcr_was_on[GPU_CX_MAX_CBCRS];
	size_t num_cbcrs;
};

static bool wait_bits(uintptr_t addr, uint32_t mask, uint32_t want)
{
	uint64_t timeout = timeout_init_us(GPU_CX_TIMEOUT_US);

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

static bool branch_on(uintptr_t cbcr)
{
	return (mmio_read_32(cbcr) & CBCR_CLK_OFF) == 0U;
}

static bool branch_enable(uintptr_t cbcr)
{
	mmio_setbits_32(cbcr, CBCR_CLK_ENABLE);

	return wait_bits(cbcr, CBCR_CLK_OFF, 0U);
}

/*
 * Power GPU CX and the clocks its SMMU registers need. On failure, the
 * caller still calls gpu_cx_restore() to undo the part that was done.
 */
static int gpu_cx_on(const struct qti_smmu_gpu_cx *gpu,
		     struct gpu_cx_state *s)
{
	*s = (struct gpu_cx_state){
		.cx_was_on = true,
		.clk_was_on = true,
	};

	assert(gpu->num_cx_cbcrs <= GPU_CX_MAX_CBCRS);

	s->ahb_was_on = branch_enabled(gpu->cfg_ahb_cbcr);
	if (!s->ahb_was_on && !branch_enable(gpu->cfg_ahb_cbcr)) {
		ERROR("GPU SMMU: GPU config AHB clock stays off\n");
		return -ETIMEDOUT;
	}

	s->cx_was_on = (mmio_read_32(gpu->tz_vote_gds) &
			TZ_VOTE_GDS_SW_COLLAPSE) == 0U;
	mmio_clrbits_32(gpu->tz_vote_gds, TZ_VOTE_GDS_SW_COLLAPSE);
	if (!wait_bits(gpu->tz_vote_gds, TZ_VOTE_GDS_PWR_ON,
		       TZ_VOTE_GDS_PWR_ON)) {
		ERROR("GPU SMMU: GPU CX stays off\n");
		return -ETIMEDOUT;
	}

	s->clk_was_on = (mmio_read_32(gpu->tz_vote_clk) &
			 TZ_VOTE_CLK_ENABLE) != 0U;
	mmio_setbits_32(gpu->tz_vote_clk, TZ_VOTE_CLK_ENABLE);
	if (!wait_bits(gpu->tz_vote_clk, TZ_VOTE_CLK_OFF, 0U)) {
		ERROR("GPU SMMU: GPU SMMU clock stays off\n");
		return -ETIMEDOUT;
	}

	for (size_t i = 0U; i < gpu->num_cx_cbcrs; i++) {
		s->cbcr_was_on[i] = branch_enabled(gpu->cx_cbcrs[i]);
		s->num_cbcrs++;
		if (!s->cbcr_was_on[i] && !branch_enable(gpu->cx_cbcrs[i])) {
			ERROR("GPU SMMU: GPU CX clock 0x%lx stays off\n",
			      gpu->cx_cbcrs[i]);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

/* Give back every vote and clock that gpu_cx_on() found off. */
static void gpu_cx_restore(const struct qti_smmu_gpu_cx *gpu,
			   const struct gpu_cx_state *s)
{
	for (size_t i = s->num_cbcrs; i > 0U; i--) {
		if (!s->cbcr_was_on[i - 1U]) {
			mmio_clrbits_32(gpu->cx_cbcrs[i - 1U], CBCR_CLK_ENABLE);
		}
	}
	if (!s->clk_was_on) {
		mmio_clrbits_32(gpu->tz_vote_clk, TZ_VOTE_CLK_ENABLE);
	}
	if (!s->cx_was_on) {
		mmio_setbits_32(gpu->tz_vote_gds, TZ_VOTE_GDS_SW_COLLAPSE);
	}
	if (!s->ahb_was_on) {
		mmio_clrbits_32(gpu->cfg_ahb_cbcr, CBCR_CLK_ENABLE);
	}
}

/*
 * Program the secure GPU SMMU registers and turn on the secure CX register
 * retention control, which keeps them over CX collapses: the OS sets its own
 * only when it powers CX on, after the collapse that follows this setup.
 * The OS can neither read nor repair these registers, so a failed setup
 * stops the boot.
 */
void qti_smmu_gpu_init(void)
{
	const struct qti_smmu_gpu_cx *gpu = &qti_smmu_gpu_cx;
	struct gpu_cx_state s;
	int ret;

	if (chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		return;
	}

	ret = gpu_cx_on(gpu, &s);
	if (ret == 0) {
		mmio_setbits_32(gpu->tz_gdsc_ctrl,
				TZ_GDSC_CTRL_RETAIN_FF_ENABLE);
		for (size_t i = 0U; i < gpu->num_regs; i++) {
			mmio_write_32(gpu->regs[i].addr, gpu->regs[i].value);
		}
	}
	gpu_cx_restore(gpu, &s);

	if (ret != 0) {
		ERROR("GPU SMMU: setup failed\n");
		panic();
	}
}

bool plat_qti_gpu_smmu_aperture_supported(void)
{
	return !chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U);
}

/* A pair with a 0xff byte is unused. */
#define APERTURE_UNUSED			0xffU

static int aperture_set(const struct qti_smmu_gpu_cx *gpu, uint32_t cb,
			uint32_t slot)
{
	uint32_t val = gpu->cb0_offset + (cb * GPU_SMMU_CB_SIZE);

	mmio_write_32(gpu->aperture[slot], val);
	if (mmio_read_32(gpu->aperture[slot]) != val) {
		ERROR("GPU SMMU: aperture 0x%lx reads 0x%x, wrote 0x%x\n",
		      gpu->aperture[slot], mmio_read_32(gpu->aperture[slot]),
		      val);
		return -EIO;
	}

	return 0;
}

/* At runtime the OS owns GPU CX and its clocks: check them, never vote. */
static bool gpu_cx_is_on(const struct qti_smmu_gpu_cx *gpu)
{
	if (!branch_on(gpu->cfg_ahb_cbcr) ||
	    ((mmio_read_32(gpu->cx_gdscr) & GDSCR_PWR_ON) == 0U) ||
	    !branch_on(gpu->tz_vote_clk)) {
		return false;
	}

	for (size_t i = 0U; i < gpu->num_cx_cbcrs; i++) {
		if (!branch_on(gpu->cx_cbcrs[i])) {
			return false;
		}
	}

	return true;
}

int plat_qti_gpu_smmu_aperture(const uint32_t pairs[4])
{
	const struct qti_smmu_gpu_cx *gpu = &qti_smmu_gpu_cx;
	uint32_t cb[8], slot[8];
	int ret = 0;

	for (unsigned int i = 0U; i < 8U; i++) {
		uint32_t pair = pairs[i / 2U] >> (16U * (i % 2U));

		cb[i] = pair & 0xffU;
		slot[i] = (pair >> 8) & 0xffU;
		if ((cb[i] == APERTURE_UNUSED) ||
		    (slot[i] == APERTURE_UNUSED)) {
			slot[i] = APERTURE_UNUSED;
		} else if ((cb[i] >= gpu->num_cbs) ||
			   (slot[i] >= gpu->num_aperture)) {
			return -EINVAL;
		}
	}

	if (!gpu_cx_is_on(gpu)) {
		return -EBUSY;
	}

	for (unsigned int i = 0U; (ret == 0) && (i < 8U); i++) {
		if (slot[i] != APERTURE_UNUSED) {
			ret = aperture_set(gpu, cb[i], slot[i]);
		}
	}

	/* Enable the slots once they point at their context banks. */
	if ((ret == 0) && (gpu->aperture_ctrl != 0U)) {
		mmio_write_32(gpu->aperture_ctrl, gpu->aperture_ctrl_en);
		if (mmio_read_32(gpu->aperture_ctrl) != gpu->aperture_ctrl_en) {
			ERROR("GPU SMMU: aperture control reads 0x%x\n",
			      mmio_read_32(gpu->aperture_ctrl));
			ret = -EIO;
		}
	}

	return ret;
}

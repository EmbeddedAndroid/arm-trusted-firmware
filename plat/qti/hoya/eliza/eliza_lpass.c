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
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <qti_plat.h>

/* GCC */
#define GCC_LPASS_CFG_NOC_SWAY_CBCR		0x00147000U
#define GCC_LPASS_AXIS2_CBCR			0x00147014U
#define GCC_LPASS_AGGRE_NOC_DDRSS_SHUB_CBCR	0x00147030U
#define GCC_CONFIG_NOC_LPASS_BOOT_CBCR		0x0014717cU

/* LPASS */
#define LPASS_AUDIO_CC_SLIMBUS_CBCR		0x06bd5018U
#define LPASS_AUDIO_CC_SLIMBUS_CORE_CBCR	0x06bd502cU
#define LPASS_AON_CC_PDC_H_CBCR			0x06e1100cU
#define LPASS_AON_CC_AUDIO_HM_GDSCR		0x06e11090U
#define LPASS_AON_CC_PDC_HM_GDSCR		0x06e20090U
#define LPASS_AON_CC_LPI_NOC_HS_CBCR		0x06e21038U
#define LPASS_CORE_GDSCR_DEBUG_VOTE		0x07e02000U

/* TCSR */
#define TCSR_LPASS_BOOT_IMEM_DISABLE		0x01ff7000U

#define CBCR_CLK_ENABLE				BIT_32(0)
#define CBCR_CLK_OFF				BIT_32(31)
#define GDSCR_SW_COLLAPSE			BIT_32(0)
#define GDSCR_PWR_ON				BIT_32(31)
#define DEBUG_VOTE_RETENTION_ENABLE		BIT_32(0)

#define POLL_RETRIES				500U

static bool poll(uintptr_t reg, uint32_t mask, uint32_t val)
{
	uint32_t n;

	for (n = 0U; n < POLL_RETRIES; n++) {
		if ((mmio_read_32(reg) & mask) == val) {
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

	mmio_setbits_32(cbcr, CBCR_CLK_ENABLE);
	if (!poll(cbcr, CBCR_CLK_OFF, 0U)) {
		WARN("LPASS: clock 0x%lx did not turn on\n", cbcr);
	}

	return was_on;
}

/* Power a GDSC up; returns whether it was collapsed before. */
static bool gdsc_up(uintptr_t gdscr)
{
	if ((mmio_read_32(gdscr) & GDSCR_PWR_ON) != 0U) {
		return false;
	}

	mmio_clrbits_32(gdscr, GDSCR_SW_COLLAPSE);
	if (!poll(gdscr, GDSCR_PWR_ON, GDSCR_PWR_ON)) {
		WARN("LPASS: GDSC 0x%lx did not power up\n", gdscr);
	}

	return true;
}

static void gdsc_collapse(uintptr_t gdscr)
{
	mmio_setbits_32(gdscr, GDSCR_SW_COLLAPSE);
	if (!poll(gdscr, GDSCR_PWR_ON, 0U)) {
		WARN("LPASS: GDSC 0x%lx did not collapse\n", gdscr);
	}
}

/*
 * Secure LPASS setup for the ADSP, done once at cold boot: with the LPASS
 * audio and PDC hardware modules powered, vote for LPASS core retention and
 * release the LPASS boot IMEM. The bring-up clocks and power domains go back
 * to the state they were found in; the boot NoC and AXIS2 clocks stay off.
 */
void plat_qti_lpass_init(void)
{
	static const uintptr_t branches[] = {
		GCC_LPASS_AGGRE_NOC_DDRSS_SHUB_CBCR,
		LPASS_AUDIO_CC_SLIMBUS_CBCR,
		LPASS_AUDIO_CC_SLIMBUS_CORE_CBCR,
		LPASS_AON_CC_PDC_H_CBCR,
		LPASS_AON_CC_LPI_NOC_HS_CBCR,
	};
	bool was_on[ARRAY_SIZE(branches)];
	bool sway_on, audio_up, pdc_up;
	unsigned int i;

	sway_on = branch_on(GCC_LPASS_CFG_NOC_SWAY_CBCR);
	audio_up = gdsc_up(LPASS_AON_CC_AUDIO_HM_GDSCR);
	pdc_up = gdsc_up(LPASS_AON_CC_PDC_HM_GDSCR);
	for (i = 0U; i < ARRAY_SIZE(branches); i++) {
		was_on[i] = branch_on(branches[i]);
	}

	mmio_setbits_32(LPASS_CORE_GDSCR_DEBUG_VOTE,
			DEBUG_VOTE_RETENTION_ENABLE);

	/* No line of the boot IMEM may be written back once it is gone. */
	dcsw_op_all(DCCISW);
	mmio_write_32(TCSR_LPASS_BOOT_IMEM_DISABLE, 1U);
	dsbsy();

	mmio_clrbits_32(GCC_CONFIG_NOC_LPASS_BOOT_CBCR, CBCR_CLK_ENABLE);
	mmio_clrbits_32(GCC_LPASS_AXIS2_CBCR, CBCR_CLK_ENABLE);

	for (i = ARRAY_SIZE(branches); i > 0U; i--) {
		if (!was_on[i - 1U]) {
			mmio_clrbits_32(branches[i - 1U], CBCR_CLK_ENABLE);
		}
	}
	if (pdc_up) {
		gdsc_collapse(LPASS_AON_CC_PDC_HM_GDSCR);
	}
	if (audio_up) {
		gdsc_collapse(LPASS_AON_CC_AUDIO_HM_GDSCR);
	}
	if (!sway_on) {
		mmio_clrbits_32(GCC_LPASS_CFG_NOC_SWAY_CBCR, CBCR_CLK_ENABLE);
	}
}

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPU_PWR_H
#define QTI_CPU_PWR_H

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

/*
 * CPU power-up steps shared by the hoya and wildcat power code: the SAW4 rail
 * controller of a CPU rail and the IPM block of a core or a core complex.
 */

#define QTI_SAW4_VCTL			0x900U
#define QTI_SAW4_AVS_CTL		0x904U
#define QTI_SAW4_PMIC_STS		0xc18U

#define QTI_SAW4_AVS_CTL_EN		BIT_32(0)
#define QTI_SAW4_AVS_CTL_CTL_SEL	BIT_32(4)

#define QTI_SAW4_PMIC_STS_STATE_MASK	GENMASK_32(17, 16)
#define QTI_SAW4_PMIC_STS_DATA_MASK	GENMASK_32(15, 0)

/*
 * SAW4_VCTL holds SIZE[20], ADR_IDX[18:16] and PMIC_DATA[15:0]: the rail
 * enable is an 8-bit write of 0x80 to address index 3, the voltage a 16-bit
 * write of the millivolts to address index 0.
 */
#define QTI_SAW4_VCTL_ENABLE		0x30080U
#define QTI_SAW4_VCTL_SET_MV(mv)	(BIT_32(20) | (mv))

#define QTI_SAW4_PMIC_WRITE_RETRY	200U

#define QTI_IPM_HEAD_SWITCH_CTL	0x08U
#define QTI_IPM_SEQ_FORCE_PWR_CTL_EN	0x1cU
#define QTI_IPM_SEQ_FORCE_PWR_CTL_VAL	0x20U
#define QTI_IPM_PCHANNEL_FSM_CTL	0x44U

static inline bool qti_saw4_pmic_wait(uintptr_t saw4, uint32_t data,
				       bool check_data)
{
	unsigned int retry = QTI_SAW4_PMIC_WRITE_RETRY;

	while (retry-- > 0U) {
		uint32_t sts = mmio_read_32(saw4 + QTI_SAW4_PMIC_STS);

		if (((sts & QTI_SAW4_PMIC_STS_STATE_MASK) == 0U) &&
		    (!check_data ||
		     ((sts & QTI_SAW4_PMIC_STS_DATA_MASK) == data))) {
			return true;
		}

		udelay(2);
	}

	return false;
}

/* Enable a gold CPU rail through its SAW4 and set it to @mv. */
static inline bool qti_saw4_rail_on(uintptr_t saw4, uint32_t mv)
{
	/* Let the SAW hardware adjust the rail voltage. */
	mmio_setbits_32(saw4 + QTI_SAW4_AVS_CTL, QTI_SAW4_AVS_CTL_EN);
	mmio_setbits_32(saw4 + QTI_SAW4_AVS_CTL, QTI_SAW4_AVS_CTL_CTL_SEL);

	mmio_write_32(saw4 + QTI_SAW4_VCTL, QTI_SAW4_VCTL_ENABLE);
	if (!qti_saw4_pmic_wait(saw4, 0U, false)) {
		ERROR("gold SAW4 PMIC enable timed out\n");
		return false;
	}
	udelay(90);

	mmio_write_32(saw4 + QTI_SAW4_VCTL, QTI_SAW4_VCTL_SET_MV(mv));
	if (!qti_saw4_pmic_wait(saw4, mv, true)) {
		ERROR("gold SAW4 PMIC voltage set timed out\n");
		return false;
	}

	return true;
}

/*
 * IPM power-up of a core or a complex. @pchannel_req is true where the core
 * P-channel power-up request is raised around the reset release.
 */
static inline void qti_ipm_power_up(uintptr_t ipm, bool pchannel_req)
{
	/* Program skew between en_few and en_rest. */
	mmio_write_32(ipm + QTI_IPM_HEAD_SWITCH_CTL, 0x28U);

	/* Clear power-control enables. */
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_EN, 0x0U);

	/* Close the logic head switch, then drop the memory and logic clamps. */
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x642U);
	udelay(2);
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x402U);

	/* Deassert memory slp_nret_n, then slp_ret_n. */
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x40aU);
	udelay(4);
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x40eU);
	udelay(4);

	/* Assert and deassert wl_en_clk. */
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x50eU);
	udelay(2);
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x40eU);

	/* Deassert clock off. */
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x40cU);
	udelay(2);

	if (pchannel_req) {
		mmio_write_32(ipm + QTI_IPM_PCHANNEL_FSM_CTL, 0x1U);
	}

	/* Deassert reset. */
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x43cU);

	if (pchannel_req) {
		mmio_write_32(ipm + QTI_IPM_PCHANNEL_FSM_CTL, 0x0U);
	}

	/* Assert OSM core active, then CPU_PWRDUP. */
	mmio_write_32(ipm + QTI_IPM_SEQ_FORCE_PWR_CTL_VAL, 0x443cU);
	mmio_write_32(ipm + QTI_IPM_HEAD_SWITCH_CTL, 0x428U);
}

#endif /* QTI_CPU_PWR_H */

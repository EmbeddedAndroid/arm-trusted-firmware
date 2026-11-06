/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <pakala_plat.h>
#include <qti_xpu4.h>

/* LPASS LPI AON master port MPU, on the LPASS path to DDR */
#define LPASS_LPIAON_MPU_BASE		0x07e30000U
#define XPU4_QAD_LPASS_Q6		BIT_32(2)

/* LLCC LPI region (llcc-lpi@ff800000 in the SM8750 DT), end exclusive */
#define LLCC_LPI_BASE			0xff800000ULL
#define LLCC_LPI_END			0x100000000ULL

/*
 * The LPI AON MPU denies the LPASS Q6 and the apps domain its traffic carries
 * any access outside its enabled resource groups. The boot firmware enables
 * none over the LLCC LPI region: take the first group it leaves disabled.
 */
void pakala_lpass_lpi_mpu_open(void)
{
	const uintptr_t b = LPASS_LPIAON_MPU_BASE;
	const uint32_t qads = XPU4_QAD_APPS | XPU4_QAD_NS | XPU4_QAD_LPASS_Q6;
	unsigned int rg, nrg = XPU4_IDR0_NRG(mmio_read_32(b + XPU4_IDR0));

	for (rg = 0U; rg < nrg; rg++) {
		if ((mmio_read_32(b + XPU4_RGCR1(rg)) & XPU4_RGCR1_RGE) == 0U) {
			break;
		}
	}
	if (rg == nrg) {
		ERROR("LPASS: no free LPI AON MPU group, LLCC LPI left closed\n");
		return;
	}

	mmio_write_32(b + XPU4_RGCSAR0(rg), (uint32_t)LLCC_LPI_BASE);
	mmio_write_32(b + XPU4_RGCSAR1(rg), (uint32_t)(LLCC_LPI_BASE >> 32));
	mmio_write_32(b + XPU4_RGCEAR0(rg), (uint32_t)LLCC_LPI_END);
	mmio_write_32(b + XPU4_RGCEAR1(rg), (uint32_t)(LLCC_LPI_END >> 32));
	mmio_write_32(b + XPU4_RGRDR(rg), qads);
	mmio_write_32(b + XPU4_RGWRR(rg), qads);
	mmio_setbits_32(b + XPU4_RGCR1(rg), XPU4_RGCR1_RGE);
	dsbsy();

	if (((mmio_read_32(b + XPU4_RGCR1(rg)) & XPU4_RGCR1_RGE) == 0U) ||
	    (mmio_read_32(b + XPU4_RGRDR(rg)) != qads) ||
	    (mmio_read_32(b + XPU4_RGWRR(rg)) != qads)) {
		ERROR("LPASS: LPI AON MPU RG %u did not take the LLCC LPI\n",
		      rg);
		return;
	}

	INFO("LPASS: LLCC LPI open to the ADSP (LPI AON MPU RG %u)\n", rg);
}

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <pakala_plat.h>
#include <platform_def.h>
#include <qti_plat.h>

/* Resets the AOSS halts the SoC on for the debug image instead. */
#define AOSS_CC_RESET_DEBUG_ENABLE	0x0c2f0000U
#define SECURE_WDOG_RESET_DEBUG_EN	BIT_32(6)
#define MSM_TSENSE_RESET_DEBUG_EN	BIT_32(5)
#define PMIC_RESIN_RESET_DEBUG_EN	BIT_32(4)

void plat_qti_bl31_setup_post(void)
{
#if !QTI_RAMDUMP
	/*
	 * XBL leaves these resets on the debug path, which ends in RAM dump
	 * mode instead of a reboot.
	 */
	mmio_clrbits_32(AOSS_CC_RESET_DEBUG_ENABLE,
			SECURE_WDOG_RESET_DEBUG_EN | MSM_TSENSE_RESET_DEBUG_EN |
			PMIC_RESIN_RESET_DEBUG_EN);
#endif

	pakala_pdp_calcomp();
}

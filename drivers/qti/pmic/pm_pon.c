/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <pm_pon.h>
#include <spmi_arb.h>

#include <qti_plat.h>

/*
 * This driver implements PON support for PM8998-compatible PMICs. This can
 * include other part numbers like PM6150.
 */

void pm_app_ps_hold_cfg(enum pon_reset_type reset_type)
{
	int ret;

	/* QTI recommends disabling reset for 10 cycles before reconfiguring. */
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL2, 0);
	mdelay(1);

	ret = spmi_arb_write8(PON_PS_HOLD_RESET_CTL, reset_type);
	if (ret != 0) {
		VERBOSE("PON: failed to set reset type 0x%x: %d\n",
			reset_type, ret);
	}

	/* Re-arm S2_RESET_EN regardless, to restore the default config. */
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL2, S2_RESET_EN);
	mdelay(1);
}

void qti_pmic_prepare_reset(void)
{
	pm_app_ps_hold_cfg(RESET_TYPE_HARD_RESET);
}

void qti_pmic_prepare_shutdown(void)
{
	pm_app_ps_hold_cfg(RESET_TYPE_SHUTDOWN);
}

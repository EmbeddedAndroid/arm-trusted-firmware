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
#include <qti_reset.h>

#ifndef PON_PS_HOLD_SYSTEM_RESET_TYPE
#define PON_PS_HOLD_SYSTEM_RESET_TYPE	RESET_TYPE_HARD_RESET
#endif

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
	pm_app_ps_hold_cfg(PON_PS_HOLD_SYSTEM_RESET_TYPE);
}

void qti_pmic_prepare_shutdown(void)
{
	pm_app_ps_hold_cfg(RESET_TYPE_SHUTDOWN);
}

void qti_pmic_configure_reset(uint32_t reset_config)
{
	pm_app_ps_hold_cfg((reset_config == QTI_PON_CFG_HARD_RESET) ?
			   RESET_TYPE_HARD_RESET : RESET_TYPE_WARM_RESET);
}

/*
 * On some platforms the boot loader leaves the PON interrupts (power key,
 * RESIN) sent to an SPMI master other than the apps PMIC arbiter, so the OS
 * never sees them.
 */
void qti_pmic_route_pon_irqs(void)
{
#ifdef PON_INT_MID_SEL
	(void)spmi_arb_write8(PON_INT_MID_SEL, PON_INT_MID_APPS);
#endif
}

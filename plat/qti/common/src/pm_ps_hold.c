/*
 * Copyright (c) 2020, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>

#include <qti_plat.h>
#include <qti_reset.h>
#include <spmi_arb.h>

/*
 * This driver implements PON support for PM8998-compatible PMICs. This can
 * include other part numbers like PM6150.
 */

#define S2_RESET_EN			BIT(7)

static void configure_ps_hold(uint32_t reset_type)
{
	/* QTI recommends disabling reset for 10 cycles before reconfiguring. */
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL2, 0);
	mdelay(1);

	spmi_arb_write8(PON_PS_HOLD_RESET_CTL, reset_type);
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL2, S2_RESET_EN);
	mdelay(1);
}

void qti_pmic_prepare_reset(void)
{
	configure_ps_hold(PON_PS_HOLD_SYSTEM_RESET_TYPE);
}

void qti_pmic_prepare_shutdown(void)
{
	configure_ps_hold(PON_RESET_TYPE_SHUTDOWN);
}

void qti_pmic_configure_reset(uint32_t reset_config)
{
	configure_ps_hold((reset_config == QTI_PON_CFG_HARD_RESET) ?
			  PON_RESET_TYPE_HARD_RESET : PON_RESET_TYPE_WARM_RESET);
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

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_internal.h>
#include <lib/utils_def.h>

/* PDC interrupt mapping for QCS615 (Talos) APSS */
struct pdc_interrupt_mapping g_pdc_interrupt_mapping[] = {
	/* Bit 0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 512 }, /* rpmh_wake */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 513 }, /* ee0_apps_hlos_spmi_periph_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 514 }, /* ee1_apps_trustzone_spmi_periph_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV0 }, 515 }, /* secure_wdog_expired */

	/* Bit 4 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV0 }, 516 }, /* secure_wdog_bark_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 517 }, /* aop_wdog_expired_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 518 }, /* not-connected */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 519 }, /* not-connected */

	/* Bit 8 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 520 }, /* eud_p0_dmse_int_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 521 }, /* eud_p0_dpse_int_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 522 }, /* eud_p1_dmse_int_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 523 }, /* eud_p1_dpse_int_mx */

	/* Bit 12 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 524 }, /* eud_int_mx[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 525 }, /* ssc_xpu_irq_summary */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 526 }, /* wd_bite_apps */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 527 }, /* ssc_vmidmt_irq_summary */

	/* Bit 16 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 528 }, /* sdc_gpo[0][4] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 529 }, /* not-connected */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 530 }, /* aoss_pmic_arb_mpu_xpu_summary_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 531 }, /* rpmh_wake_2 */

	/* Bit 20 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 532 }, /* apps_pdc_irq_in_20 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 533 }, /* apps_pdc_irq_in_21 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 534 }, /* pdc_apps_epcb_timerout_summary_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 535 }, /* spmi_protocol_irq */

	/* Bit 24 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 536 }, /* tsense0_tsense_max_min_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 537 }, /* tsense1_tsense_max_min_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 538 }, /* tsense0_upper_lower_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 539 }, /* tsense1_upper_lower_intr */

	/* Bit 28 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 540 }, /* tsense0_critical_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 541 }, /* tsense1_critical_intr */
};

const uint32_t g_pdc_interrupt_table_size = ARRAY_SIZE(g_pdc_interrupt_mapping);

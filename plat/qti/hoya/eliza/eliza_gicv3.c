/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/gicv3.h>
#include <lib/utils_def.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * Only the SGIs BL31 itself uses are secure. The secure watchdog bark and the
 * bus error interrupts stay non-secure until their SM7750 interrupt IDs are
 * established.
 */
static const interrupt_prop_t qti_interrupt_props[] = {
	INTR_PROP_DESC(PLAT_INT_ID_CPU_WAKEUP_SGI,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(PLAT_INT_ID_RESET_SGI, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
};

const interrupt_prop_t *plat_qti_get_interrupt_props(unsigned int *num_props)
{
	*num_props = ARRAY_SIZE(qti_interrupt_props);
	return qti_interrupt_props;
}

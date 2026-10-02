/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/gicv3.h>
#include <lib/utils_def.h>

#include <platform_def.h>
#include <qti_plat.h>

/* The memory protection error summaries are the only EL3 interrupts. */
static const interrupt_prop_t qti_interrupt_props[] = {
	INTR_PROP_DESC(PLAT_INT_ID_XPU_SEC, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(PLAT_INT_ID_XPU_NON_SEC, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(PLAT_INT_ID_VMIDMT_ERR_CLT_SEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(PLAT_INT_ID_VMIDMT_ERR_CLT_NONSEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(PLAT_INT_ID_VMIDMT_ERR_CFG_SEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(PLAT_INT_ID_VMIDMT_ERR_CFG_NONSEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
};

const interrupt_prop_t *plat_qti_get_interrupt_props(unsigned int *num_props)
{
	*num_props = ARRAY_SIZE(qti_interrupt_props);
	return qti_interrupt_props;
}

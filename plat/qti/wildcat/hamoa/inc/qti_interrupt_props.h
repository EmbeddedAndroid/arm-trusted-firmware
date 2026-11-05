/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_INTERRUPT_PROPS_H
#define QTI_INTERRUPT_PROPS_H

#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include <watchdog_defs.h>

#include <platform_def.h>

static const interrupt_prop_t qti_interrupt_props[] = {
	INTR_PROP_DESC(PLAT_INT_ID_CPU_WAKEUP_SGI, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(PLAT_INT_ID_RESET_SGI, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(WDOG_BARK_INT_ID, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0, GIC_INTR_CFG_EDGE),
};

#endif /* QTI_INTERRUPT_PROPS_H */

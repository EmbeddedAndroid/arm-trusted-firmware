/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_CPU_CONFIG_H
#define QTI_CPUCP_CPU_CONFIG_H

#include <stdint.h>

#include <clkdom_config.h>
#include <cpucp_hwio.h>
#include <lib/mmio.h>

struct clkdom_cpumask {
	unsigned int clkdom;
	unsigned int cpumask;
};

extern struct clkdom_cpumask clkdom_cpumasks[CD_MAX];
extern unsigned int clkdom_init_status[CD_MAX];

/*
 * Whether clock domain @clkdom is up: 1 if it is, 0 if not yet, -1 where the
 * chipset describes no up indication (CPUCP_CLKDOM_UP_REG() and
 * CPUCP_CLKDOM_UP_BIT in its cpucp_hwio.h).
 */
static inline int cpucp_clkdom_up(unsigned int clkdom)
{
#ifdef CPUCP_CLKDOM_UP_REG
	return ((mmio_read_32(CPUCP_CLKDOM_UP_REG(clkdom)) &
		 CPUCP_CLKDOM_UP_BIT) != 0U) ? 1 : 0;
#else
	(void)clkdom;
	return -1;
#endif
}

#endif /* QTI_CPUCP_CPU_CONFIG_H */

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cpu_config.h>

struct clkdom_cpumask clkdom_cpumasks[CD_MAX] = {
	{ CD_L3,       0x00U },
	{ CD_SILVER,   0x07U },
	{ CD_GOLD,     0x78U },
	{ CD_GOLDPLUS, 0x80U },
};

/* Every domain is enabled through CPUCP when its first core comes up. */
unsigned int clkdom_init_status[CD_MAX];

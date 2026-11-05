/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cpu_config.h>

/*
 * BL31 requests no clock domain on X1E80100: CPUCP brings a cluster's domain
 * up with the first core it powers there.
 */
struct clkdom_cpumask clkdom_cpumasks[CD_MAX] = {
	{ CD_CLUSTER0, 0x00U },
	{ CD_CLUSTER1, 0x00U },
	{ CD_CLUSTER2, 0x00U },
};

unsigned int clkdom_init_status[CD_MAX] = { 1U, 1U, 1U };

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_KAILUA_CLKDOM_CONFIG_H
#define QTI_CPUCP_KAILUA_CLKDOM_CONFIG_H

/*
 * In CPUCP SCMI clock ID order: the L3, the Cortex-A510 cores 0 to 2, the
 * Cortex-A715 and Cortex-A710 cores 3 to 6 and the Cortex-X3 core 7.
 */
enum clock_domain_id {
	CD_L3,
	CD_SILVER,
	CD_GOLD,
	CD_GOLDPLUS,
	CD_MAX
};

#endif /* QTI_CPUCP_KAILUA_CLKDOM_CONFIG_H */

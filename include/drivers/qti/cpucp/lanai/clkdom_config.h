/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_LANAI_CLKDOM_CONFIG_H
#define QTI_CPUCP_LANAI_CLKDOM_CONFIG_H

/*
 * In CPUCP SCMI clock ID order: the L3, the Cortex-A520 cores 0 and 1, the
 * APC1 Cortex-A720 cores 5 and 6, the Cortex-X4 core 7 and the APC2
 * Cortex-A720 cores 2 to 4.
 */
enum clock_domain_id {
	CD_L3,
	CD_SILVER,
	CD_GOLD,
	CD_GOLDPLUS,
	CD_APC2GOLD,
	CD_MAX
};

#endif /* QTI_CPUCP_LANAI_CLKDOM_CONFIG_H */

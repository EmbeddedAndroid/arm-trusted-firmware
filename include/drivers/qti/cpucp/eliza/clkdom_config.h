/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_ELIZA_CLKDOM_CONFIG_H
#define QTI_CPUCP_ELIZA_CLKDOM_CONFIG_H

/*
 * In CPUCP SCMI clock ID order: the L3, the Cortex-A520 cores 0 to 2, the
 * Cortex-A720 cores 3 to 6 and the Cortex-A720 core 7 (the Linux EPSS
 * frequency domains 0, 1 and 2).
 */
enum clock_domain_id {
	CD_L3,
	CD_SILVER,
	CD_GOLD,
	CD_GOLDPLUS,
	CD_MAX
};

#endif /* QTI_CPUCP_ELIZA_CLKDOM_CONFIG_H */

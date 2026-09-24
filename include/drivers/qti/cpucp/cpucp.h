/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_H
#define QTI_CPUCP_H

/*
 * Lean CPUCP host interface for TF-A. Only the clock-domain enable path that
 * is required during secondary-core cold boot and, with QTI_CPUCP_START,
 * starting the CPUCP firmware are implemented natively. The full CPUCP
 * firmware-load / DCVS machinery still lives in qtiseclib.
 */

/* Request to enable the clock domain owning the calling core on cold boot. */
void cpucp_clkdom_init(void);

#ifdef QTI_CPUCP_START_ENABLED
/* Start the CPUCP firmware the boot firmware placed in EPSS memory. */
void qti_cpucp_init(void);
#else
static inline void qti_cpucp_init(void)
{
}
#endif

#endif /* QTI_CPUCP_H */

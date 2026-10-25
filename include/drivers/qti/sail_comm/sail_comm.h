/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SAIL_COMM_H
#define SAIL_COMM_H

#ifdef QTI_SAIL_COMM_ENABLE
void qti_sail_notify_shutdown(void);
void qti_sail_notify_reset(void);
void qti_sail_notify_warm_reset(void);
void qti_sail_notify_reset2(uint32_t reset_config);
#else
static inline void qti_sail_notify_shutdown(void) {}
static inline void qti_sail_notify_reset(void) {}
static inline void qti_sail_notify_warm_reset(void) {}
static inline void qti_sail_notify_reset2(uint32_t reset_config)
{
	(void)reset_config;
}
#endif

#endif /* SAIL_COMM_H */

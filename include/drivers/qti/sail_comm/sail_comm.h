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
#else
static inline void qti_sail_notify_shutdown(void) {}
static inline void qti_sail_notify_reset(void) {}
#endif

#endif /* SAIL_COMM_H */

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_EUD_H
#define QTI_EUD_H

#ifdef QTI_EUD_ENABLE
void qti_eud_enable(void);
#else
static inline void qti_eud_enable(void) {}
#endif

#endif /* QTI_EUD_H */

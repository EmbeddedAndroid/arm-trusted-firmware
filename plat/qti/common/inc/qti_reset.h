/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_RESET_H
#define QTI_RESET_H

#include <stdint.h>

#ifndef QTI_RESET2_COOKIE_STORE_ENABLE
#define QTI_RESET2_COOKIE_STORE_ENABLE	0
#endif

#define QTI_PON_CFG_WARM_RESET		0U
#define QTI_PON_CFG_HARD_RESET		1U
#define QTI_RESET2_BOOT_MISC_DETECT_REG	0x01FD3000U

#if QTI_RESET2_COOKIE_STORE_ENABLE
void qti_reset_write_sdam_cookie(uint8_t cookie);
void qti_reset_write_gpio_tcsr(uint32_t val);
#else
static inline void qti_reset_write_sdam_cookie(uint8_t cookie)
{
	(void)cookie;
}

static inline void qti_reset_write_gpio_tcsr(uint32_t val)
{
	(void)val;
}
#endif

#endif /* QTI_RESET_H */

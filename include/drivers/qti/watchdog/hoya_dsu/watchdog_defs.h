/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H

#include <lib/utils_def.h>

#include <platform_def.h>

/* Secure APSS watchdog (TMR2); TMR1 is the non-secure one. */
#define WDOG_BARK_INT_ID		54

#define WDOG_REG_BASE			(QTI_APSS_HM_BASE + 0x00411000)
#define WDOG_RESET_ADDR			(WDOG_REG_BASE + 0x4)
#define RESET				0x1
#define WDOG_CTL_ADDR			(WDOG_REG_BASE + 0x8)
/* Count, and raise the bark interrupt. */
#define ENABLE_BIT			(BIT(0) | BIT(1))
#define CLK_ENABLE_BIT			0U
#define HW_SLEEP_WAKEUP_EN		0U
#define CHIP_AUTOPET_EN			0U
#define WDOG_BARK_ADDR			(WDOG_REG_BASE + 0x10)
#define WDOG_BARK_MASK			0xfffff
#define BARK_SYNC_BIT			0U
#define WDOG_BITE_ADDR			(WDOG_REG_BASE + 0x14)
#define WDOG_BITE_MASK			0xfffff
#define BITE_SYNC_BIT			0U

#endif /* WATCHDOG_DEFS_H */

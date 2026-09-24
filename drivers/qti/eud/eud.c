/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/eud/eud.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>

/*
 * The EUD register space is three 4 KiB blocks: the Acore CSR at
 * QTI_EUD_BASE, the mode manager, and the secure mode manager, which
 * only accepts secure writes.
 */
#define EUD_MODE_MGR_BASE		(QTI_EUD_BASE + 0x1000U)
#define EUD_MODE_MGR2_BASE		(QTI_EUD_BASE + 0x2000U)

#define EUD_CSR_EUD_EN			(EUD_MODE_MGR_BASE + 0x14U)
#define EUD_EUD_STATUS			(EUD_MODE_MGR_BASE + 0x20U)
#define EUD_PBUS_CTRL_EN		(EUD_MODE_MGR_BASE + 0x24U)
#define EUD_EUD_EN2			(EUD_MODE_MGR2_BASE + 0x0U)

#define EUD_ENABLE			BIT_32(0)

#define CBCR_CLK_ENABLE			BIT_32(0)
#define CBCR_HW_CTL			BIT_32(1)
#define CBCR_CLK_OFF			BIT_32(31)
#define CBCR_POLL_COUNT			1000U

static bool eud_ahb_clk_enable(void)
{
	unsigned int count;
	uint32_t cbcr;

	mmio_setbits_32(QTI_EUD_AHB_CBCR, CBCR_CLK_ENABLE);

	for (count = 0U; count < CBCR_POLL_COUNT; count++) {
		cbcr = mmio_read_32(QTI_EUD_AHB_CBCR);
		/* A hardware gated branch can read as off while it is idle. */
		if (((cbcr & CBCR_CLK_OFF) == 0U) ||
		    ((cbcr & CBCR_HW_CTL) != 0U)) {
			return true;
		}
	}

	return false;
}

void qti_eud_enable(void)
{
	if (!eud_ahb_clk_enable()) {
		WARN("EUD: AHB clock is off, debug mode not enabled\n");
		return;
	}

	/* EUD_EN2 is in the secure block, hence enabling EUD from EL3. */
	mmio_write_32(EUD_EUD_EN2, EUD_ENABLE);
	mmio_write_32(EUD_CSR_EUD_EN, EUD_ENABLE);

	INFO("EUD: debug mode enabled (status 0x%x, PHY tuning %s)\n",
	     mmio_read_32(EUD_EUD_STATUS),
	     ((mmio_read_32(EUD_PBUS_CTRL_EN) & EUD_ENABLE) != 0U) ?
	     "loaded" : "not loaded");
}

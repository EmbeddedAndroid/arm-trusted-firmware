/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for kodiak (qcs6490): the secure UFS GDSC
 * retention vote.
 */

#include <drivers/qti/clock/clock_driver.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define GCC_APCS_TZ_UFS_PHY_SGDSCR	0x00153008U
#define SGDSCR_RETAIN_FF_ENABLE		BIT_32(1)
#define SGDSCR_SW_OVERRIDE		BIT_32(0)

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	/*
	 * The OS collapses the UFS GDSC and resumes the link from hibern8,
	 * which needs the host state retained. SW_OVERRIDE stays clear, as
	 * in QTISECLIB.
	 */
	mmio_clrsetbits_32(GCC_APCS_TZ_UFS_PHY_SGDSCR, SGDSCR_SW_OVERRIDE,
			   SGDSCR_RETAIN_FF_ENABLE);

	return 0;
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	return 0;
}

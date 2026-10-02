/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <qti_plat.h>

#define TCSR_BOOT_IMEM_DISABLE			0x003d3300
#define GCC_MSS_AXIS2_CBCR			0x0143300c
#define GCC_MSS_Q6SS_BOOT_GPLL0_MUXR		0x0143314c
#define GCC_MSS_Q6_BIMC_AXI_CBCR		0x01433154
#define GCC_MSS_Q6_BIMC_AXI_PIPE_CBCR		0x01433160
#define GCC_BIMC_MSS_Q6_AXI_CBCR		0x0148a164
#define GCC_MSS_RESTART				0x014a0000

#define CBCR_CLK_ENABLE				BIT_32(0)

/*
 * XBL leaves the modem memory enabled as boot IMEM. Release it, switch the
 * modem Q6 boot clock back to XO and restart MSS, as the modem otherwise
 * takes a bus error on the memory NoC when it boots. Then enable the modem
 * Q6 AXI clocks to the DDR controller.
 */
static void agatti_mss_boot_release(void)
{
	mmio_setbits_32(TCSR_BOOT_IMEM_DISABLE, BIT_32(0));
	mmio_clrbits_32(GCC_MSS_Q6SS_BOOT_GPLL0_MUXR, BIT_32(0));
	mmio_clrbits_32(GCC_MSS_AXIS2_CBCR, CBCR_CLK_ENABLE);

	mmio_write_32(GCC_MSS_RESTART, 1U);
	udelay(200);
	mmio_write_32(GCC_MSS_RESTART, 0U);

	mmio_setbits_32(GCC_BIMC_MSS_Q6_AXI_CBCR, CBCR_CLK_ENABLE);
	mmio_setbits_32(GCC_MSS_Q6_BIMC_AXI_CBCR, CBCR_CLK_ENABLE);
	mmio_setbits_32(GCC_MSS_Q6_BIMC_AXI_PIPE_CBCR, CBCR_CLK_ENABLE);
}

void plat_qti_bl31_setup_post(void)
{
	agatti_mss_boot_release();
}

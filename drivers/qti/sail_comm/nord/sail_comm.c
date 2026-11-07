/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/qti/sail_comm/sail_comm.h>
#include <lib/mmio.h>
#include <lib/mmio_poll.h>
#include <platform_def.h>

#define QTI_SAIL_STATUS_TIMEOUT_US		1000000U

/* Messages from the secure world that SAIL acknowledges. */
#define QTI_SAIL_MD_GRACEFUL_SHUTDOWN		0xB2200430U
#define QTI_SAIL_MD_SOC_HR			0xB2200510U
#define QTI_SAIL_RESET_READY			0xA0300100U

/* TCSR mailbox: one 32-bit word each way, then the SMSS TZ IPC doorbell. */
#define QTI_MAIN2SAIL_STATUS8_REG		(QTI_CORE_TOP_CSR_BASE + 0x7a02cU)
#define QTI_SAIL2MAIN_SHADOW_STATUS4_REG	(QTI_CORE_TOP_CSR_BASE + 0x7b014U)
#define QTI_SAIL_TZ_IPC_INTERRUPT_REG		(QTI_APSS_HM_BASE + 0x00824004U)
#define QTI_SAIL_TZ_IPC_SMSS_MASK		0x00080000U

static void qti_sail_comm_notify(uint32_t status_to_sail)
{
	uint32_t val;
	int ret;

	mmio_write_32(QTI_MAIN2SAIL_STATUS8_REG, status_to_sail);
	dsb();
	isb();
	mmio_write_32(QTI_SAIL_TZ_IPC_INTERRUPT_REG, QTI_SAIL_TZ_IPC_SMSS_MASK);

	ret = mmio_read_32_poll_timeout(QTI_SAIL2MAIN_SHADOW_STATUS4_REG, val,
					val == QTI_SAIL_RESET_READY,
					QTI_SAIL_STATUS_TIMEOUT_US);
	if (ret != 0) {
		ERROR("SAIL did not acknowledge status 0x%x, last response 0x%x\n",
		      status_to_sail, val);
	}
}

void qti_sail_notify_shutdown(void)
{
	qti_sail_comm_notify(QTI_SAIL_MD_GRACEFUL_SHUTDOWN);
}

void qti_sail_notify_reset(void)
{
	qti_sail_comm_notify(QTI_SAIL_MD_SOC_HR);
}

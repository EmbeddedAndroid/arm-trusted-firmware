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

#define QTI_SAIL_RESET_READY			0xAA030000U
#define QTI_SAIL_MD_GRACEFUL_SHUTDOWN		0xCD060000U
#define QTI_SAIL_MD_SOC_HR			0xCD070000U

#define QTI_SAIL_TCSR_REG_BASE			(QTI_CORE_TOP_CSR_BASE + 0x000c0000U)
#define QTI_SAIL2MAIN_STATUS4_REG		(QTI_SAIL_TCSR_REG_BASE + 0x3d018U)
#define QTI_SAIL2MAIN_STATUS5_REG		(QTI_SAIL_TCSR_REG_BASE + 0x3d01cU)
#define QTI_SAIL2MAIN_STATUS6_REG		(QTI_SAIL_TCSR_REG_BASE + 0x3d020U)
#define QTI_SAIL2MAIN_STATUS7_REG		(QTI_SAIL_TCSR_REG_BASE + 0x3d024U)
#define QTI_MAIN2SAIL_STATUS4_REG		(QTI_SAIL_TCSR_REG_BASE + 0x3e02cU)
#define QTI_MAIN2SAIL_STATUS5_REG		(QTI_SAIL_TCSR_REG_BASE + 0x3e030U)
#define QTI_MAIN2SAIL_STATUS6_REG		(QTI_SAIL_TCSR_REG_BASE + 0x3e034U)
#define QTI_MAIN2SAIL_STATUS7_REG		(QTI_SAIL_TCSR_REG_BASE + 0x3e038U)

#define QTI_SAIL_IPC_INTERRUPT_REG		(QTI_APSS_HM_BASE + 0x00400008U)
#define QTI_SAIL_IPC_SMSS_TZ_IPC		0x00100000U

static uint32_t qti_sail_comm_read_status(uint32_t status4)
{
	uint32_t status;

	status = mmio_read_32(QTI_SAIL2MAIN_STATUS7_REG) & 0xffU;
	status = (status << 8) |
		 (mmio_read_32(QTI_SAIL2MAIN_STATUS6_REG) & 0xffU);
	status = (status << 8) |
		 (mmio_read_32(QTI_SAIL2MAIN_STATUS5_REG) & 0xffU);
	status = (status << 8) | (status4 & 0xffU);

	return status;
}

static void qti_sail_comm_write_status(uint32_t status)
{
	mmio_write_32(QTI_MAIN2SAIL_STATUS7_REG, (status >> 24) & 0xffU);
	mmio_write_32(QTI_MAIN2SAIL_STATUS6_REG, (status >> 16) & 0xffU);
	mmio_write_32(QTI_MAIN2SAIL_STATUS5_REG, (status >> 8) & 0xffU);
	mmio_write_32(QTI_MAIN2SAIL_STATUS4_REG, status & 0xffU);

	dsb();
	isb();

	mmio_write_32(QTI_SAIL_IPC_INTERRUPT_REG, QTI_SAIL_IPC_SMSS_TZ_IPC);
}

static void qti_sail_comm_notify(uint32_t status_to_sail)
{
	int ret;
	uint32_t val;

	qti_sail_comm_write_status(status_to_sail);

	ret = mmio_read_32_poll_timeout(QTI_SAIL2MAIN_STATUS4_REG, val,
					qti_sail_comm_read_status(val) ==
					QTI_SAIL_RESET_READY,
					QTI_SAIL_STATUS_TIMEOUT_US);
	if (ret != 0) {
		ERROR("SAIL did not acknowledge status 0x%x, last response 0x%x\n",
		      status_to_sail, qti_sail_comm_read_status(val));
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

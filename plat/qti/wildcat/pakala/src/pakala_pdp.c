/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <pakala_plat.h>
#include <qti_plat.h>

/*
 * Each cluster's power domain processor (PDP) serves a QCOM SCMI vendor
 * protocol channel to the non-secure world: an SCMI shared memory area in
 * the PDP_NS region (pdp-ns-shared@81f00000) and a doorbell in the PDP's
 * mailbox. Once told that DVFS calibration is complete ("calcomp", message
 * 19), the PDP passes it on to CPUCP, which only then services the SCMI
 * performance fastchannels.
 */
#define PDP_NS_SHMEM(n)			(0x81f810c0UL + (0x60UL * (n)))
#define PDP_NS_SHMEM_PAGE		0x81f81000UL
#define PDP_MBOX_TX_CMD(n)		(0x18980104UL + (0x1000000UL * (n)))
#define PDP_COUNT			2U

#define SHMEM_CHANNEL_STATUS		0x04U
#define SHMEM_FLAGS			0x10U
#define SHMEM_LENGTH			0x14U
#define SHMEM_HEADER			0x18U
#define SHMEM_PAYLOAD			0x1cU
#define SHMEM_CHANNEL_FREE		BIT_32(0)

#define SCMI_PROTOCOL_QCOM_VENDOR	0x80U
#define SCMI_MSG_HDR(proto, msg)	(((proto) << 10) | (msg))
#define QCOM_VENDOR_CALCOMP_MSG		19U
/* "calcomp" as the little-endian algorithm identifier */
#define QCOM_VENDOR_ALGO_CALCOMP	0x00706d6f636c6163ULL
/* buf_len, algorithm (u64), param_id */
#define QCOM_VENDOR_REQ_LEN		(4U + 16U)

/* CPUCP perf fastchannel interrupt enables, per performance domain */
#define CPUCP_FC_IRQ_EN(n)		(0x1784206cUL + (0x100UL * (n)))

#define POLL_US				100000U

static bool poll_set(uintptr_t reg, uint32_t mask)
{
	uint32_t n;

	for (n = 0U; n < POLL_US; n += 10U) {
		if ((mmio_read_32(reg) & mask) != 0U) {
			return true;
		}
		udelay(10U);
	}

	return false;
}

static int pdp_calcomp(unsigned int n)
{
	const uintptr_t ch = PDP_NS_SHMEM(n);

	mmio_write_32(ch + SHMEM_CHANNEL_STATUS, 0U);
	mmio_write_32(ch + SHMEM_FLAGS, 0U);
	mmio_write_32(ch + SHMEM_LENGTH, QCOM_VENDOR_REQ_LEN);
	mmio_write_32(ch + SHMEM_HEADER,
		      SCMI_MSG_HDR(SCMI_PROTOCOL_QCOM_VENDOR,
				   QCOM_VENDOR_CALCOMP_MSG));
	mmio_write_32(ch + SHMEM_PAYLOAD, 0U);
	mmio_write_32(ch + SHMEM_PAYLOAD + 0x4U,
		      (uint32_t)QCOM_VENDOR_ALGO_CALCOMP);
	mmio_write_32(ch + SHMEM_PAYLOAD + 0x8U,
		      (uint32_t)(QCOM_VENDOR_ALGO_CALCOMP >> 32));
	mmio_write_32(ch + SHMEM_PAYLOAD + 0xcU, 0U);
	dsbsy();
	mmio_write_32(PDP_MBOX_TX_CMD(n), 1U);

	if (!poll_set(ch + SHMEM_CHANNEL_STATUS, SHMEM_CHANNEL_FREE)) {
		return -ETIMEDOUT;
	}
	if (mmio_read_32(ch + SHMEM_PAYLOAD) != 0U) {
		return -EIO;
	}
	if (!poll_set(CPUCP_FC_IRQ_EN(n), BIT_32(0))) {
		return -EAGAIN;
	}

	return 0;
}

/*
 * Report DVFS calibration complete to both PDPs. The UEFI of the SM8750
 * boot binaries does it before it exits boot services; without it CPUCP
 * never arms the perf fastchannel interrupt and ignores every fastchannel
 * level request.
 */
void pakala_pdp_calcomp(void)
{
	unsigned int n;
	int ret;

	if (qti_mmap_add_dynamic_region(PDP_NS_SHMEM_PAGE, PAGE_SIZE,
					MT_DEVICE | MT_RW | MT_NS) != 0) {
		ERROR("PDP: cannot map the shared memory\n");
		return;
	}

	for (n = 0U; n < PDP_COUNT; n++) {
		ret = pdp_calcomp(n);
		if (ret != 0) {
			WARN("PDP%u: calcomp failed: %d\n", n, ret);
		}
	}

	(void)qti_mmap_remove_dynamic_region(PDP_NS_SHMEM_PAGE, PAGE_SIZE);
}

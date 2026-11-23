/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/smem/smem.h>
#include <lib/cassert.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <qti_llcc_mpu.h>
#include <qti_xpu4.h>

static uint64_t xpu4_read64(uintptr_t hi, uintptr_t lo)
{
	return ((uint64_t)mmio_read_32(hi) << 32) | mmio_read_32(lo);
}

/* The enabled LLCC MPU resource group protecting exactly [start, end), or -1 */
static int llcc_mpu_find_rg(uint64_t start, uint64_t end)
{
	const uintptr_t b = QTI_LLCC_MPU_BASE;
	unsigned int n, nrg = MIN(XPU4_IDR0_NRG(mmio_read_32(b + XPU4_IDR0)),
				  XPU4_NRG_MAPPED(QTI_LLCC_MPU_SIZE));

	for (n = 0U; n < nrg; n++) {
		if (((mmio_read_32(b + XPU4_RGCR1(n)) & XPU4_RGCR1_RGE) != 0U) &&
		    (xpu4_read64(b + XPU4_RGCSAR1(n), b + XPU4_RGCSAR0(n)) ==
		     start) &&
		    (xpu4_read64(b + XPU4_RGCEAR1(n), b + XPU4_RGCEAR0(n)) ==
		     end)) {
			return (int)n;
		}
	}

	return -1;
}

/*
 * The ADSP's traffic carries the apps non-secure access domain, which the
 * boot firmware leaves out of the resource groups of the ADSP's partitions.
 * A partition the ADSP shares with TZ is never opened. The AND broadcast
 * instance programs the MPU of every LLCC channel.
 */
void qti_llcc_mpu_open_adsp_smem(void)
{
	const uint32_t qads = XPU4_QAD_APPS | XPU4_QAD_NS;
	uint32_t i, off, size;
	uint16_t adsp, tz, h0, h1;
	int ret, rg;

	ret = qti_smem_host_id(QTI_SMEM_PROC_ADSP, 0U, 0U, 0U, &adsp);
	if (ret == 0) {
		ret = qti_smem_host_id(QTI_SMEM_PROC_TZ, 0U, 0U, 0U, &tz);
	}
	if (ret != 0) {
		ERROR("LLCC MPU: SMEM host ID: %d\n", ret);
		return;
	}

	for (i = 0U; ; i++) {
		uint64_t start = (uint64_t)QTI_SMEM_BASE;
		uintptr_t rdr, wrr;

		ret = qti_smem_partition(i, &off, &size, &h0, &h1);
		if (ret == -ERANGE) {
			continue;
		}
		if (ret != 0) {
			break;
		}

		start += off;
		if (((h0 != adsp) && (h1 != adsp)) ||
		    (h0 == tz) || (h1 == tz)) {
			continue;
		}

		rg = llcc_mpu_find_rg(start, start + size);
		if (rg < 0) {
			WARN("LLCC MPU: no RG for SMEM 0x%llx-0x%llx\n",
			     (unsigned long long)start,
			     (unsigned long long)(start + size));
			continue;
		}

		rdr = QTI_LLCC_MPU_BASE + XPU4_RGRDR(rg);
		wrr = QTI_LLCC_MPU_BASE + XPU4_RGWRR(rg);
		mmio_setbits_32(rdr, qads);
		mmio_setbits_32(wrr, qads);
		if (((mmio_read_32(rdr) & qads) != qads) ||
		    ((mmio_read_32(wrr) & qads) != qads)) {
			ERROR("LLCC MPU: SMEM 0x%llx: RG %d did not take the ADSP\n",
			      (unsigned long long)start, rg);
		} else {
			INFO("LLCC MPU: SMEM 0x%llx-0x%llx open to the ADSP (RG %d)\n",
			     (unsigned long long)start,
			     (unsigned long long)(start + size), rg);
		}
	}

	if (ret != -ENOENT) {
		ERROR("LLCC MPU: SMEM partition table: %d\n", ret);
	}
}

#ifdef QTI_LLCC_MPU_TZ_RG
CASSERT(QTI_LLCC_MPU_TZ_RG < XPU4_NRG_MAPPED(QTI_LLCC_MPU_SIZE),
	assert_llcc_mpu_tz_rg_mapped);

void qti_llcc_mpu_protect_tz(void)
{
	const uintptr_t b = QTI_LLCC_MPU_BASE;
	const unsigned int rg = QTI_LLCC_MPU_TZ_RG;
	const uint64_t start = QTI_LLCC_MPU_TZ_BASE;
	const uint64_t end = QTI_LLCC_MPU_TZ_LIMIT;
	const uint32_t qads = XPU4_QAD_APPS | XPU4_QAD_S;

	if ((rg >= XPU4_IDR0_NRG(mmio_read_32(b + XPU4_IDR0))) ||
	    ((mmio_read_32(b + XPU4_RGCR1(rg)) & XPU4_RGCR1_RGE) != 0U)) {
		ERROR("LLCC MPU: RG %u not available\n", rg);
		panic();
	}

	mmio_write_32(b + XPU4_RGCSAR1(rg), (uint32_t)(start >> 32));
	mmio_write_32(b + XPU4_RGCSAR0(rg), (uint32_t)start);
	mmio_write_32(b + XPU4_RGCEAR1(rg), (uint32_t)(end >> 32));
	mmio_write_32(b + XPU4_RGCEAR0(rg), (uint32_t)end);
	mmio_write_32(b + XPU4_RGRDR(rg), qads);
	mmio_write_32(b + XPU4_RGWRR(rg), qads);
	mmio_write_32(b + XPU4_RGCR1(rg), XPU4_RGCR1_RGE);
	mmio_write_32(b + XPU4_QADRGL(rg), qads);

	if ((llcc_mpu_find_rg(start, end) != (int)rg) ||
	    (mmio_read_32(b + XPU4_RGRDR(rg)) != qads) ||
	    (mmio_read_32(b + XPU4_RGWRR(rg)) != qads) ||
	    (mmio_read_32(b + XPU4_QADRGL(rg)) != qads)) {
		ERROR("LLCC MPU: RG %u did not take 0x%llx-0x%llx\n", rg,
		      (unsigned long long)start, (unsigned long long)end);
		panic();
	}

	INFO("LLCC MPU: 0x%llx-0x%llx secure only (RG %u)\n",
	     (unsigned long long)start, (unsigned long long)end, rg);
}
#endif

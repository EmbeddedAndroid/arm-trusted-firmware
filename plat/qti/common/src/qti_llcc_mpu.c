/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/smem/smem.h>
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
	unsigned int n, nrg = XPU4_IDR0_NRG(mmio_read_32(b + XPU4_IDR0));

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
 * The boot firmware protects SMEM partitions at the LLCC MPU for their hosts'
 * access domains, without the one the ADSP uses: its traffic carries the apps
 * non-secure domain, which the Qualcomm TZ adds to the ADSP's partitions at
 * cold boot (and which HLOS uses too). Without it, the ADSP's first access to
 * the partition it shares with the modem is rejected, and the SoC is reset
 * seconds later. Add the domain through the MPU's AND broadcast instance,
 * which programs the MPU of every LLCC channel. Partitions without a resource
 * group are not protected.
 */
void qti_llcc_mpu_open_adsp_smem(void)
{
	const uint32_t qads = XPU4_QAD_APPS | XPU4_QAD_NS;
	uint32_t i, off, size;
	uint16_t h0, h1;
	int ret, rg;

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
		if ((h0 != QTI_SMEM_PROC_ADSP) && (h1 != QTI_SMEM_PROC_ADSP)) {
			continue;
		}

		rg = llcc_mpu_find_rg(start, start + size);
		if (rg < 0) {
			continue;
		}

		rdr = QTI_LLCC_MPU_BASE + XPU4_RGRDR(rg);
		wrr = QTI_LLCC_MPU_BASE + XPU4_RGWRR(rg);
		mmio_setbits_32(rdr, qads);
		mmio_setbits_32(wrr, qads);
		if (((mmio_read_32(rdr) & qads) != qads) ||
		    ((mmio_read_32(wrr) & qads) != qads)) {
			ERROR("LPASS: SMEM 0x%llx: MPU RG %d did not take the ADSP\n",
			      (unsigned long long)start, rg);
		} else {
			NOTICE("LPASS: SMEM 0x%llx-0x%llx open to the ADSP (MPU RG %d)\n",
			       (unsigned long long)start,
			       (unsigned long long)(start + size), rg);
		}
	}

	if (ret != -ENOENT) {
		ERROR("LPASS: SMEM partition table: %d\n", ret);
	}
}

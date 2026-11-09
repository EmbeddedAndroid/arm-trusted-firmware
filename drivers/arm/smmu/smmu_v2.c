/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/arm/smmu_v2.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#define SMMUV2_POLL_TIMEOUT_US	U(1000)

static int smmuv2_poll(uintptr_t reg, uint32_t mask, uint32_t value)
{
	uint64_t timeout = timeout_init_us(SMMUV2_POLL_TIMEOUT_US);
	uint32_t val;

	do {
		val = mmio_read_32(reg);
		if ((val & mask) == value) {
			return 0;
		}
	} while (!timeout_elapsed(timeout));

	ERROR("Timeout polling SMMUv2 register @%p: 0x%x\n", (void *)reg, val);
	return -1;
}

/*
 * Make an SMMUv2 with the Security Extensions fault every transaction from a
 * Non-secure stream, whatever Non-secure software programmed before: each
 * Non-secure stream mapping group faults and its SMR no longer matches,
 * unmatched streams fault (NSCR0.USFCFG), the SMMU translates rather than
 * bypasses Non-secure clients (NSCR0.CLIENTPD) and stream match conflicts
 * fault (NSCR0.SMCFCFG). Secure streams are left alone. Non-secure software
 * that owns the SMMU afterwards reprograms all of this when it resets the
 * SMMU.
 */
int smmuv2_ns_set_abort_all(uintptr_t smmu_base)
{
	uint32_t idr0 = mmio_read_32(smmu_base + SMMUV2_SIDR0);
	uint32_t scr1 = mmio_read_32(smmu_base + SMMUV2_SCR1);
	uint32_t num_smrg = idr0 & SMMUV2_SIDR0_NUMSMRG_MASK;
	uint32_t ns_smrg = (scr1 >> SMMUV2_SCR1_NSNUMSMRGO_SHIFT) &
			   SMMUV2_SCR1_NSNUMSMRGO_MASK;
	uint32_t cr0;

	if (ns_smrg > num_smrg) {
		ns_smrg = num_smrg;
	}

	for (uint32_t i = 0U; i < ns_smrg; i++) {
		mmio_write_32(smmu_base + SMMUV2_S2CR(i), SMMUV2_S2CR_TYPE_FAULT);
		if ((idr0 & SMMUV2_SIDR0_SMS) != 0U) {
			mmio_write_32(smmu_base + SMMUV2_SMR(i), 0U);
		}
	}

	cr0 = mmio_read_32(smmu_base + SMMUV2_NSCR0);
	cr0 &= ~SMMUV2_CR0_CLIENTPD;
	cr0 |= SMMUV2_CR0_USFCFG | SMMUV2_CR0_SMCFCFG;
	mmio_write_32(smmu_base + SMMUV2_NSCR0, cr0);

	/* Drop translations cached for Non-secure streams before the change. */
	mmio_write_32(smmu_base + SMMUV2_TLBIALLNSNH, 0U);
	mmio_write_32(smmu_base + SMMUV2_STLBGSYNC, 0U);
	if (smmuv2_poll(smmu_base + SMMUV2_STLBGSTATUS,
			SMMUV2_STLBGSTATUS_GSACTIVE, 0U) != 0) {
		return -1;
	}

	cr0 = mmio_read_32(smmu_base + SMMUV2_NSCR0);
	if ((cr0 & (SMMUV2_CR0_CLIENTPD | SMMUV2_CR0_USFCFG)) !=
	    SMMUV2_CR0_USFCFG) {
		ERROR("SMMUv2 @0x%lx: NSCR0 0x%x does not abort\n",
		      smmu_base, cr0);
		return -1;
	}

	INFO("SMMUv2 @0x%lx: %u of %u stream mapping groups Non-secure, NSCR0 0x%x\n",
		smmu_base, ns_smrg, num_smrg, cr0);

	return 0;
}

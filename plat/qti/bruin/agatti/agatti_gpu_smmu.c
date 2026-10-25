/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <qti_plat.h>

/*
 * The GPU registers that give the CP an aperture to an Adreno SMMU context
 * bank, for the pagetable switches it does from the ringbuffer, and the
 * context banks they may point at.
 */
#define GPU_SMMU_CB_BASE		0x059a8000
#define GPU_SMMU_CB_SIZE		0x1000
#define GPU_SMMU_NUM_CB			8

static const uintptr_t gpu_smmu_aperture_regs[] = {
	0x05960000,
	0x05960004,
	0x0596000c,
	0x05960010,
};

bool plat_qti_gpu_smmu_aperture_supported(void)
{
	return true;
}

/*
 * Each word holds two requests, from the low byte up: context bank,
 * aperture, context bank, aperture. Requests out of range are skipped,
 * which is how the caller leaves a slot unused (0xff).
 */
int plat_qti_gpu_smmu_aperture(const uint32_t pairs[4])
{
	unsigned int i, shift, cb, reg;
	uintptr_t cb_addr;

	for (i = 0; i < 4; i++) {
		for (shift = 0; shift < 32; shift += 16) {
			cb = (pairs[i] >> shift) & 0xff;
			reg = (pairs[i] >> (shift + 8)) & 0xff;
			if ((cb >= GPU_SMMU_NUM_CB) ||
			    (reg >= ARRAY_SIZE(gpu_smmu_aperture_regs))) {
				continue;
			}
			/* The aperture takes the offset in the GPU's 1 MiB window. */
			cb_addr = GPU_SMMU_CB_BASE + cb * GPU_SMMU_CB_SIZE;
			mmio_write_32(gpu_smmu_aperture_regs[reg],
				      cb_addr & 0xfffff);
		}
	}

	return 0;
}

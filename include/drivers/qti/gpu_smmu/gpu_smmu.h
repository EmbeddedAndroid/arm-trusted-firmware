/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_GPU_SMMU_H
#define QTI_GPU_SMMU_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * GPU SMMU secure setup and aperture: the Adreno CP switches the page tables
 * of the GPU SMMU context banks through the aperture, for per-process page
 * tables. Only the secure world sets them up, and they are in the GPU CX
 * domain, which is collapsed when BL31 starts.
 */

struct qti_gpu_smmu_reg {
	uintptr_t addr;
	uint32_t value;
	/* Read back after the write and fail the setup on a mismatch. */
	bool verify;
};

/* What a chipset provides to program its GPU SMMU (gpu_smmu_cfg.c). */
struct qti_gpu_smmu_cfg {
	/* GCC branch of the GPU config interface. */
	uintptr_t cfg_ahb_cbcr;
	/* GPU_CC TZ votes for the GPU SMMU power domain (GPU CX) and clock. */
	uintptr_t tz_vote_gds;
	uintptr_t tz_vote_clk;
	/* GPU_CC_CX_GDSCR, for register retention across CX collapse. */
	uintptr_t cx_gdscr;
	/* Further GPU CX branches the registers below need, if any. */
	const uintptr_t *cx_cbcrs;
	size_t num_cx_cbcrs;
	const struct qti_gpu_smmu_reg *regs;
	size_t num_regs;
};

extern const struct qti_gpu_smmu_cfg qti_gpu_smmu_cfg;

#ifdef QTI_GPU_SMMU_ENABLED
void qti_gpu_smmu_init(void);
#else
static inline void qti_gpu_smmu_init(void)
{
}
#endif

#endif /* QTI_GPU_SMMU_H */

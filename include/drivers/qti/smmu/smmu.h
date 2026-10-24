/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_H
#define SMMU_H

#include <stddef.h>
#include <stdint.h>

#ifdef QTI_SMMU_ENABLED
struct smmu_cfg_s {
	uintptr_t addr;
	uint32_t value;
};

void qti_smmu_init(void);
void qti_smmu_clocked_init(void);
extern struct smmu_cfg_s qti_smmu_cfg[];
extern const size_t qti_smmu_cfg_count;
#else
static inline void qti_smmu_init(void) {}
static inline void qti_smmu_clocked_init(void) {}
#endif

/*
 * GPU SMMU registers in the GPU CX domain, which is collapsed with its clocks
 * off when BL31 starts: the secure setup, and the aperture slots through which
 * the Adreno CP reaches a context bank to switch it to per-process page
 * tables.
 */
struct qti_smmu_gpu_cx {
	/* GCC branch of the GPU config interface. */
	uintptr_t cfg_ahb_cbcr;
	/* GPU_CC TZ votes for the GPU SMMU power domain (GPU CX) and clock. */
	uintptr_t tz_vote_gds;
	uintptr_t tz_vote_clk;
	/* GPU_CC TZ control of GPU CX, for register retention. */
	uintptr_t tz_gdsc_ctrl;
	/* GPU CX GDSC, checked before a runtime aperture write. */
	uintptr_t cx_gdscr;
	/* Further GPU CX branches the registers below need, if any. */
	const uintptr_t *cx_cbcrs;
	size_t num_cx_cbcrs;
	const struct smmu_cfg_s *regs;
	size_t num_regs;
	/* Aperture slots, numbered as the OS requests them. */
	const uintptr_t *aperture;
	size_t num_aperture;
	/* Context bank 0 as an offset in the GPU register space. */
	uint32_t cb0_offset;
	uint32_t num_cbs;
};

extern const struct qti_smmu_gpu_cx qti_smmu_gpu_cx;

#ifdef QTI_SMMU_GPU_CX
void qti_smmu_gpu_init(void);
#else
static inline void qti_smmu_gpu_init(void) {}
#endif

#endif

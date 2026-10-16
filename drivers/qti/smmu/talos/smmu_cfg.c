/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/smmu/smmu.h>
#include <lib/utils_def.h>

/*
 * GPU SMMU aperture slots 0 and 1 point at context banks 0 and 1 of the GPU
 * SMMU (offsets from the GPU register base), through which the Adreno CP
 * switches them to per-process page tables. Only the secure world can write
 * them; they are in GPU CX, retained over CX collapse.
 */
#define GPU_SMMU_APERTURE_S1CB0		0x05060000U
#define GPU_SMMU_APERTURE_S1CB1		0x05060004U
#define GPU_SMMU_APERTURE_CTL		0x05060008U
#define GPU_SMMU_CB0_OFFSET		0x000a8000U
#define GPU_SMMU_CB1_OFFSET		0x000a9000U

struct smmu_cfg_s qti_smmu_cfg[] = {};

const size_t qti_smmu_cfg_count = ARRAY_SIZE(qti_smmu_cfg);

const struct smmu_cfg_s qti_smmu_gpu_cfg[] = {
	{GPU_SMMU_APERTURE_S1CB0, GPU_SMMU_CB0_OFFSET},
	{GPU_SMMU_APERTURE_S1CB1, GPU_SMMU_CB1_OFFSET},
	{GPU_SMMU_APERTURE_CTL, 0U},
};

const size_t qti_smmu_gpu_cfg_count = ARRAY_SIZE(qti_smmu_gpu_cfg);

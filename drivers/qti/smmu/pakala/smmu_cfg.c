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
 * GPU SMMU aperture slot 0 points at GPU SMMU context bank 0, as an offset in
 * the GPU register space (gpu@3d00000): the context banks start in the second
 * half of the 256 KB GPU SMMU at 0x03da0000. The aperture control keeps its
 * reset value.
 */
#define GPU_SMMU_APERTURE_GC		0x03d60000U
#define GPU_SMMU_CB0_OFFSET		0x000c0000U

/* The apps SMMU needs no setup from BL31. */
struct smmu_cfg_s qti_smmu_cfg[] = {};

const size_t qti_smmu_cfg_count = ARRAY_SIZE(qti_smmu_cfg);

const struct smmu_cfg_s qti_smmu_gpu_cfg[] = {
	{GPU_SMMU_APERTURE_GC, GPU_SMMU_CB0_OFFSET},
};

const size_t qti_smmu_gpu_cfg_count = ARRAY_SIZE(qti_smmu_gpu_cfg);

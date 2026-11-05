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
 * The CP fetches its trusted (zap shader) microcode through stream ID 0x3
 * of the GPU SMMU, which no OS stream mapping covers: bypass it in the last
 * of the 25 stream match groups.
 */
#define GPU_SMMU_SMR24			0x03da0860U
#define GPU_SMMU_S2CR24			0x03da0c60U
#define SMR_VALID			BIT_32(31)
#define GPU_CP_TRUSTED_UCODE_SID	0x3U
#define S2CR_TYPE_BYPASS		(1U << 16)
#define S2CR_CBNDX_NONE			0xffU

/*
 * GPU SMMU aperture slot 0 (GC) points at context bank 0 (0x03dc0000), as an
 * offset in the GPU register space: the CP switches a process's page tables
 * through it. It is in GPU CX, retained over CX collapse.
 */
#define GPU_SMMU_APERTURE_GC		0x03d60000U
#define GPU_SMMU_CB0_OFFSET		0x000c0000U

struct smmu_cfg_s qti_smmu_cfg[] = {};

const size_t qti_smmu_cfg_count = ARRAY_SIZE(qti_smmu_cfg);

const struct smmu_cfg_s qti_smmu_gpu_cfg[] = {
	{GPU_SMMU_S2CR24, S2CR_TYPE_BYPASS | S2CR_CBNDX_NONE},
	{GPU_SMMU_SMR24, SMR_VALID | GPU_CP_TRUSTED_UCODE_SID},
	{GPU_SMMU_APERTURE_GC, GPU_SMMU_CB0_OFFSET},
};

const size_t qti_smmu_gpu_cfg_count = ARRAY_SIZE(qti_smmu_gpu_cfg);

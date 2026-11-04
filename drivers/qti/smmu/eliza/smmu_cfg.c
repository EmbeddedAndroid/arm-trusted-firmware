/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/smmu/smmu.h>
#include <lib/utils_def.h>

/* SACR.CACHE_LOCK clear, so that the OS's context bank ACTLR writes apply. */
#define APPS_SMMU_SACR			0x15000010U
#define GPU_SMMU_SACR			0x03da0010U
#define SMMU_SACR			0x00000004U

/*
 * GPU SMMU aperture slot 0 (GC) points at context bank 0, as an offset in the
 * GPU register space, and the aperture control enables the slots.
 */
#define GPU_SMMU_APERTURE_GC		0x03d60000U
#define GPU_SMMU_APERTURE_CTRL		0x03d60008U
#define GPU_SMMU_CB0_OFFSET		0x000c0000U
#define GPU_SMMU_APERTURE_CTRL_EN	0x00000002U

struct smmu_cfg_s qti_smmu_cfg[] = {
	{APPS_SMMU_SACR, SMMU_SACR},
};

const size_t qti_smmu_cfg_count = ARRAY_SIZE(qti_smmu_cfg);

const struct smmu_cfg_s qti_smmu_gpu_cfg[] = {
	{GPU_SMMU_SACR, SMMU_SACR},
	{GPU_SMMU_APERTURE_GC, GPU_SMMU_CB0_OFFSET},
	{GPU_SMMU_APERTURE_CTRL, GPU_SMMU_APERTURE_CTRL_EN},
};

const size_t qti_smmu_gpu_cfg_count = ARRAY_SIZE(qti_smmu_gpu_cfg);

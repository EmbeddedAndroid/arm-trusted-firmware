/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_GPU_SMMU_H
#define QTI_GPU_SMMU_H

/*
 * GPU SMMU aperture: the Adreno CP switches the page tables of the GPU SMMU
 * context banks through it, for per-process page tables. Only the secure
 * world sets it up.
 */
#ifdef QTI_GPU_SMMU_ENABLED
void qti_gpu_smmu_init(void);
#else
static inline void qti_gpu_smmu_init(void)
{
}
#endif

#endif /* QTI_GPU_SMMU_H */

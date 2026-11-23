/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_LLCC_MPU_H
#define QTI_LLCC_MPU_H

#include <platform_def.h>

/*
 * Add the apps non-secure access domain to the LLCC MPU resource group of each
 * SMEM partition shared with the ADSP. The platform maps the MPU's AND
 * broadcast instance at QTI_LLCC_MPU_BASE, QTI_LLCC_MPU_SIZE bytes (only the
 * resource groups inside that mapping are searched), and the SMEM driver must
 * be initialized.
 */
#ifdef QTI_LLCC_MPU_BASE
void qti_llcc_mpu_open_adsp_smem(void);
#else
static inline void qti_llcc_mpu_open_adsp_smem(void)
{
}
#endif

/*
 * Restrict [QTI_LLCC_MPU_TZ_BASE, QTI_LLCC_MPU_TZ_LIMIT) to the secure apps
 * domain with LLCC MPU resource group QTI_LLCC_MPU_TZ_RG, which must be
 * free, and set its lock vector to that domain. Panics if the MPU does not
 * take the setting.
 */
#if defined(QTI_LLCC_MPU_BASE) && defined(QTI_LLCC_MPU_TZ_RG)
void qti_llcc_mpu_protect_tz(void);
#else
static inline void qti_llcc_mpu_protect_tz(void)
{
}
#endif

#endif /* QTI_LLCC_MPU_H */

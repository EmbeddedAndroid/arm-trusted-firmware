/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SECURE_IO_CFG_H
#define QTI_SECURE_IO_CFG_H

#include <stdint.h>

/*
 * Registers the non-secure world may access through the secure IO calls:
 * the download mode cookie and the two EUD enables.
 */
#define TCSR_TCSR_BOOT_MISC_DETECT		0x003d3000
#define TCSR_APSS_SPARE_REG1			0x003e5018
#define AHB2PHY_USBEUD_EUD_EN2			0x01612000

static const uintptr_t qti_secure_io_allowed_regs[] = {
	TCSR_TCSR_BOOT_MISC_DETECT,
	TCSR_APSS_SPARE_REG1,
	AHB2PHY_USBEUD_EUD_EN2,
};

/*
 * The GPU registers that give the CP an aperture to an Adreno SMMU context
 * bank, for the pagetable switches it does from the ringbuffer, and the
 * context banks they may point at.
 */
#define QTI_GPU_SMMU_CB_BASE			0x059a8000
#define QTI_GPU_SMMU_CB_SIZE			0x1000
#define QTI_GPU_SMMU_NUM_CB			8

static const uintptr_t qti_gpu_smmu_aperture_regs[] = {
	0x05960000,
	0x05960004,
	0x0596000c,
	0x05960010,
};

#endif /* QTI_SECURE_IO_CFG_H */

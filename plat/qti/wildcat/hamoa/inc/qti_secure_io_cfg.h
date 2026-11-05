/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SECURE_IO_CFG_H
#define QTI_SECURE_IO_CFG_H

#include <stdint.h>

/*
 * Secure registers the non-secure world may access through the SiP secure
 * I/O call: the APPS SMMU (0x15000000) registers the Linux SMMU driver uses,
 * the APPS PDC GPIO interrupt mode the Linux PDC driver sets and the boot
 * misc detect register (TCSR) that holds the download mode.
 */
#define APPS_SMMU_TBU_PWR_STATUS		0x15002204
#define APPS_SMMU_CUSTOM_CFG			0x15002300
#define APPS_SMMU_STATS_SYNC_INV_TBU_ACK	0x150025DC
#define APPS_SMMU_SAFE_SEC_CFG			0x15002648
#define APPS_SMMU_MMU2QSS_AND_SAFE_WAIT_CNTR	0x15002670
#define RPMH_PDC_APPS_AUX_GPIO_INT_CTL_ENABLE	0x0B2045E8
#define TCSR_TCSR_BOOT_MISC_DETECT		0x01FD9000

static const uintptr_t qti_secure_io_allowed_regs[] = {
	APPS_SMMU_TBU_PWR_STATUS,
	APPS_SMMU_CUSTOM_CFG,
	APPS_SMMU_STATS_SYNC_INV_TBU_ACK,
	APPS_SMMU_SAFE_SEC_CFG,
	APPS_SMMU_MMU2QSS_AND_SAFE_WAIT_CNTR,
	RPMH_PDC_APPS_AUX_GPIO_INT_CTL_ENABLE,
	TCSR_TCSR_BOOT_MISC_DETECT,
};

#endif /* QTI_SECURE_IO_CFG_H */

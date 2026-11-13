/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */
#ifndef DRTM_REMEDIATION_H
#define DRTM_REMEDIATION_H

#include <cdefs.h>
#include <stdint.h>

#include <lib/utils_def.h>

uint64_t drtm_set_error(uint64_t x1, void *ctx);
uint64_t drtm_get_error(void *ctx);

/* Error codes the DCE records before it stops a launch */
#define DRTM_REMEDIATION_DMA_PROT	ULL(1)
#define DRTM_REMEDIATION_TPM		ULL(2)
#define DRTM_REMEDIATION_LAUNCH		ULL(3)

void __dead2 drtm_enter_remediation(uint64_t error_code,
				    const char *error_str);

#endif /* DRTM_REMEDIATION_H */

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

	.section .rodata.qti_spmc_manifest, "a"
	.balign 8
	.global qti_spmc_manifest
	.global qti_spmc_manifest_end
qti_spmc_manifest:
	.incbin QTI_SPMC_MANIFEST_DTB
qti_spmc_manifest_end:

	/* BL31 copies it to TOS_FW_CONFIG_BASE, which ends where BL31 starts */
	.if (qti_spmc_manifest_end - qti_spmc_manifest) > TOS_FW_CONFIG_SIZE
	.error "SPMC manifest larger than TOS_FW_CONFIG_SIZE"
	.endif

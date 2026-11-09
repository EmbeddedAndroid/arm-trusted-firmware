/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.section .rodata.qti_spmc_manifest, "a"
	.balign 8
	.global qti_spmc_manifest
	.global qti_spmc_manifest_end
qti_spmc_manifest:
	.incbin QTI_SPMC_MANIFEST_DTB
qti_spmc_manifest_end:

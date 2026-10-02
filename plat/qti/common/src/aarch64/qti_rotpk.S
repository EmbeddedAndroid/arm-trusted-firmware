/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.section .rodata.qti_rotpk_hash, "a"
	.global qti_rotpk_hash
	.global qti_rotpk_hash_end
qti_rotpk_hash:
	/* DER header of a SHA-256 DigestInfo */
	.byte 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48
	.byte 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
	.incbin ROTPK_HASH
qti_rotpk_hash_end:

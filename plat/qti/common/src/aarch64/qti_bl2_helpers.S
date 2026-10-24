/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.global	platform_mem_init

	/* -----------------------------------------------------------------
	 * void platform_mem_init(void)
	 *
	 * The image the boot loader loads does not describe a separate BL2
	 * NOLOAD region, so nothing has zeroed it. Discard any cache lines
	 * left for it and zero it before the C runtime uses it. Runs with
	 * the MMU off and without a stack.
	 * -----------------------------------------------------------------
	 */
func platform_mem_init
#if SEPARATE_BL2_NOLOAD_REGION
	mov	x9, x30
	adrp	x10, __BL2_NOLOAD_START__
	add	x10, x10, :lo12:__BL2_NOLOAD_START__
	adrp	x11, __BL2_NOLOAD_END__
	add	x11, x11, :lo12:__BL2_NOLOAD_END__
	sub	x11, x11, x10
	mov	x0, x10
	mov	x1, x11
	bl	inv_dcache_range
	mov	x0, x10
	mov	x1, x11
	bl	zeromem
	ret	x9
#else
	ret
#endif
endfunc platform_mem_init

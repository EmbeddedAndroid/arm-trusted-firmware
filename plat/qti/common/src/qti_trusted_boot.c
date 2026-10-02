/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <plat/common/platform.h>

extern char qti_rotpk_hash[], qti_rotpk_hash_end[];

int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	*key_ptr = qti_rotpk_hash;
	*key_len = qti_rotpk_hash_end - qti_rotpk_hash;
	*flags = ROTPK_IS_HASH;

	return 0;
}

/*
 * There is no rollback protection: the counters read as 0 and cannot be
 * raised, so the certificates must carry NV counter 0.
 */
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	*nv_ctr = 0;

	return 0;
}

int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	return 1;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}

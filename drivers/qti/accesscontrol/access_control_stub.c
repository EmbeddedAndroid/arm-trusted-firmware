/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/accesscontrol/accesscontrol.h>

/*
 * Access-control stub for platforms that do not provide a memory-assignment
 * backend. The stub is intentionally free of any QTISECLIB dependency so it
 * can be reused by non-QTISECLIB architectures. Memory-assign requests are
 * rejected, or accepted as a no-op with QTI_MEM_ASSIGN_NOOP.
 */
uint64_t
qti_accesscontrol_mem_assign(const qti_accesscontrol_mem_t *mem,
			     uint32_t mem_len,
			     const uint32_t *src, uint32_t src_len,
			     const qti_accesscontrol_perm_t *perm,
			     uint32_t perm_len)
{
	(void)mem;
	(void)mem_len;
	(void)src;
	(void)src_len;
	(void)perm;
	(void)perm_len;

	return (QTI_MEM_ASSIGN_NOOP != 0) ? 0U : 1U;
}

void qti_accesscontrol_init(void)
{
}

bool qti_accesscontrol_mem_assign_available(void)
{
	return QTI_MEM_ASSIGN_NOOP != 0;
}

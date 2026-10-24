/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ACCESSCONTROL_H
#define ACCESSCONTROL_H

#include <stdbool.h>
#include <stdint.h>

/*
 * A platform without an access control backend links access_control_stub.c,
 * whose memory assignment does nothing. QTI_MEM_ASSIGN_NOOP=1 has it accept
 * MEM_ASSIGN calls anyway, which tells the caller memory changed hands while
 * nothing protects it; otherwise the SiP service rejects them.
 */
#ifndef QTI_MEM_ASSIGN_NOOP
#define QTI_MEM_ASSIGN_NOOP	0
#endif

typedef struct qti_accesscontrol_mem {
	uint64_t mem_addr;
	uint64_t mem_size;
} qti_accesscontrol_mem_t;

typedef struct qti_accesscontrol_perm {
	uint32_t dst_vm;
	uint32_t dst_vm_perm;
	uint64_t ctx;
	uint32_t ctx_size;
} qti_accesscontrol_perm_t;

uint64_t qti_accesscontrol_mem_assign(const qti_accesscontrol_mem_t *mem_info,
				      uint32_t mem_len,
				      const uint32_t *src, uint32_t src_len,
				      const qti_accesscontrol_perm_t *dst,
				      uint32_t dst_len);
void qti_accesscontrol_init(void);
/* Whether qti_accesscontrol_mem_assign() is offered to callers. */
bool qti_accesscontrol_mem_assign_available(void);

#endif /* ACCESSCONTROL_H */

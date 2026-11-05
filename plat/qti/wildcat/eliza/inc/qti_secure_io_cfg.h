/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QTI_SECURE_IO_CFG_H
#define QTI_SECURE_IO_CFG_H

#include <stdint.h>

/*
 * List of peripheral/IO memory areas that are protected from
 * non-secure world but not required to be secure.
 */

#define TCSR_BOOT_MISC_DETECT			0x01fd9000

static const uintptr_t qti_secure_io_allowed_regs[] = {
	TCSR_BOOT_MISC_DETECT,
};

#endif /* QTI_SECURE_IO_CFG_H */

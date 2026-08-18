/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <errno.h>
#include <stdint.h>

#include <drivers/qti/ipcc/ipcc.h>

void qti_ipcc_init(void)
{
}

int32_t qti_ipcc_signal(enum ipcc_protocol protocol __unused,
			enum ipcc_client cid __unused,
			uint16_t sig_lo __unused,
			uint16_t sig_hi __unused)
{
	return -ENODEV;
}

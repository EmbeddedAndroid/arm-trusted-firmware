/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <drivers/qti/sec_core/sec_core.h>
#include <lib/mmio.h>

#include <platform_def.h>

/*
 * The Oryon clusters take the reset address of their cores from NCC_RVBAR
 * in each cluster's NCC_ARCH block (PLAT_NCC_ARCH_BASE, one block every
 * PLAT_NCC_CLUSTER_STRIDE), not from the Kryo RVBARADDR registers.
 */
#define NCC_RVBAR			0x328U

void qti_sec_core_remap(uintptr_t entrypoint)
{
	unsigned int cluster;

	for (cluster = 0U; cluster < PLAT_CLUSTER_COUNT; cluster++) {
		mmio_write_64(PLAT_NCC_ARCH_BASE +
			      (cluster * PLAT_NCC_CLUSTER_STRIDE) + NCC_RVBAR,
			      entrypoint);
	}
	dsbsy();
}

void qti_sec_core_init(void)
{
}

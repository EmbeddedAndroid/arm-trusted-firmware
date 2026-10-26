/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HOYA_DSU_PM_H
#define HOYA_DSU_PM_H

#include <stdbool.h>
#include <stdint.h>

/* Cluster IPM block (APSS_CL_IPM), home of the rail sequencers. */
#define HOYA_CL_IPM_BASE		(QTI_APSS_HM_BASE + 0x8a0000U)

/*
 * A gold CPU rail, off at cold boot: the first CPU_ON of a core in
 * [first_core, last_core] enables the rail through its SAW4 and then has the
 * rail sequencer repair the memories on it (repair_bit in the EN and VAL
 * registers, repair_done in the status register).
 */
struct hoya_gold_rail {
	unsigned int first_core;
	unsigned int last_core;
	uintptr_t saw4;
	uintptr_t repair_en;
	uintptr_t repair_val;
	uintptr_t repair_sts;
	uint32_t repair_bit;
	uint32_t repair_done;
	bool on;
};

#endif /* HOYA_DSU_PM_H */

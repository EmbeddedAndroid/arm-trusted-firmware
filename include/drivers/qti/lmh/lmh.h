/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LMH_H
#define LMH_H

#include <stdint.h>

struct lmh_llm_write {
	uintptr_t addr;
	uint32_t val;
};

/* An LLM block and the RPMh resource of the CPU rail it limits. */
struct lmh_llm_rail {
	uintptr_t base;
	const char *rail;
};

extern const struct lmh_llm_write lmh_llm_cfg[];
extern const unsigned int lmh_llm_cfg_count;
extern const struct lmh_llm_rail lmh_llm_rails[];
extern const unsigned int lmh_llm_rail_count;

void lmh_plat_clock_init(void);

/*
 * qti_lmh_init - program the CPU limits management (LMh) hardware so the
 * clusters are throttled on temperature. Must run before CPUCP takes the
 * clock domains.
 */
#if QTI_LMH_ENABLED
void qti_lmh_init(void);
#else
static inline void qti_lmh_init(void)
{
}
#endif

#endif /* LMH_H */

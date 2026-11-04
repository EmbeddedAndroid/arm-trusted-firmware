/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/sec_core/sec_core.h>
#include <lib/utils_def.h>

#include <sec_core_defs.h>

#define APSS_ALIAS_APC_SECURE(n)					\
	{ APSS_ALIAS_0_APC_SECURE_ADDR +				\
		  ((n) * APSS_ALIASn_APC_SECURE_OFFSET_TO_NEXT),	\
	  APSS_ALIAS_0_APC_SECURE_RMSK }

/*
 * Keep the core, cluster and rail power controls and the secure watchdog
 * (TMR2) secure-only, and give the other watchdog (TMR1) to the normal
 * world, which XBL leaves running and secure.
 */
const struct sec_core_cfg_s qti_sec_core_cfg[] = {
	APSS_ALIAS_APC_SECURE(0),
	APSS_ALIAS_APC_SECURE(1),
	APSS_ALIAS_APC_SECURE(2),
	APSS_ALIAS_APC_SECURE(3),
	APSS_ALIAS_APC_SECURE(4),
	APSS_ALIAS_APC_SECURE(5),
	APSS_ALIAS_APC_SECURE(6),
	APSS_ALIAS_APC_SECURE(7),
	{APSS_CL_SECURE_ADDR, APSS_CL_SECURE_RMSK},
	{SILVER_SAW4_SECURE_ADDR, SAW4_SECURE_RMSK},
	{GOLD_SAW4_SECURE_ADDR, SAW4_SECURE_RMSK},
	{WDT_TMR2_WDOG_SECURE_ADDR, WDOG_SECURE_SEC},
	{WDT_TMR1_WDOG_SECURE_ADDR, WDOG_SECURE_NSEC},
};

const size_t qti_sec_core_cfg_count = ARRAY_SIZE(qti_sec_core_cfg);

const uintptr_t qti_sec_core_rvbaraddr_lo = APSS_SHARED_KRYO_RVBARADDR_LO_ADDR;
const uintptr_t qti_sec_core_rvbaraddr_hi = APSS_SHARED_KRYO_RVBARADDR_HI_ADDR;

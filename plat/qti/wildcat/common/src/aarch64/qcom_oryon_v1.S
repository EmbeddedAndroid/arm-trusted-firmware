/*
* Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cpu_macros.S>

#include <plat_cpu_helpers.S>
#include <plat_macros.S>
#include <qcom_oryon_v1.h>


/*
 * Hardware handled coherency
 * #if HW_ASSISTED_COHERENCY == 0
 * #error "oryon_v1 must be compiled with HW_ASSISTED_COHERENCY enabled"
 * #endif
 */

cpu_reset_prologue oryon_v1

cpu_reset_func_start oryon_v1
/* Enable Exclusive load/store accesses to Non Cacheable Memory
 * This is required to prevent a Data Abort and allow NC memory
 * read/write access
 * Set Bit[1] in ACTLR_EL3
 */
en_ld_str_nc_mem:
	ENABLE_EX_LD_STR_TO_NC_MEM

	dsb	sy
	isb
	ret
cpu_reset_func_end oryon_v1

func oryon_v1_core_pwr_dwn
	dsb	sy
	isb
	ret
endfunc oryon_v1_core_pwr_dwn

declare_cpu_ops oryon_v1, ORYON_V1_MIDR, \
	oryon_v1_reset_func, \
	oryon_v1_core_pwr_dwn

declare_cpu_ops oryon_v1, ORYON_V1_POWER_MIDR, \
	oryon_v1_reset_func, \
	oryon_v1_core_pwr_dwn
declare_cpu_ops oryon_v1, ORYON_V1_PERF_MIDR, \
	oryon_v1_reset_func, \
	oryon_v1_core_pwr_dwn

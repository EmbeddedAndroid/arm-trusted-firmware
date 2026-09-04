/*
* Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include <arch.h>
#include <dsu_def.h>

#include "qti_cpu_id.h"

//CPTR and fields
#define CPTR_EL2					S3_4_c1_c1_2
#define CPTR_EL3					S3_6_c1_c1_2
#define CPTR_TAM_BITS					1


// SCR_EL3 and fields
#define SCR_EL3						S3_6_C1_C1_0
#define SCR_EL3_FGTEN_SHFT				27
#define SCR_EL3_FGTEN_BITS				1
#define SCR_EL3_ENTP2_SHFT				41
#define SCR_EL3_ENTP2_BITS				1

	.macro SPR_OP spr, val, lsb, width
	MRS X0, \spr
	MOV X1, \val
	BFI X0, X1, \lsb, \width
	MSR \spr, X0
	.endm

// Helper macros for cpu core functions

	.macro ENABLE_EX_LD_STR_TO_NC_MEM
	SPR_OP ACTLR_EL3, #3, #0, #2
	.endm

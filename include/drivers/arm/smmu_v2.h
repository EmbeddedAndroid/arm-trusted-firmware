/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_V2_H
#define SMMU_V2_H

#include <stdint.h>

#include <lib/utils_def.h>

/*
 * SMMUv2 (Arm IHI 0062) global register space 0, as seen by a Secure access
 * on an implementation with the Security Extensions.
 */
#define SMMUV2_SCR1		U(0x004)
#define SMMUV2_SIDR0		U(0x020)
#define SMMUV2_TLBIALLNSNH	U(0x068)
#define SMMUV2_STLBGSYNC	U(0x070)
#define SMMUV2_STLBGSTATUS	U(0x074)
#define SMMUV2_NSCR0		U(0x400)
#define SMMUV2_SMR(n)		(U(0x800) + ((n) << 2))
#define SMMUV2_S2CR(n)		(U(0xc00) + ((n) << 2))

/* sCR1 */
#define SMMUV2_SCR1_NSNUMSMRGO_SHIFT	U(8)
#define SMMUV2_SCR1_NSNUMSMRGO_MASK	U(0xff)

/* sIDR0 */
#define SMMUV2_SIDR0_NUMSMRG_MASK	U(0xff)
#define SMMUV2_SIDR0_SMS		BIT_32(27)

/* NSCR0 */
#define SMMUV2_CR0_CLIENTPD		BIT_32(0)
#define SMMUV2_CR0_USFCFG		BIT_32(10)
#define SMMUV2_CR0_SMCFCFG		BIT_32(21)

/* sTLBGSTATUS */
#define SMMUV2_STLBGSTATUS_GSACTIVE	BIT_32(0)

/* S2CRn */
#define SMMUV2_S2CR_TYPE_FAULT		(U(2) << 16)

int smmuv2_ns_set_abort_all(uintptr_t smmu_base);

#endif /* SMMU_V2_H */

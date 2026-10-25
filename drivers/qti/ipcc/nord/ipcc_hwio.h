/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPCC_HWIO_H
#define IPCC_HWIO_H

#include <lib/utils_def.h>

/* Client pages at 0x09000000; the platform must map this window. */
#define IPCC_BASE			UL(0x09000000)
#define IPCC_SIZE			UL(0x00400000)

/* 5 protocol blocks of 0xc0000, each holding 192 client pages 0x1000 apart. */
#define IPCC_PROTO_STRIDE		U(0xc0000)
#define IPCC_CLIENT_STRIDE		U(0x1000)

/* Signals per client on MPROC. Varies by target and by protocol. */
#define IPCC_MPROC_NUM_SIGS		U(8)

/* APSS_INTU_APSS_INTU_IPC, a window the platform maps separately. */
#define IPCC_DIRECT_BASE		UL(0x17824000)

/* TOP_MODE and TRACE block offsets from IPCC_BASE. */
#define IPCC_TOP_MODE_BLOCK_OFF		U(0x3c0000)
#define IPCC_TRACE_BLOCK_OFF		U(0x3c1000)

/* APSS_INTU_TZ_TME_MBOX_INTERRUPT, write-only. */
#define IPCC_DIRECT_TME_TRIG_REG	(IPCC_DIRECT_BASE + UL(0x10))

/* Field TME_INT [0:0], so one signal is the hardware limit. */
#define IPCC_DIRECT_TME_SIG0_MASK	BIT_32(0)

#endif /* IPCC_HWIO_H */

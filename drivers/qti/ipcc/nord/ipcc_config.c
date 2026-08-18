/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/ipcc/ipcc.h>
#include <lib/utils_def.h>

#include <ipcc_hwio.h>

#include "ipcc_internal.h"

/* TME signals: mask is the whole TME_INT field of the mailbox register. */
static const struct ipcc_direct_signal tme_signals[] = {
	[0] = { .out_mask = IPCC_DIRECT_TME_SIG0_MASK },
};

/* Indexed by client ID; holes keep num_signals == 0 and are rejected. */
static const struct ipcc_direct_client direct_clients[] = {
	[IPCC_CLIENT_TMESS] = {
		.num_signals = ARRAY_SIZE(tme_signals),
		.signals = tme_signals,
		.trigger_reg = IPCC_DIRECT_TME_TRIG_REG,
		.is_supported = true,
	},
};

static const struct ipcc_direct_cfg direct_cfg = {
	.num_clients = ARRAY_SIZE(direct_clients),
	.clients = direct_clients,
};

const struct ipcc_direct_cfg *const ipcc_chipset_direct_cfg = &direct_cfg;

/* MPROC clients in physical page order; reordering retargets every signal. */
static const struct ipcc_client_cfg mproc_clients[] = {
	{ .client = IPCC_CLIENT_AOP },				/* physical index 0 */
	{ .client = IPCC_CLIENT_APSS_S },			/* physical index 1, us */
	{ .client = IPCC_CLIENT_ADSP0 },
	{ .client = IPCC_CLIENT_NSP0 },
	{ .client = IPCC_CLIENT_NSP1 },
	{ .client = IPCC_CLIENT_NSP2 },
	{ .client = IPCC_CLIENT_NSP3 },
	{ .client = IPCC_CLIENT_APSS_NS0 },
	{ .client = IPCC_CLIENT_APSS_NS1 },
	{ .client = IPCC_CLIENT_APSS_NS2 },
	{ .client = IPCC_CLIENT_APSS_NS3 },
	{ .client = IPCC_CLIENT_ADSP1 },
	{ .client = IPCC_CLIENT_ADSP2 },
	{ .client = IPCC_CLIENT_APSS_NS4 },
	{ .client = IPCC_CLIENT_DUMMY },
	{ .client = IPCC_CLIENT_SOCCP },
	{ .client = IPCC_CLIENT_CPUCP },
	{ .client = IPCC_CLIENT_A78CSS },
	{
		/* Reached over the TME mailbox doorbell, not the router. */
		.client = IPCC_CLIENT_TMESS,
		.backend = IPCC_BACKEND_DIRECT,
	},
	{ .client = IPCC_CLIENT_SAIL0 },
	{ .client = IPCC_CLIENT_SAIL1 },
	{ .client = IPCC_CLIENT_SAIL2 },
	{ .client = IPCC_CLIENT_SAIL3 },
	{ .client = IPCC_CLIENT_SAIL4 },
	{ .client = IPCC_CLIENT_SAIL5 },			/* physical index 24 */
};

/* Only MPROC is described; protocols 1 to 4 exist but EL3 does not use them. */
static const struct ipcc_proto_cfg protocols[] = {
	[IPCC_PROTO_MPROC] = {
		.protocol_id = IPCC_PROTO_MPROC,
		.num_sigs = IPCC_MPROC_NUM_SIGS,
		.num_clients = ARRAY_SIZE(mproc_clients),
		.proto_block_base = IPCC_BASE,
		.has_router = true,
		.clients = mproc_clients,
		.backends = IPCC_CAP_DIRECT | IPCC_CAP_ROUTER,
		.proto_stride = IPCC_PROTO_STRIDE,
		.client_stride = IPCC_CLIENT_STRIDE,
		.backend_ops = {
			[IPCC_BACKEND_DIRECT] = &ipcc_direct_ops,
			[IPCC_BACKEND_ROUTER] = &ipcc_router_ops,
		},
	},
};

static const struct ipcc_cfg ipcc_cfg = {
	.protocols = protocols,
	.num_protocols = ARRAY_SIZE(protocols),
	.client = IPCC_CLIENT_APSS_S,
	.ctrl_block_base = IPCC_BASE,
	.has_ctrl_block = false,
	.flags = IPCC_CFG_ROUTER_MODE | IPCC_CFG_HW_MEM_OPT,
};

const struct ipcc_cfg *const ipcc_chipset_cfg = &ipcc_cfg;

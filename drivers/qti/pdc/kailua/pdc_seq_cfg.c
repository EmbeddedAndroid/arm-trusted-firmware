/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/qti/pdc/pdc_seq.h>

static uint8_t apps_branches[PDC_SEQ_BR_ADDR_REG_COUNT] = { 0 };
static uint32_t apps_delays[PDC_SEQ_DELAY_REG_COUNT] = { 0 };

static struct pdc_seq_cfg apps_pdc_cfg = {
	PDC_SEQ_ARRAY(apps_branches),
	PDC_SEQ_ARRAY(apps_delays),
};

/* The AOP loads the apps PDC sleep and wake sequences on SM8550. */
static struct pdc_seq pdc_seq_instance = {
	PDC_SEQ_APPS,
	&apps_pdc_cfg,
	NULL,
	0U,
	0x200000U,	/* PDC offset from AOSS base */
	0xA00000U,	/* RSC offset from SS base   */
};

struct pdc_seq *g_pdc_seqs = &pdc_seq_instance;
uint32_t g_pdc_seq_count = 1U;

/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/cmd_db/cmd_db.h>
#include <drivers/qti/lmh/lmh.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define LLM_CFG_7			0x0c4U
#define LLM_CFG_7_LLM_EN		BIT_32(0)
#define LLM_CLX_RX_CFG_1_0		0x610U
#define LLM_CLX_RX_CFG_1_1		0x614U
#define LLM_CLX_RX_MSG_RPID_SHIFT	16U

/*
 * The voltage limit messages an LLM sends carry the RPMh id of the CPU rail
 * (CLX_RX_CFG_1_0) and of MX (CLX_RX_CFG_1_1): bits [15:8] of the resource
 * address in the command DB.
 */
static uint32_t llm_rpid(const char *res)
{
	uint32_t addr = cmd_db_query_addr(res);

	if (addr == 0U) {
		WARN("LMh: no %s in the command DB\n", res);
	}

	return ((addr >> 8) & 0xffU) << LLM_CLX_RX_MSG_RPID_SHIFT;
}

static uint32_t llm_value(uintptr_t addr, uint32_t val)
{
	unsigned int i;

	for (i = 0U; i < lmh_llm_rail_count; i++) {
		if (addr == lmh_llm_rails[i].base + LLM_CLX_RX_CFG_1_0) {
			return val | llm_rpid(lmh_llm_rails[i].rail);
		}
		if (addr == lmh_llm_rails[i].base + LLM_CLX_RX_CFG_1_1) {
			return val | llm_rpid("vrm.mx");
		}
	}

	return val;
}

void qti_lmh_init(void)
{
	unsigned int i;

	lmh_plat_clock_init();

	for (i = 0U; i < lmh_llm_cfg_count; i++) {
		mmio_write_32(lmh_llm_cfg[i].addr,
			      llm_value(lmh_llm_cfg[i].addr, lmh_llm_cfg[i].val));
	}

	for (i = 0U; i < lmh_llm_rail_count; i++) {
		if ((mmio_read_32(lmh_llm_rails[i].base + LLM_CFG_7) &
		     LLM_CFG_7_LLM_EN) == 0U) {
			WARN("LMh: LLM 0x%lx not enabled\n",
			     lmh_llm_rails[i].base);
		}
	}
}

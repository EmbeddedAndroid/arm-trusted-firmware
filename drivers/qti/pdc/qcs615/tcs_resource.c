/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_tcs.h>
#include <drivers/qti/pdc/tcs_resource.h>

#define TCS_RESOURCE_CX		"cx.lvl"
#define TCS_RESOURCE_MX		"mx.lvl"
#define TCS_RESOURCE_XO		"xo.lvl"
#define TCS_RESOURCE_SOC	"vrm.soc"

#define RES_CX_OFF	0U
#define RES_CX_RET	1U
#define RES_CX_MOL	2U	/* SVS3 (min_svs) */

#define RES_XO_OFF	0U
#define RES_XO_MOL	3U	/* ON */

#define VRM_SOC_OFF	0U
#define VRM_SOC_ON	1U

/* The vrm.soc enable vote is at offset 0x4 of the resource. */
struct pdc_tcs_resource g_pdc_resource_list[TCS_TOTAL_RESOURCE_NUM] = {
	{ TCS_RESOURCE_CX  },
	{ TCS_RESOURCE_MX  },
	{ TCS_RESOURCE_XO  },
	{ TCS_RESOURCE_SOC, 0x4U },
};

#define TCS_UNUSED	{ { 0U }, { 0U, TCS_CFG_OPT_NOT_USED, 0U } }

/* QCS615 does not vote MX from the apps PDC. */
struct pdc_tcs_config g_pdc_tcs_config[TCS_NUM_TOTAL][NUM_COMMANDS_PER_TCS] = {
	/* TCS 0 - Sleep: CX retention */
	{
		{ { RES_IDX_XO  }, { RES_XO_OFF, TCS_CFG_OPT_NONE, 0U } },
		{ { RES_IDX_CX  }, { RES_CX_RET, TCS_CFG_OPT_NONE, 0U } },
		{ { RES_IDX_SOC }, { VRM_SOC_OFF, TCS_CFG_OPT_NONE, 0U } },
		TCS_UNUSED,
	},
	/* TCS 1 - Sleep: CX off */
	{
		{ { RES_IDX_XO  }, { RES_XO_OFF, TCS_CFG_OPT_NONE, 0U } },
		{ { RES_IDX_CX  }, { RES_CX_OFF, TCS_CFG_OPT_NONE, 0U } },
		{ { RES_IDX_SOC }, { VRM_SOC_OFF, TCS_CFG_OPT_NONE, 0U } },
		TCS_UNUSED,
	},
	/* TCS 2 - Sleep: unused (matches DV/VI sequences) */
	{
		{ { RES_IDX_XO  }, { RES_XO_OFF, TCS_CFG_OPT_NONE, 0U } },
		{ { RES_IDX_CX  }, { RES_CX_OFF, TCS_CFG_OPT_NONE, 0U } },
		{ { RES_IDX_SOC }, { VRM_SOC_OFF, TCS_CFG_OPT_NONE, 0U } },
		TCS_UNUSED,
	},
	/* TCS 3 - Wake: resources to MOL */
	{
		{ { RES_IDX_CX  }, { RES_CX_MOL, TCS_CFG_OPT_CMD_RESP_REQ, 0U } },
		{ { RES_IDX_XO  }, { RES_XO_MOL, TCS_CFG_OPT_CMD_RESP_REQ, 0U } },
		{ { RES_IDX_SOC }, { VRM_SOC_ON,  TCS_CFG_OPT_CMD_RESP_REQ, 0U } },
		TCS_UNUSED,
	},
};

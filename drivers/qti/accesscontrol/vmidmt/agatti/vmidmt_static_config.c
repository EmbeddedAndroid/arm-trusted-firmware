/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <lib/utils_def.h>
#include <vmidmt.h>
#include <vmidmt_internal.h>
#include <vmidmt_target_hwio.h>

/* The VMIDMTs keep the configuration left by the boot loaders. */
const struct vmidmt_map g_vmid_map[] = {
};

const uint32_t g_vmid_map_count = ARRAY_SIZE(g_vmid_map);

const struct vmidmt_cfg g_vmidmt_cfg[] = {
};

const uint32_t g_vmidmt_cfg_count = ARRAY_SIZE(g_vmidmt_cfg);

struct hal_vmidmt_port_map g_vmidmt_info_cfg[] = {
};

const uint8_t g_vmidmt_info_cfg_count = ARRAY_SIZE(g_vmidmt_info_cfg);

/*
 * Mapping of VMIDMT position in VMIDMT error interrupt status register
 * to corresponding HAL VMIDMT index
 */
struct vmidmt_err_pos_to_hal_map vmidmt_err_pos_to_hal_map
	[ACC_VMIDMT_ERR_INT_STATUS_REG_NUM][ACC_VMIDMT_ERR_NUM_PER_REG] = {
		{
			{ 0, HAL_VMIDMT_CRYPTO0_BAM },
			{ 1, HAL_VMIDMT_RPM_MSGRAM },
			{ 2, HAL_VMIDMT_QUPV3_0 },
			{ 3, HAL_VMIDMT_COUNT },
			{ 4, HAL_VMIDMT_QPIC_BAM },
			{ 5, HAL_VMIDMT_COUNT },
			{ 6, HAL_VMIDMT_COUNT },
			{ 7, HAL_VMIDMT_COUNT },
			{ 8, HAL_VMIDMT_COUNT },
			{ 9, HAL_VMIDMT_IPA },
			{ 10, HAL_VMIDMT_QDSS_VMIDETR },
			{ 11, HAL_VMIDMT_QDSS_VMIDDAP },
			{ 12, HAL_VMIDMT_COUNT },
			{ 13, HAL_VMIDMT_SSC_QUPV3 },
			{ 14, HAL_VMIDMT_LPASS_RXTX },
			{ 15, HAL_VMIDMT_LPASS_AUD },
			{ 16, HAL_VMIDMT_LPASS_VA },
			{ 17, HAL_VMIDMT_LPASS_AUD_SB },
		},
	};

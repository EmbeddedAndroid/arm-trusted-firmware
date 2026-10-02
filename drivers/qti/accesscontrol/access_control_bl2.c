/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <drivers/qti/accesscontrol/accesscontrol.h>
#include <xpu_target_info.h>

void qti_accesscontrol_bl2_lock(void)
{
	xpu_lock_down_assets(msm_xpu_cfg, msm_xpu_cfg_count);
	xpu_lock_down_assets(msm_xpu_bl2_cfg, msm_xpu_bl2_cfg_count);
	dsbsy();
}

void qti_accesscontrol_bl2_release(void)
{
	xpu_release_assets(msm_xpu_bl2_cfg, msm_xpu_bl2_cfg_count);
	dsbsy();
}

/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/image_decompress.h>
#include <drivers/io/io_storage.h>
#include <drivers/qti/accesscontrol/accesscontrol.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>
#include <qti_uart_console.h>

static console_t g_qti_console_uart;

void bl2_early_platform_setup2(u_register_t x0, u_register_t x1,
			       u_register_t x2, u_register_t x3)
{
	qti_console_uart_register(&g_qti_console_uart,
				  PLAT_QTI_UART_BASE);
	console_set_scope(&g_qti_console_uart,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH);
}

void bl2_plat_arch_setup(void)
{
	int ret;

#if SEPARATE_BL2_NOLOAD_REGION
	mmap_add_region(BL2_NOLOAD_START, BL2_NOLOAD_START,
			BL2_NOLOAD_LIMIT - BL2_NOLOAD_START,
			MT_MEMORY | MT_RW | MT_SECURE);
#endif
	qti_setup_page_tables(BL2_BASE,
			      BL2_SIZE,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END);
	enable_mmu_el3(0);

#if QTI_BL2_ACCESS_CONTROL
	qti_accesscontrol_bl2_lock();
#endif

	ret = qti_io_setup();
	if (ret) {
		ERROR("failed to setup io devices\n");
		plat_error_handler(ret);
	}
}

void bl2_platform_setup(void)
{
}

void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

bl_load_info_t *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

bl_params_t *plat_get_next_bl_params(void)
{
	return get_next_bl_params_from_mem_params_desc();
}

void bl2_plat_preload_setup(void)
{
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	struct image_info *image_info;
	unsigned int attr = MT_MEMORY | MT_RW | MT_NS;
#if QTI_BL2_ACCESS_CONTROL
	entry_point_info_t *ep = &get_bl_mem_params_node(image_id)->ep_info;

	/* The secure image carve-outs reject non-secure writes by now. */
	if (GET_SECURITY_STATE(ep->h.attr) == SECURE) {
		attr = MT_MEMORY | MT_RW | MT_SECURE;
	}
#endif

	image_info = qti_get_image_info(image_id);

	return mmap_add_dynamic_region(image_info->image_base,
				      image_info->image_base,
				      image_info->image_max_size, attr);
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return 0;
}

#if QTI_BL2_ACCESS_CONTROL
void bl2_plat_prepare_exit(void)
{
	qti_accesscontrol_bl2_release();
}
#endif

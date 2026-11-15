/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/cmd_db/cmd_db.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>

#include <platform_def.h>
#include <qti_plat.h>

/* Features the GPU driver asks for */
#define GPU_ALWAYS_EN			(1U << 0)
#define GPU_BCL_EN			(1U << 1)
#define GPU_CLX_EN			(1U << 2)
#define GPU_TSENSE_EN			(1U << 3)

/* Adreno GPU CX registers (GFX 0x03d00000) */
#define GPU_CX_MISC_SLICE_ENABLE_MASK	0x03d9e210U
#define GPU_CX_MISC_SW_FUSE_VALUE	0x03d9f000U
#define GPU_CX_MISC_SW_FUSE_SLICE_EN	0x03d9f008U
#define GPU_CX_MISC_SW_FUSE_FREQ_LIMIT_CNTL 0x03d9f010U
#define GMU_CBCAST_DATA_EN		0x03d80008U
#define GMU_CBCAST_INTERFACE_EN		0x03d81000U
#define GMU_CBCAST_GX_VRM_0_ID		0x03d81040U
#define GMU_CBCAST_MX_VRM_0_ID		0x03d81044U
#define GMU_CBCAST_CX_VRM_0_ID		0x03d81048U
#define GMU_CBCAST_GX_VRM_1_ID		0x03d81050U
#define GMU_CBCAST_MX_VRM_1_ID		0x03d81088U

#define CBCAST_DATA_EN_VOLTAGE		(1U << 1)

/*
 * LPAC, ray tracing and fast blend, and frequency limit select 1: the values
 * the Kaanapali stock firmware writes; the SecaAU stock values are not known.
 */
#define SW_FUSE_FEATURES		0x7U
#define SW_FUSE_SLICES			0x7U
#define SW_FUSE_FREQ_LIMIT_SEL		0x1U

static spinlock_t gpu_lock;

bool plat_qti_gpu_init_regs_supported(void)
{
	return true;
}

/* CBCAST id of an RPMh VRM: <id> and the VRM address (bits 15:8) */
static uint32_t vrm_id(uint32_t addr, uint32_t id)
{
	return (id << 8) | ((addr >> 8) & 0xffU);
}

static int gpu_always_en(void)
{
	/* All slices are off at reset: enable those the part has. */
	uint32_t slices = mmio_read_32(GPU_CX_MISC_SLICE_ENABLE_MASK) &
			  SW_FUSE_SLICES;
	uint32_t gfx, mxg, cx;

	mmio_write_32(GPU_CX_MISC_SW_FUSE_VALUE, SW_FUSE_FEATURES);
	mmio_write_32(GPU_CX_MISC_SW_FUSE_SLICE_EN, slices);
	mmio_write_32(GPU_CX_MISC_SW_FUSE_FREQ_LIMIT_CNTL,
		      SW_FUSE_FREQ_LIMIT_SEL);

	gfx = cmd_db_query_addr("vrm.gfx");
	mxg = cmd_db_query_addr("vrm.mxg");
	cx = cmd_db_query_addr("vrm.cx");
	if ((gfx == 0U) || (mxg == 0U) || (cx == 0U)) {
		ERROR("GPU: VRM missing from the command DB\n");
		return -1;
	}

	mmio_setbits_32(GMU_CBCAST_INTERFACE_EN, 1U);
	mmio_write_32(GMU_CBCAST_GX_VRM_0_ID, vrm_id(gfx, 3U));
	mmio_write_32(GMU_CBCAST_GX_VRM_1_ID, vrm_id(gfx, 2U));
	mmio_write_32(GMU_CBCAST_MX_VRM_0_ID, vrm_id(mxg, 3U));
	mmio_write_32(GMU_CBCAST_MX_VRM_1_ID, vrm_id(mxg, 2U));
	mmio_write_32(GMU_CBCAST_CX_VRM_0_ID, vrm_id(cx, 3U));
	mmio_setbits_32(GMU_CBCAST_DATA_EN, CBCAST_DATA_EN_VOLTAGE);

	return 0;
}

int plat_qti_gpu_init_regs(uint32_t mask)
{
	int ret = 0;

	spin_lock(&gpu_lock);
	if ((mask & GPU_ALWAYS_EN) != 0U) {
		ret = gpu_always_en();
	}
	spin_unlock(&gpu_lock);

	/* The GPU sensor and BCL broadcast ids of SecaAU are not known. */
	if ((mask & (GPU_BCL_EN | GPU_CLX_EN | GPU_TSENSE_EN)) != 0U) {
		NOTICE("GPU: BCL, CLX and TSENSE not set up (mask 0x%x)\n", mask);
	}

	return ret;
}

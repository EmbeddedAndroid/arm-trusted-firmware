/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>
#include <xpu3.h>
#include <xpu_target_info.h>

#include <platform_def.h>

/*
 * XBL_SEC leaves the DDR and pIMEM MPUs enabled with their unmapped regions
 * open to the normal world. It keeps its own pages secure (BIMC RG 19 holds
 * 0x60000000-0x61800000, which also backs the pIMEM window; pIMEM RG 2 holds
 * its 8 KiB) and leaves the other resource groups free. Take a free one on
 * each MPU so that only the secure world reaches BL31 in pIMEM and BL32 in
 * DDR. The secure owner gets read and write access with the ownership.
 */
#define BIMC_DDR0_BL32_RG	1
#define PIMEM_MPU_BL31_RG	7

/* The pIMEM MPU matches offsets into the pIMEM aperture. */
#define PIMEM_OFFSET(addr)	((addr) - QTI_PIMEM_BASE)

/* BL2 in pIMEM sits right below BL31 and takes the same resource group. */
#if defined(IMAGE_BL2) && (BL2_BASE >= QTI_PIMEM_BASE)
#define PIMEM_SECURE_BASE	BL2_BASE
#else
#define PIMEM_SECURE_BASE	BL31_BASE
#endif

static struct rg_domain_ownership bimc_ddr0_rgs[] = {
	{ BIMC_DDR0_BL32_RG, APPS_S_DOMAIN },
};

static struct rg_partition_range bimc_ddr0_rg_addr[] = {
	{ BIMC_DDR0_BL32_RG, BL32_BASE, BL32_LIMIT },
};

static struct rg_domain_ownership pimem_mpu_rgs[] = {
	{ PIMEM_MPU_BL31_RG, APPS_S_DOMAIN },
};

static struct rg_partition_range pimem_mpu_rg_addr[] = {
	{ PIMEM_MPU_BL31_RG, PIMEM_OFFSET(PIMEM_SECURE_BASE),
	  PIMEM_OFFSET(BL31_LIMIT) },
};

struct xpu_instance msm_xpu_cfg[] = {
	{ HWIO_BIMC_S_DDR0_XPU3_GCR0_ADDR, ARRAY_SIZE(bimc_ddr0_rgs),
	  bimc_ddr0_rgs, ARRAY_SIZE(bimc_ddr0_rg_addr), bimc_ddr0_rg_addr,
	  XPU_TYPE_BIMC_MPU0, XPU_PROTECTION_STATIC },
	{ HWIO_RAMBLUR_PIMEM_MPU_XPU3_GCR0_ADDR, ARRAY_SIZE(pimem_mpu_rgs),
	  pimem_mpu_rgs, ARRAY_SIZE(pimem_mpu_rg_addr), pimem_mpu_rg_addr,
	  XPU_TYPE_RAMBLUR_PIMEM_MPU, XPU_PROTECTION_STATIC },
};

const uint32_t msm_xpu_cfg_count = ARRAY_SIZE(msm_xpu_cfg);

/*
 * XBL loads the FIP to PLAT_QTI_FIP_IOBASE in plain DDR. BL2 holds that
 * window in another free resource group of the DDR MPU while it reads the
 * FIP, and frees it again before BL33 can use the memory.
 */
#define BIMC_DDR0_FIP_RG	2

static struct rg_domain_ownership bimc_ddr0_fip_rgs[] = {
	{ BIMC_DDR0_FIP_RG, APPS_S_DOMAIN },
};

static struct rg_partition_range bimc_ddr0_fip_rg_addr[] = {
	{ BIMC_DDR0_FIP_RG, PLAT_QTI_FIP_IOBASE,
	  PLAT_QTI_FIP_IOBASE + PLAT_QTI_FIP_MAXSIZE },
};

struct xpu_instance msm_xpu_bl2_cfg[] = {
	{ HWIO_BIMC_S_DDR0_XPU3_GCR0_ADDR, ARRAY_SIZE(bimc_ddr0_fip_rgs),
	  bimc_ddr0_fip_rgs, ARRAY_SIZE(bimc_ddr0_fip_rg_addr),
	  bimc_ddr0_fip_rg_addr, XPU_TYPE_BIMC_MPU0, XPU_PROTECTION_STATIC },
};

const uint32_t msm_xpu_bl2_cfg_count = ARRAY_SIZE(msm_xpu_bl2_cfg);

/* The modem is not brought up, so there are no modem MPU ranges. */
struct mpu_ranges msm_mpu_ranges[] = {
};

const uint32_t msm_mpu_ranges_count = ARRAY_SIZE(msm_mpu_ranges);

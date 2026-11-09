/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DRTM (Arm DEN0113) platform hooks for QTI platforms.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <event_measure.h>
#include <lib/utils_def.h>
#include <plat/common/plat_drtm.h>
#include <plat/common/platform.h>
#include <services/drtm_svc.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * Non-secure DRAM a DLME, its data and the DRTM parameters may live in:
 * the DDR above the last firmware carveout (QTEE, trusted applications,
 * TZ statistics and CPUCP firmware end at 0xdb300000), up to the 64 GiB
 * the platform decodes.
 */
#define QTI_DRTM_NS_DRAM_BASE	ULL(0xdb300000)
#define QTI_DRTM_NS_DRAM_END	ULL(0x1000000000)

/*
 * AOP, SAIL, CPUCP and the other always-running subsystem processors reach
 * DRAM outside the SMMUs. They run trusted firmware whose DRAM access is
 * fenced by the xPU/MPU configuration, so they are non-host platforms in the
 * DRTM sense.
 */
bool plat_has_non_host_platforms(void)
{
	return true;
}

/*
 * Every other bus master on the SoC (DSPs, camera, video, display, PCIe and
 * USB controllers, the GPU) reaches DRAM through one of the SMMU-500
 * instances below.
 */
bool plat_has_unmanaged_dma_peripherals(void)
{
	return false;
}

static const uintptr_t qti_drtm_smmus[] = {
	QTI_DRTM_SMMU_BASES
};

unsigned int plat_get_total_smmus(void)
{
	return ARRAY_SIZE(qti_drtm_smmus);
}

void plat_enumerate_smmus(const uintptr_t **smmus_out, size_t *smmu_count_out)
{
	*smmus_out = qti_drtm_smmus;
	*smmu_count_out = ARRAY_SIZE(qti_drtm_smmus);
}

static const plat_drtm_dma_prot_features_t qti_dma_prot_features = {
	.max_num_mem_prot_regions = 0,
	.dma_protection_support = ARM_DRTM_DMA_PROT_FEATURES_DMA_SUPPORT_COMPLETE
};

const plat_drtm_dma_prot_features_t *plat_drtm_get_dma_prot_features(void)
{
	return &qti_dma_prot_features;
}

uint64_t plat_drtm_dma_prot_get_max_table_bytes(void)
{
	return 0ULL;
}

/*
 * The TPM is a discrete chip owned by OP-TEE: BL31 records the DRTM
 * measurements in the event log; their extension into PCR 17/18 at
 * locality 4 is done by OP-TEE (not wired yet).
 */
static const plat_drtm_tpm_features_t qti_tpm_features = {
	.tpm_based_hash_support = false,
	.firmware_hash_algorithm = TPM_ALG_ID
};

const plat_drtm_tpm_features_t *plat_drtm_get_tpm_features(void)
{
	return &qti_tpm_features;
}

int plat_drtm_validate_ns_region(uintptr_t region_start, size_t region_size)
{
	uint64_t start = region_start;
	uint64_t end = start + region_size;

	if (!region_size || end < start)
		return -1;

	if (start < QTI_DRTM_NS_DRAM_BASE || end > QTI_DRTM_NS_DRAM_END)
		return -1;

	return 0;
}

/* No persistent storage for the DRTM error code is wired yet */
static uint64_t qti_drtm_error;

int plat_set_drtm_error(uint64_t error_code)
{
	qti_drtm_error = error_code;

	return 0;
}

int plat_get_drtm_error(uint64_t *error_code)
{
	*error_code = qti_drtm_error;

	return 0;
}

uint64_t plat_drtm_get_min_size_normal_world_dce(void)
{
	return 0ULL;
}

uint64_t plat_drtm_get_imp_def_dlme_region_size(void)
{
	return 0ULL;
}

uint64_t plat_drtm_get_tcb_hash_features(void)
{
	return 0ULL;
}

uint64_t plat_drtm_get_tcb_hash_table_size(void)
{
	return 0ULL;
}

uint64_t plat_drtm_get_acpi_tables_region_size(void)
{
	return 0ULL;
}

uint64_t plat_drtm_get_dlme_img_auth_features(void)
{
	return 0ULL;
}

/* The DLME address map is the BL31 memory map */
const mmap_region_t *plat_get_addr_mmap(void)
{
	return plat_qti_mmap;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}

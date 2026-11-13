/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DRTM (Arm DEN0113) platform hooks for QTI platforms.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/arm/smmu_v2.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/pmic/spmi_arb.h>
#include <drivers/qti/smem/smem.h>
#include <event_measure.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/plat_drtm.h>
#include <plat/common/platform.h>
#include <services/drtm_svc.h>
#include <services/el3_spmd_logical_sp.h>
#include <services/ffa_svc.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * AOP, SAIL, CPUCP and the other always-running subsystem processors reach
 * DRAM outside the SMMUs and run trusted firmware: they are non-host
 * platforms in the DRTM sense.
 */
bool plat_has_non_host_platforms(void)
{
	return true;
}

/*
 * The other bus masters on the SoC (DSPs, camera, video, display, PCIe and
 * USB controllers, the GPU) reach DRAM through one of the SMMU-500 instances
 * below. The GIC is the exception: its redistributors read and write the
 * LPI tables in DRAM outside any SMMU, which the generic DMA protection
 * does not restrict yet (see the TODO in drtm_dma_prot.c).
 */
bool plat_has_unmanaged_dma_peripherals(void)
{
	return false;
}

static const uintptr_t qti_drtm_smmus[] = {
	QTI_DRTM_SMMU_BASES,
#ifdef QTI_DRTM_GPU_SMMU_BASE
	QTI_DRTM_GPU_SMMU_BASE,
#endif
};

#define QTI_GDSCR_PWR_ON	BIT_32(31)

/* Branch clock vote register: enable, and the branch's clock-off status */
#define QTI_CLK_VOTE_ENABLE	BIT_32(0)
#define QTI_CLK_VOTE_CLK_OFF	BIT_32(31)
#define QTI_CLK_VOTE_TIMEOUT_US	U(1000)

/*
 * The GPU SMMU registers read as zero and ignore writes while the GPU CX
 * domain is collapsed, and the GPU behind it cannot reach DRAM then either:
 * engage the GPU SMMU only when GPU CX is up. The DLME owns the GPU power
 * after the launch.
 */
static size_t qti_drtm_num_smmus(void)
{
	size_t n = ARRAY_SIZE(qti_drtm_smmus);

#ifdef QTI_DRTM_GPU_SMMU_BASE
	uint32_t gdscr = mmio_read_32(QTI_DRTM_GPU_CX_GDSCR);

	if ((gdscr & QTI_GDSCR_PWR_ON) == 0U) {
		INFO("DRTM: GPU CX off (GDSCR 0x%x), GPU SMMU not engaged\n",
		     gdscr);
		n--;
	}
#endif
	return n;
}

/*
 * Both count the GPU SMMU only while GPU CX is up, so the check at DRTM
 * init that every SMMU was enumerated holds whatever the GPU power state:
 * with CX off the GPU SMMU is absent rather than missed.
 */
unsigned int plat_get_total_smmus(void)
{
	return (unsigned int)qti_drtm_num_smmus();
}

void plat_enumerate_smmus(const uintptr_t **smmus_out, size_t *smmu_count_out)
{
	*smmus_out = qti_drtm_smmus;
	*smmu_count_out = qti_drtm_num_smmus();
}

#ifdef QTI_DRTM_GPU_SMMU_BASE
/*
 * An access to the GPU SMMU registers hangs while the GPU SMMU clock is off,
 * which it may be with GPU CX up: vote it on for the engage.
 */
static int qti_drtm_engage_gpu_smmu(void)
{
	uint64_t timeout = timeout_init_us(QTI_CLK_VOTE_TIMEOUT_US);
	int rc = -1;

	mmio_setbits_32(QTI_DRTM_GPU_SMMU_CLK_VOTE, QTI_CLK_VOTE_ENABLE);
	do {
		if ((mmio_read_32(QTI_DRTM_GPU_SMMU_CLK_VOTE) &
		     QTI_CLK_VOTE_CLK_OFF) == 0U) {
			rc = smmuv2_ns_set_abort_all(QTI_DRTM_GPU_SMMU_BASE);
			break;
		}
	} while (!timeout_elapsed(timeout));

	if (rc != 0) {
		ERROR("DRTM: GPU SMMU not engaged (clock vote 0x%x)\n",
		      mmio_read_32(QTI_DRTM_GPU_SMMU_CLK_VOTE));
	}
	mmio_clrbits_32(QTI_DRTM_GPU_SMMU_CLK_VOTE, QTI_CLK_VOTE_ENABLE);

	return rc;
}
#endif

int plat_drtm_engage_smmu(uintptr_t smmu_base)
{
#ifdef QTI_DRTM_GPU_SMMU_BASE
	if (smmu_base == QTI_DRTM_GPU_SMMU_BASE) {
		return qti_drtm_engage_gpu_smmu();
	}
#endif
	return smmuv2_ns_set_abort_all(smmu_base);
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
 * The TPM is a discrete chip owned by OP-TEE: BL31 hashes the DRTM events
 * itself (event log) and OP-TEE extends PCR 17/18 at locality 4, see
 * plat_drtm_tpm_extend() below.
 */
static const plat_drtm_tpm_features_t qti_tpm_features = {
	.tpm_based_hash_support = false,
	.firmware_hash_algorithm = EVLOG_TPM_ALG_SHA256,
	.dlme_locality_support = true
};

const plat_drtm_tpm_features_t *plat_drtm_get_tpm_features(void)
{
	return &qti_tpm_features;
}

/*
 * SMEM usable RAM partition table (item 402), version 2 and later: the DDR
 * banks the boot firmware found. A DCE read outside them hits no memory.
 */
#define QTI_SMEM_RAM_PARTITION_TABLE	402U
#define QTI_RAM_PART_MAGIC1		U(0x9da5e0a8)
#define QTI_RAM_PART_MAGIC2		U(0xaf9ec4e2)
#define QTI_RAM_PART_SDRAM		U(14)
#define QTI_RAM_PART_SYS_MEMORY		U(1)

struct qti_ram_part_entry {
	char name[16];
	uint64_t start;
	uint64_t length;
	uint32_t attribute;
	uint32_t category;
	uint32_t domain;
	uint32_t type;
	uint32_t num_partitions;
	uint32_t hw_info;
	uint8_t highest_bank_bit;
	uint8_t reserved[3];
	uint32_t min_pasr_size;
	uint64_t available_length;
};

struct qti_ram_part_table {
	uint32_t magic1;
	uint32_t magic2;
	uint32_t version;
	uint32_t reserved1;
	uint32_t num_partitions;
	uint32_t reserved2;
	struct qti_ram_part_entry entries[];
};

/* The partition table in SMEM, or NULL; @count entries are within the item */
static const struct qti_ram_part_table *qti_drtm_ram_parts(uint32_t *count)
{
	const struct qti_ram_part_table *t;
	size_t size;

	if (qti_smem_lookup(QTI_SMEM_HOST_COMMON, QTI_SMEM_RAM_PARTITION_TABLE,
			    QTI_SMEM_FLAG_NONE, (void **)&t, &size) != 0 ||
	    size < sizeof(*t) || t->magic1 != QTI_RAM_PART_MAGIC1 ||
	    t->magic2 != QTI_RAM_PART_MAGIC2 || t->version < 2U) {
		WARN("DRTM: no DDR partition table in SMEM\n");
		return NULL;
	}

	*count = MIN(t->num_partitions,
		     (uint32_t)((size - sizeof(*t)) / sizeof(t->entries[0])));

	return t;
}

static bool qti_drtm_is_ddr(const struct qti_ram_part_entry *e)
{
	return (e->category == QTI_RAM_PART_SDRAM) &&
	       (e->type == QTI_RAM_PART_SYS_MEMORY);
}

/*
 * The DLME address map: the peripheral range as device memory and the DDR
 * banks of the SMEM partition table as normal memory. It is built once, at
 * DRTM setup before the normal world runs, as the SMEM item stays writable
 * by the normal world; DLME regions are checked against this copy.
 */
static mmap_region_t qti_drtm_addr_mmap[PLAT_DRTM_MMAP_ENTRIES + 1U];
static bool qti_drtm_addr_mmap_built;

const mmap_region_t *plat_get_addr_mmap(void)
{
	const struct qti_ram_part_table *t;
	uint32_t count = 0U;
	unsigned int n = 0U;
	uint32_t i;

	if (qti_drtm_addr_mmap_built) {
		return qti_drtm_addr_mmap;
	}

	qti_drtm_addr_mmap[n++] = (mmap_region_t)MAP_REGION_FLAT(
		QTI_DEVICE_BASE, QTI_DEVICE_SIZE, MT_DEVICE | MT_RW | MT_NS);

	t = qti_drtm_ram_parts(&count);
	for (i = 0U; (t != NULL) && (i < count); i++) {
		const struct qti_ram_part_entry *e = &t->entries[i];

		if (!qti_drtm_is_ddr(e) || (e->length == 0U)) {
			continue;
		}
		if (n == PLAT_DRTM_MMAP_ENTRIES) {
			ERROR("DRTM: DDR banks past %u left out of the address map\n",
			      PLAT_DRTM_MMAP_ENTRIES - 1U);
			break;
		}
		qti_drtm_addr_mmap[n++] = (mmap_region_t)MAP_REGION_FLAT(
			e->start, e->length, MT_MEMORY | MT_RW | MT_NS);
	}

	qti_drtm_addr_mmap[n] = (mmap_region_t){ 0 };
	qti_drtm_addr_mmap_built = true;

	return qti_drtm_addr_mmap;
}

static bool qti_drtm_in_ddr(uint64_t start, uint64_t end)
{
	const mmap_region_t *m;

	for (m = qti_drtm_addr_mmap; m->size != 0U; m++) {
		if ((MT_TYPE(m->attr) == MT_MEMORY) && (start >= m->base_pa) &&
		    (end <= m->base_pa + m->size)) {
			return true;
		}
	}

	return false;
}

int plat_drtm_validate_ns_region(uintptr_t region_start, size_t region_size)
{
	uint64_t start = region_start;
	uint64_t end = start + region_size;

	if ((region_size == 0U) || (end < start)) {
		return -1;
	}

	if ((start < QTI_DRTM_NS_DRAM_BASE) || !qti_drtm_in_ddr(start, end)) {
		return -1;
	}

	return 0;
}

#ifndef QTI_DRTM_ERROR_SDAM_ADDR
#error "QTI_DRTM_ERROR_SDAM_ADDR: no persistent slot for the DRTM error code"
#endif

/* DRTM_SET_ERROR and DRTM_GET_ERROR may come from any PE */
static spinlock_t qti_drtm_error_lock;

int plat_set_drtm_error(uint64_t error_code)
{
	unsigned int i;
	int ret = 0;

	spin_lock(&qti_drtm_error_lock);
	for (i = 0U; (i < sizeof(error_code)) && (ret == 0); i++) {
		ret = spmi_arb_write8(QTI_DRTM_ERROR_SDAM_ADDR + i,
				      (uint8_t)(error_code >> (8U * i)));
	}
	spin_unlock(&qti_drtm_error_lock);

	return (ret != 0) ? -1 : 0;
}

int plat_get_drtm_error(uint64_t *error_code)
{
	uint64_t val = 0ULL;
	unsigned int i;
	int ret = 0;

	spin_lock(&qti_drtm_error_lock);
	for (i = 0U; i < sizeof(val); i++) {
		ret = spmi_arb_read8(QTI_DRTM_ERROR_SDAM_ADDR + i);
		if (ret < 0) {
			break;
		}
		val |= (uint64_t)ret << (8U * i);
	}
	spin_unlock(&qti_drtm_error_lock);

	if (ret < 0) {
		return -1;
	}

	*error_code = val;

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

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}

/*
 * The TPM belongs to OP-TEE, the S-EL1 SPMC. BL31 records the launch in it
 * with FF-A direct requests from an SPMD logical partition to OP-TEE's
 * D-RTM measurement endpoint (CFG_TPM2_DRTM_LSP), which runs them at
 * locality 4. Request: x3 = command | argument << 8 (| the PCR banks
 * measured in << 16 for the launch event), x4-x7 = 32 bytes of payload;
 * response x3 = 0 on success. A SHA-384 digest does not fit in
 * one request: its first 32 bytes are staged, the extend runs with the
 * last 16.
 */
#define QTI_DRTM_SPMD_LP_ID		SPMD_LP_ID_START
#define QTI_DRTM_TPM_ENDPOINT_ID	U(0x8100)
#define QTI_DRTM_TPM_CMD_DL_EVENT	U(1)
#define QTI_DRTM_TPM_CMD_EXTEND		U(2)
#define QTI_DRTM_TPM_CMD_SHA384_LO	U(3)
#define QTI_DRTM_TPM_CMD_SHA384_HI	U(4)
#define QTI_DRTM_TPM_CMD_OPEN_LOCALITY	U(5)
#define QTI_DRTM_TPM_CMD_CLOSE_LOCALITY	U(6)
#define QTI_DRTM_TPM_DLME_LOCALITY	U(2)
#define QTI_DRTM_TPM_PAYLOAD_SIZE	U(32)
#define QTI_DRTM_TPM_X3(cmd, arg)	((uint64_t)(cmd) | ((uint64_t)(arg) << 8))
/* 0b6a42d5-7f1e-4c88-a3d1-6c2e8f5b9071 */
#define QTI_DRTM_SPMD_LP_UUID \
	{0xd5426a0b, 0x884c1e7f, 0x2e6cd1a3, 0x71905b8f}

static int qti_drtm_tpm_request(uint64_t x3, const uint8_t *data, size_t size)
{
	uint64_t payload[QTI_DRTM_TPM_PAYLOAD_SIZE / sizeof(uint64_t)] = { 0 };
	struct ffa_value ret;

	if (size > sizeof(payload)) {
		return -1;
	}
	if (size != 0U) {
		memcpy(payload, data, size);
	}

	if (!spmd_el3_ffa_msg_direct_req(
		    ((uint64_t)QTI_DRTM_SPMD_LP_ID << 16) |
		    QTI_DRTM_TPM_ENDPOINT_ID, 0U, x3,
		    payload[0], payload[1], payload[2], payload[3],
		    cm_get_context(NON_SECURE), &ret)) {
		return -1;
	}

	if (is_ffa_error(&ret) || (ret.arg3 != 0U)) {
		ERROR("DRTM: TPM request 0x%lx failed: 0x%lx 0x%lx\n",
		      x3, ret.func, ret.arg3);
		return -1;
	}

	return 0;
}

/* The event log banks as EFI_TCG2_BOOT_HASH_ALG bits, for the TPM owner */
static uint64_t qti_drtm_tpm_banks(void)
{
	static const uint16_t algs[] = { PLAT_DRTM_EVENT_LOG_ALGS };
	uint64_t banks = 0ULL;
	unsigned int i;

	for (i = 0U; i < ARRAY_SIZE(algs); i++) {
		switch (algs[i]) {
		case EVLOG_TPM_ALG_SHA256:
			banks |= BIT_64(1);
			break;
		case EVLOG_TPM_ALG_SHA384:
			banks |= BIT_64(2);
			break;
		default:
			break;
		}
	}

	return banks;
}

int plat_drtm_tpm_dl_event(const uint8_t *data, size_t size)
{
	return qti_drtm_tpm_request(QTI_DRTM_TPM_X3(QTI_DRTM_TPM_CMD_DL_EVENT,
						    size) |
				    (qti_drtm_tpm_banks() << 16),
				    data, size);
}

int plat_drtm_tpm_extend(unsigned int pcr, const void *digests)
{
	const uint8_t *d = digests;
	uint32_t count, i;
	uint16_t alg;
	int ret = 0;

	/* TPML_DIGEST_VALUES: count, then algorithm ID and digest per bank */
	memcpy(&count, d, sizeof(count));
	d += sizeof(count);

	for (i = 0U; (i < count) && (ret == 0); i++) {
		memcpy(&alg, d, sizeof(alg));
		d += sizeof(alg);

		switch (alg) {
		case EVLOG_TPM_ALG_SHA256:
			ret = qti_drtm_tpm_request(
				QTI_DRTM_TPM_X3(QTI_DRTM_TPM_CMD_EXTEND, pcr),
				d, SHA256_DIGEST_SIZE);
			d += SHA256_DIGEST_SIZE;
			break;
		case EVLOG_TPM_ALG_SHA384:
			/* 48 bytes: staged in two requests */
			ret = qti_drtm_tpm_request(
				QTI_DRTM_TPM_X3(QTI_DRTM_TPM_CMD_SHA384_LO, pcr),
				d, QTI_DRTM_TPM_PAYLOAD_SIZE);
			if (ret == 0) {
				ret = qti_drtm_tpm_request(
					QTI_DRTM_TPM_X3(QTI_DRTM_TPM_CMD_SHA384_HI,
							pcr),
					d + QTI_DRTM_TPM_PAYLOAD_SIZE,
					SHA384_DIGEST_SIZE -
					QTI_DRTM_TPM_PAYLOAD_SIZE);
			}
			d += SHA384_DIGEST_SIZE;
			break;
		default:
			ret = -1;
			break;
		}
	}

	return ret;
}

/*
 * OP-TEE serves the DLME's own D-RTM requests (locality 2, PCRs 19 to 22)
 * only between these two requests.
 */
int plat_drtm_tpm_dlme_locality(bool open)
{
	return qti_drtm_tpm_request(
		QTI_DRTM_TPM_X3(open ? QTI_DRTM_TPM_CMD_OPEN_LOCALITY :
				QTI_DRTM_TPM_CMD_CLOSE_LOCALITY,
				QTI_DRTM_TPM_DLME_LOCALITY), NULL, 0U);
}

static int32_t qti_drtm_spmd_lp_init(void)
{
	return 0;
}

DECLARE_SPMD_LOGICAL_PARTITION(
	qti_drtm,
	qti_drtm_spmd_lp_init,
	QTI_DRTM_SPMD_LP_ID,
	QTI_DRTM_SPMD_LP_UUID,
	FFA_PARTITION_DIRECT_REQ_SEND
);

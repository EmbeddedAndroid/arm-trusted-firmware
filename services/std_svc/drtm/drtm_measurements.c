/*
 * Copyright (c) 2022-2025 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM measurements into TPM PCRs.
 *
 * Authors:
 *      Lucian Paul-Trifu <lucian.paultrifu@gmail.com>
 *
 */
#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <event_measure.h>
#include <event_print.h>
#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/plat_drtm.h>

#include "drtm_main.h"
#include "drtm_measurements.h"
#include "drtm_remediation.h"

/*
 * Hash algorithms of the DRTM event log, one digest each per event: a
 * platform with several active TPM PCR banks lists them all.
 */
#ifndef PLAT_DRTM_EVENT_LOG_ALGS
#define PLAT_DRTM_EVENT_LOG_ALGS	TPM_ALG_ID
#endif

/* Event Log buffer */
static uint8_t drtm_event_log[PLAT_DRTM_EVENT_LOG_MAX_SIZE];

/* Hash algorithms of the DRTM event log, one digest each per event */
static const tpm_alg_id drtm_event_log_algs[] = {
	PLAT_DRTM_EVENT_LOG_ALGS,
};

/* The digest buffers of libeventlog hold HASH_ALG_COUNT digests at most */
CASSERT(ARRAY_SIZE(drtm_event_log_algs) <= HASH_ALG_COUNT,
	assert_drtm_event_log_algs_fit_libeventlog);

struct drtm_dlme_img_window {
	uintptr_t pa;
	size_t size;
	uintptr_t va;
	size_t mapped;
};

/*
 * Record an event whose digests are in @digests (TPML_DIGEST_VALUES) in the
 * event log and in the TPM. @data and @data_size are the measured data of
 * the DCE event.
 */
static int drtm_event_log_record(const uint8_t *digests, uint32_t event_type,
				 const char *event_name, unsigned int pcr,
				 const uint8_t *data, size_t data_size)
{
	int rc;
	size_t event_data_size =
		(event_name != NULL) ? (strlen(event_name) + 1U) : 0U;

	rc = event_log_write_pcr_event2(pcr, event_type,
					(const tpml_digest_values *)digests,
					(const uint8_t *)event_name,
					event_data_size);
	if (rc != 0) {
		return rc;
	}

	/*
	 * The DCE event is the dynamic launch event: the TPM resets the D-RTM
	 * PCRs and hashes its data into PCR 17 itself.
	 */
	if (event_type == DRTM_EVENT_ARM_DCE) {
		rc = plat_drtm_tpm_dl_event(data, data_size);
	} else {
		rc = plat_drtm_tpm_extend(pcr, digests);
	}

	/*
	 * The D-RTM PCRs may already hold part of this launch: record why and
	 * stop rather than start a DLME the TPM does not describe.
	 */
	if (rc != 0) {
		drtm_enter_remediation(DRTM_REMEDIATION_TPM,
				       "TPM measurement failed");
	}

	return 0;
}

/*
 * Calculate and write hash of various payloads as per DRTM specification
 * to Event Log.
 *
 * @param[in] data_base         Address of data
 * @param[in] data_size         Size of data
 * @param[in] event_type        Type of Event
 * @param[in] event_name        Name of the Event
 * @return:
 *      0 = success
 *    < 0 = error
 */
static int drtm_event_log_measure_and_record(uintptr_t data_base,
					     uint32_t data_size,
					     uint32_t event_type,
					     const char *event_name,
					     unsigned int pcr)
{
	int rc;
	unsigned char hash_data[MAX_TPML_BUFFER_SIZE];

	/*
	 * Measure the payloads requested by D-CRTM and DCE components
	 * Hash algorithm decided by the Event Log driver at build-time
	 */
	rc = event_log_measure(data_base, data_size, hash_data,
			       sizeof(hash_data));
	if (rc != 0) {
		return rc;
	}

	return drtm_event_log_record(hash_data, event_type, event_name, pcr,
				     (const uint8_t *)data_base, data_size);
}

static int drtm_dlme_img_unmap(struct drtm_dlme_img_window *w)
{
	int rc = 0;

	if (w->mapped != 0U) {
		rc = mmap_remove_dynamic_region(w->va, w->mapped);
		w->mapped = 0U;
	}

	return rc;
}

/* Map the piece of the DLME image at @offset in place of the previous one */
static int drtm_dlme_img_next(void *cookie, size_t offset, void **ptr,
			      size_t *len)
{
	struct drtm_dlme_img_window *w = cookie;
	size_t n = MIN(w->size - offset, DRTM_MAP_WINDOW_SIZE);
	size_t bytes = page_align(n, UP);
	int rc;

	rc = drtm_dlme_img_unmap(w);
	if (rc != 0) {
		return rc;
	}

	rc = mmap_add_dynamic_region_alloc_va(w->pa + offset, &w->va, bytes,
					      MT_RO_DATA | MT_NS);
	if (rc != 0) {
		WARN("DRTM: %s: mmap_add_dynamic_region() failed rc=%d\n",
		     __func__, rc);
		return rc;
	}
	w->mapped = bytes;

	*ptr = (void *)w->va;
	*len = n;

	return 0;
}

static size_t drtm_digest_size(tpm_alg_id alg)
{
	switch (alg) {
	case EVLOG_TPM_ALG_SHA256:
		return SHA256_DIGEST_SIZE;
	case EVLOG_TPM_ALG_SHA384:
		return SHA384_DIGEST_SIZE;
	case EVLOG_TPM_ALG_SHA512:
		return SHA512_DIGEST_SIZE;
	default:
		return 0U;
	}
}

/* Hash the DLME image in every algorithm of the event log */
static int drtm_measure_dlme_img(const struct_drtm_dl_args *a,
				 uint8_t *hash_buf, size_t hash_buf_size)
{
	struct drtm_dlme_img_window w = {
		.pa = a->dlme_paddr + a->dlme_img_off,
		.size = a->dlme_img_size,
	};
	tpml_digest_values *digests = (tpml_digest_values *)hash_buf;
	uint8_t *cursor = (uint8_t *)digests->digests;
	int rc = 0;

	if (w.size == 0U) {
		return -EINVAL;
	}

	digests->count = ARRAY_SIZE(drtm_event_log_algs);
	for (size_t i = 0U; i < ARRAY_SIZE(drtm_event_log_algs); i++) {
		tpmt_ha *ha = (tpmt_ha *)cursor;
		size_t size = drtm_digest_size(drtm_event_log_algs[i]);

		if ((size == 0U) ||
		    (sizeof(*ha) + size > hash_buf_size -
					  (size_t)(cursor - hash_buf))) {
			return -ENOMEM;
		}

		ha->algorithm_id = drtm_event_log_algs[i];
		rc = crypto_mod_tcg_hash_src(drtm_event_log_algs[i], w.size,
					     drtm_dlme_img_next, &w,
					     ha->digest);
		if (drtm_dlme_img_unmap(&w) != 0) {
			rc = -EINVAL;
		}
		if (rc != 0) {
			return rc;
		}

		cursor += sizeof(*ha) + size;
	}

	return 0;
}

/*
 * Initialise Event Log global variables, used during the recording
 * of various payload measurements into the Event Log buffer
 *
 * @param[in] event_log_start           Base address of Event Log buffer
 * @param[in] event_log_finish          End address of Event Log buffer,
 *                                      it is a first byte past end of the
 *                                      buffer
 */
static void drtm_event_log_init(uint8_t *event_log_start,
				uint8_t *event_log_finish)
{
	int rc;

	rc = event_log_init_and_reg(
		drtm_event_log, drtm_event_log + PLAT_DRTM_EVENT_LOG_MAX_SIZE,
		0U, crypto_mod_tcg_hash);
	if (rc < 0) {
		ERROR("Failed to initialize event log (%d).\n", rc);
		panic();
	}

	rc = event_log_write_header(drtm_event_log_algs,
				    ARRAY_SIZE(drtm_event_log_algs), 0, NULL, 0);
	if (rc < 0) {
		ERROR("Failed to write event log header (%d).\n", rc);
		panic();
	}
}

enum drtm_retc drtm_take_measurements(const struct_drtm_dl_args *a)
{
	int rc;
	uint64_t dlme_img_ep;
	unsigned char hash_data[MAX_TPML_BUFFER_SIZE];
	uint8_t drtm_null_data = 0U;
	uint8_t pcr_schema = DL_ARGS_GET_PCR_SCHEMA(a);
	const char *drtm_event_arm_sep_data = "ARM_DRTM";

	/* Initialise the EventLog driver */
	drtm_event_log_init(drtm_event_log, drtm_event_log +
			    sizeof(drtm_event_log));

	/**
	 * Measurements extended into PCR-17.
	 *
	 * PCR-17: Measure the DCE image.  Extend digest of (char)0 into PCR-17
	 * since the D-CRTM and the DCE are not separate.
	 */
	rc = drtm_event_log_measure_and_record((uintptr_t)&drtm_null_data,
					       sizeof(drtm_null_data),
					       DRTM_EVENT_ARM_DCE, NULL,
					       PCR_17);
	CHECK_RC(rc, drtm_event_log_measure_and_record(DRTM_EVENT_ARM_DCE));

	/* PCR-17: Measure the PCR schema DRTM launch argument. */
	rc = drtm_event_log_measure_and_record((uintptr_t)&pcr_schema,
					       sizeof(pcr_schema),
					       DRTM_EVENT_ARM_PCR_SCHEMA,
					       NULL, PCR_17);
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_PCR_SCHEMA));

	/* PCR-17: Measure the enable state of external-debug, and trace. */
	/*
	 * TODO: Measure the enable state of external-debug and trace.  This should
	 * be returned through a platform-specific hook.
	 */

	/* PCR-17: Measure the security lifecycle state. */
	/*
	 * TODO: Measure the security lifecycle state.  This is an implementation-
	 * defined value, retrieved through an implementation-defined mechanisms.
	 */

	/*
	 * PCR-17: Optionally measure the NWd DCE.
	 * It is expected that such subsequent DCE stages are signed and verified.
	 * Whether they are measured in addition to signing is implementation
	 * -defined.
	 * Here the choice is to not measure any NWd DCE, in favour of PCR value
	 * resilience to any NWd DCE updates.
	 */

	/* PCR-17: End of DCE measurements. */
	rc = drtm_event_log_measure_and_record((uintptr_t)drtm_event_arm_sep_data,
					       strlen(drtm_event_arm_sep_data),
					       DRTM_EVENT_ARM_SEPARATOR,
					       drtm_event_arm_sep_data,
					       PCR_17);
	CHECK_RC(rc, drtm_event_log_measure_and_record(DRTM_EVENT_ARM_SEPARATOR));

	/**
	 * Measurements extended into PCR-18.
	 *
	 * PCR-18: Measure the PCR schema DRTM launch argument.
	 */
	rc = drtm_event_log_measure_and_record((uintptr_t)&pcr_schema,
					       sizeof(pcr_schema),
					       DRTM_EVENT_ARM_PCR_SCHEMA,
					       NULL, PCR_18);
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_PCR_SCHEMA));

	/*
	 * PCR-18: Measure the public key used to verify DCE image(s) signatures.
	 * Extend digest of (char)0, since we do not expect the NWd DCE to be
	 * present.
	 */
	assert(a->dce_nwd_size == 0);
	rc = drtm_event_log_measure_and_record((uintptr_t)&drtm_null_data,
					       sizeof(drtm_null_data),
					       DRTM_EVENT_ARM_DCE_PUBKEY,
					       NULL, PCR_18);
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_DCE_PUBKEY));

	/* PCR-18: Measure the DLME image. */
	rc = drtm_measure_dlme_img(a, hash_data, sizeof(hash_data));
	if (rc != 0) {
		WARN("DRTM: %s: DLME image measurement failed rc=%d\n",
		     __func__, rc);
		return INTERNAL_ERROR;
	}

	rc = drtm_event_log_record(hash_data, DRTM_EVENT_ARM_DLME, NULL,
				   PCR_18, NULL, 0U);
	CHECK_RC(rc, drtm_event_log_record(DRTM_EVENT_ARM_DLME));

	/* PCR-18: Measure the DLME image entry point. */
	dlme_img_ep = DL_ARGS_GET_DLME_ENTRY_POINT(a);
	rc = drtm_event_log_measure_and_record((uintptr_t)&(a->dlme_img_ep_off),
					       sizeof(dlme_img_ep),
					       DRTM_EVENT_ARM_DLME_EP, NULL,
					       PCR_18);
	CHECK_RC(rc, drtm_event_log_measure_and_record(DRTM_EVENT_ARM_DLME_EP));

	/* PCR-18: End of DCE measurements. */
	rc = drtm_event_log_measure_and_record((uintptr_t)drtm_event_arm_sep_data,
					       strlen(drtm_event_arm_sep_data),
					       DRTM_EVENT_ARM_SEPARATOR,
					       drtm_event_arm_sep_data,
					       PCR_18);
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_SEPARATOR));

	/* Measure no Action event but not extend it in PCR */
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_NO_ACTION));
	/*
	 * If the DCE is unable to log a measurement because there is no available
	 * space in the event log region, the DCE must extend a hash of the value
	 * 0xFF (1 byte in size) into PCR[17] and PCR[18] and enter remediation.
	 */

	return SUCCESS;
}

void drtm_serialise_event_log(uint8_t *dst, size_t *event_log_size_out)
{
	*event_log_size_out = event_log_get_cur_size(drtm_event_log);
	memcpy(dst, drtm_event_log, *event_log_size_out);
}

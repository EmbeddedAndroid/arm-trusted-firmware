/*
 * Copyright (c) 2022-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <plat/common/platform.h>
#include <platform_def.h>

#include <event_measure.h>
#include <event_print.h>

/* DRTM TPM Features */
static const plat_drtm_tpm_features_t tpm_features = {
	/* No TPM-based hashing supported. */
	.tpm_based_hash_support = false,

	/* Set to decided algorithm by Event Log driver */
	.firmware_hash_algorithm = TPM_ALG_ID

};

const plat_drtm_tpm_features_t *plat_drtm_get_tpm_features(void)
{
	return &tpm_features;
}

/* FVP has no TPM: the event log is the only record of a launch. */
int plat_drtm_tpm_dl_event(const uint8_t *data, size_t size)
{
	(void)data;
	(void)size;

	return 0;
}

int plat_drtm_tpm_extend(unsigned int pcr, const void *digests)
{
	(void)pcr;
	(void)digests;

	return 0;
}

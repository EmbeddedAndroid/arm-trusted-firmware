#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# DRTM (DRTM_SUPPORT=1) for QTI platforms: the DRTM service records its
# measurements in an event log (SHA-256 and SHA-384 via mbedTLS,
# MBEDTLS_DIR) and engages the SMMU-500 instances for DMA protection. The
# launch is recorded in the TPM through OP-TEE's D-RTM endpoint (SPMD
# logical partition).

ifeq (${DRTM_SUPPORT},1)
ENABLE_SPMD_LP		:=	1

include drivers/measured_boot/event_log/event_log.mk
BL31_LIBS		+=	$(LIBEVLOG_LIBS)
BL31_INCLUDE_DIRS	+=	$(LIBEVLOG_INCLUDE_DIRS)

PLAT_INCLUDES		+=	-Iinclude/drivers/auth/mbedtls
BL31_SOURCES		+=	drivers/arm/smmu/smmu_v2.c		\
				drivers/auth/crypto_mod.c		\
				${PLAT_PATH}/common/src/qti_drtm.c

CRYPTO_LIB_MK		:=	drivers/auth/mbedtls/mbedtls_crypto.mk
include ${CRYPTO_LIB_MK}
endif

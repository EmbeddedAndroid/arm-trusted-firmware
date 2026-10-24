#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# BL2 authenticates the images it loads from the FIP with the TBBR chain of
# trust, rooted in the SHA-256 hash of the public part of ROT_KEY.

ifeq (${ROT_KEY},)
$(error TRUSTED_BOARD_BOOT needs ROT_KEY)
endif

include drivers/auth/auth.mk
include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

BL2_SOURCES		+=	${AUTH_SOURCES}					\
				drivers/auth/tbbr/tbbr_cot_common.c		\
				drivers/auth/tbbr/tbbr_cot_bl2.c		\
				plat/common/tbbr/plat_tbbr.c			\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_rotpk.S	\
				$(PLAT_PATH)/common/src/qti_trusted_boot.c

ROTPK_HASH		:=	$(BUILD_PLAT)/rotpk_sha256.bin
$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))

$(BUILD_PLAT)/bl2/qti_rotpk.o: $(ROTPK_HASH)

$(ROTPK_HASH): $(ROT_KEY) | $$(@D)/
	$(s)echo "  OPENSSL $@"
	$(q)${OPENSSL_BIN_PATH}/openssl pkey -in $< -pubout -outform DER | \
	${OPENSSL_BIN_PATH}/openssl dgst -sha256 -binary > $@

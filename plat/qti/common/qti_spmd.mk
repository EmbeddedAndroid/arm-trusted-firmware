#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# FF-A with OP-TEE as the S-EL1 SPMC (SPD=spmd). The SPMC core manifest,
# built from the platform's BL32_BASE and BL32_SIZE, goes into the FIP as
# TOS_FW_CONFIG; BL2 loads it at TOS_FW_CONFIG_BASE and BL31's SPMD parses
# it. Group-0 interrupts reach the QTI interrupt service through
# plat_spmd_handle_group0_interrupt() instead of the QTI EL3 handler.

ifeq (${SPD},spmd)
BL31_SOURCES		+=	$(PLAT_PATH)/common/src/qti_spmd.c

ARM_SPMC_MANIFEST_DTS	?=	$(PLAT_PATH)/common/fdts/qti_spmc_manifest.dts
FDT_SOURCES		+=	${ARM_SPMC_MANIFEST_DTS}
QTI_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${ARM_SPMC_MANIFEST_DTS})).dtb
$(eval $(call TOOL_ADD_PAYLOAD,${QTI_TOS_FW_CONFIG},--tos-fw-config,${QTI_TOS_FW_CONFIG}))

# Common manifest parser + FDT helpers required by SPMD
include lib/libfdt/libfdt.mk
include common/fdt_wrappers.mk
BL31_SOURCES		+=	plat/common/plat_spmd_manifest.c			\
				common/uuid.c						\
				${LIBFDT_SRCS}						\
				${FDT_WRAPPERS_SOURCES}
else
BL31_SOURCES		+=	$(PLAT_PATH)/common/src/qti_interrupt_svc_el3.c
endif

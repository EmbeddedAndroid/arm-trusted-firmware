#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# The X1E80100 (Hamoa) family without BL31QTILIB: BL2 booted by XBL as the
# TZ image, BL31 with the native PSCI back-end. Purwa (X1P42100) builds it
# with two clusters.

PLAT_PATH				:=	plat/qti
CHIPSET					:=	hamoa

RESET_TO_BL2				:=	1

# Oryon clusters of four cores: three on X1E80100, two on X1P42100.
HAMOA_CLUSTER_COUNT			?=	3
$(eval $(call assert_numeric,HAMOA_CLUSTER_COUNT))
$(eval $(call add_define,HAMOA_CLUSTER_COUNT))

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA		:=	1
USE_COHERENT_MEM			:=	0
WARMBOOT_ENABLE_DCACHE_EARLY		:=	1
HW_ASSISTED_COHERENCY			:=	1

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID			:=	1
ARM_RECOM_STATE_ID_ENC			:=	1
PSCI_OS_INIT_MODE			:=	1

# GIC-700 (GICv4.1, extended SPIs)
GICV3_SUPPORT_GIC600			:=	1
GIC_ENABLE_V4_EXTN			:=	1
GIC_EXT_INTID				:=	1

COLD_BOOT_SINGLE_CPU			:=	1
PROGRAMMABLE_RESET_ADDRESS		:=	1

# Enable the dynamic translation tables library
PLAT_XLAT_TABLES_DYNAMIC		:=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

# The Oryon cores are AArch64 only.
override CTX_INCLUDE_AARCH32_REGS	:=	0

# Enable stack protector.
ENABLE_STACK_PROTECTOR			:=	strong

# The Oryon cores implement the EL2 controls Linux uses at EL2 (VHE, KVM):
# grant them from EL3, each checked at runtime against the ID registers.
ENABLE_FEAT_HCX				:=	2
ENABLE_FEAT_FGT				:=	2
ENABLE_FEAT_FGT2			:=	2
ENABLE_FEAT_ECV				:=	2
ENABLE_FEAT_TCR2			:=	2
ENABLE_FEAT_SCTLR2			:=	2
ENABLE_FEAT_S1PIE			:=	2
ENABLE_FEAT_S2PIE			:=	2
ENABLE_FEAT_S1POE			:=	2
ENABLE_FEAT_S2POE			:=	2
ENABLE_FEAT_MOPS			:=	2
ENABLE_FEAT_AMU				:=	2

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\
				-I${PLAT_PATH}/wildcat/${CHIPSET}/inc			\
				-I${PLAT_PATH}/wildcat/common/inc			\
				-I${PLAT_PATH}/wildcat/common/inc/$(ARCH)		\
				-I${PLAT_PATH}/common/inc				\
				-I${PLAT_PATH}/common/inc/$(ARCH)			\
				-Iinclude/drivers/qti/qtimer/hoya

include lib/xlat_tables_v2/xlat_tables.mk
include drivers/qti/chipinfo/chipinfo.mk
include drivers/qti/smem/smem.mk
PLAT_BL_COMMON_SOURCES	+=	common/desc_image_load.c				\
				drivers/qti/crypto/rng.c				\
				lib/bl_aux_params/bl_aux_params.c			\
				plat/common/aarch64/crash_console_helpers.S		\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(PLAT_PATH)/common/src/qti_stack_protector.c		\
				$(PLAT_PATH)/common/src/qti_common.c			\
				$(PLAT_PATH)/wildcat/common/src/$(ARCH)/qcom_oryon_v1.S	\
				${XLAT_TABLES_LIB_SRCS}

BL2_SOURCES		+=	drivers/io/io_fip.c					\
				drivers/io/io_memmap.c					\
				drivers/io/io_storage.c					\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_bl2_helpers.S	\
				$(PLAT_PATH)/common/src/qti_bl2_setup.c			\
				$(PLAT_PATH)/common/src/qti_image_desc.c		\
				$(PLAT_PATH)/common/src/qti_io_storage.c

include drivers/arm/gic/v3/gicv3.mk
BL31_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c		\
				drivers/delay_timer/delay_timer.c			\
				plat/common/plat_gicv3.c				\
				${GICV3_SOURCES}					\
				plat/common/plat_psci_common.c				\
				$(PLAT_PATH)/common/src/qti_bl31_setup.c		\
				$(PLAT_PATH)/common/src/qti_gic_v3.c			\
				$(PLAT_PATH)/common/src/qti_interrupt_svc.c		\
				$(PLAT_PATH)/common/src/qti_interrupt_svc_el3.c	\
				$(PLAT_PATH)/common/src/qti_syscall.c			\
				$(PLAT_PATH)/common/src/qti_sip_platform.c		\
				$(PLAT_PATH)/common/src/qti_pm.c			\
				$(PLAT_PATH)/wildcat/common/src/$(ARCH)/wildcat_helpers.S \
				$(PLAT_PATH)/wildcat/common/src/wildcat_bl31_setup.c	\
				$(PLAT_PATH)/wildcat/common/src/wildcat_plat_helpers.c \
				$(PLAT_PATH)/wildcat/common/src/wildcat_topology.c	\
				$(PLAT_PATH)/wildcat/${CHIPSET}/src/hamoa_pm.c		\
				drivers/qti/accesscontrol/access_control_stub.c		\
				drivers/qti/qtimer/qtimer.c				\
				drivers/qti/sec_core/sec_core_ncc.c			\
				drivers/qti/watchdog/watchdog.c

PLAT_INCLUDES		+=	-Iinclude/drivers/qti/watchdog/${CHIPSET}

QTI_PMIC_ARB_VERSION	:=	pmicarb7
include drivers/qti/pmic/pmic.mk
include drivers/qti/clock/clock.mk
include drivers/qti/cpucp/cpucp.mk
include drivers/qti/smmu/smmu.mk

#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Makefile for the SM7750 (Eliza) MTP, a Wildcat part booted by XBL without
# BL31QTILIB.

PLAT_PATH				:=	plat/qti
CHIPSET					:=	eliza

RESET_TO_BL2				:=	1

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA		:=	1
USE_COHERENT_MEM			:=	0
WARMBOOT_ENABLE_DCACHE_EARLY		:=	1
HW_ASSISTED_COHERENCY			:=	1

# BL31 is the only EL3 firmware, so it applies the core and DSU errata; each
# workaround is checked against the core revision at reset.
ERRATA_A520_2630792			:=	1
ERRATA_A520_2677201			:=	1
ERRATA_A520_2858100			:=	1
ERRATA_A520_2938996			:=	1
ERRATA_A520_3631357			:=	1
ERRATA_A520_3685825			:=	1
ERRATA_A520_4491980			:=	1
ERRATA_A720_2729604			:=	1
ERRATA_A720_2792132			:=	1
ERRATA_A720_2844092			:=	1
ERRATA_A720_2926083			:=	1
ERRATA_A720_2940794			:=	1
ERRATA_A720_3456091			:=	1
ERRATA_A720_3699561			:=	1
ERRATA_A720_3711910			:=	1
ERRATA_DSU_2900952			:=	1

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID			:=	1
ARM_RECOM_STATE_ID_ENC			:=	1
PSCI_OS_INIT_MODE			:=	1

# GIC-700 (GICv4.1)
GICV3_SUPPORT_GIC600			:=	1
GIC_ENABLE_V4_EXTN			:=	1

COLD_BOOT_SINGLE_CPU			:=	1
PROGRAMMABLE_RESET_ADDRESS		:=	1

# Enable the dynamic translation tables library
PLAT_XLAT_TABLES_DYNAMIC		:=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

# The Cortex-A520 and Cortex-A720 cores are AArch64 only at EL1+.
override CTX_INCLUDE_AARCH32_REGS	:=	0

# Enable stack protector.
ENABLE_STACK_PROTECTOR			:=	strong

# The Armv9.2 cores implement the EL2 controls Linux uses at EL2 (VHE, KVM):
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
ENABLE_FEAT_AMUv1p1			:=	2
ENABLE_FEAT_MTE2			:=	2
ENABLE_FEAT_CSV2_2			:=	2
ENABLE_FEAT_TWED			:=	2
ENABLE_TRBE_FOR_NS			:=	2
ENABLE_BRBE_FOR_NS			:=	2
ENABLE_TRF_FOR_NS			:=	2
ENABLE_SYS_REG_TRACE_FOR_NS		:=	2

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\
				-I${PLAT_PATH}/wildcat/${CHIPSET}/inc			\
				-I${PLAT_PATH}/wildcat/${CHIPSET}/${PLAT}/inc		\
				-I${PLAT_PATH}/wildcat/common/inc			\
				-I${PLAT_PATH}/common/inc				\
				-I${PLAT_PATH}/common/inc/$(ARCH)			\
				-Iinclude/drivers/qti/qtimer/hoya

include lib/xlat_tables_v2/xlat_tables.mk
include drivers/qti/chipinfo/chipinfo.mk
include drivers/qti/smem/smem.mk
PLAT_BL_COMMON_SOURCES	+=	common/desc_image_load.c				\
				drivers/qti/crypto/rng.c				\
				lib/cpus/aarch64/cortex_a520.S				\
				lib/cpus/aarch64/cortex_a720.S				\
				lib/bl_aux_params/bl_aux_params.c			\
				plat/common/aarch64/crash_console_helpers.S		\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(PLAT_PATH)/common/src/qti_stack_protector.c		\
				$(PLAT_PATH)/common/src/qti_common.c			\
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
				$(PLAT_PATH)/wildcat/${CHIPSET}/src/eliza_pm.c		\
				drivers/qti/accesscontrol/access_control_stub.c		\
				drivers/qti/qtimer/qtimer.c				\
				drivers/qti/sec_core/sec_core.c				\
				drivers/qti/sec_core/${CHIPSET}/sec_core_cfg.c		\
				drivers/qti/watchdog/watchdog.c

PLAT_INCLUDES		+=	-Iinclude/drivers/qti/sec_core/${CHIPSET}		\
				-Iinclude/drivers/qti/watchdog/${CHIPSET}

QTI_PMIC_ARB_VERSION	:=	pmicarb7
include drivers/qti/pmic/pmic.mk
include drivers/qti/clock/clock.mk
include drivers/qti/smmu/smmu.mk

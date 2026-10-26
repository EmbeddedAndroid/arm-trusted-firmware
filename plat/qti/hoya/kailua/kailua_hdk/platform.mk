#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Makefile for the SM8550 (Kailua) HDK.

PLAT_PATH				:=	plat/qti
CHIPSET					:=	kailua

RESET_TO_BL2				:=	1

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA		:=	1
USE_COHERENT_MEM			:=	0
WARMBOOT_ENABLE_DCACHE_EARLY		:=	1
HW_ASSISTED_COHERENCY			:=	1

# TF-A replaces the stock TZ, so it applies the core and DSU errata; each
# workaround is checked against the core revision at reset.
ERRATA_A510_1910738			:=	1
ERRATA_A510_1937669			:=	1
ERRATA_A510_1942494			:=	1
ERRATA_A510_1952872			:=	1
ERRATA_A510_1966377			:=	1
ERRATA_A510_1975068			:=	1
ERRATA_A510_1976290			:=	1
ERRATA_A510_2002389			:=	1
ERRATA_A510_2008766			:=	1
ERRATA_A510_2027318			:=	1
ERRATA_A510_2028010			:=	1
ERRATA_A510_2041909			:=	1
ERRATA_A510_2042739			:=	1
ERRATA_A510_2080326			:=	1
ERRATA_A510_2169012			:=	1
ERRATA_A510_2172148			:=	1
ERRATA_A510_2218134			:=	1
ERRATA_A510_2218950			:=	1
ERRATA_A510_2250311			:=	1
ERRATA_A510_2288014			:=	1
ERRATA_A510_2347730			:=	1
ERRATA_A510_2371937			:=	1
ERRATA_A510_2420992			:=	1
ERRATA_A510_2666669			:=	1
ERRATA_A510_2684597			:=	1
ERRATA_A510_2971420			:=	1
ERRATA_A510_3672349			:=	1
ERRATA_A510_3704847			:=	1
ERRATA_A510_4568615			:=	1
ERRATA_A715_2238661			:=	1
ERRATA_A715_2239006			:=	1
ERRATA_A715_2275754			:=	1
ERRATA_A715_2284544			:=	1
ERRATA_A715_2285473			:=	1
ERRATA_A715_2292761			:=	1
ERRATA_A715_2331818			:=	1
ERRATA_A715_2344187			:=	1
ERRATA_A715_2376701			:=	1
ERRATA_A715_2409570			:=	1
ERRATA_A715_2413290			:=	1
ERRATA_A715_2420947			:=	1
ERRATA_A715_2429384			:=	1
ERRATA_A715_2561034			:=	1
ERRATA_A715_2728106			:=	1
ERRATA_A715_2804830			:=	1
ERRATA_A715_3456084			:=	1
ERRATA_A715_3699560			:=	1
ERRATA_A715_3711916			:=	1
ERRATA_A710_1785648			:=	1
ERRATA_A710_1793423			:=	1
ERRATA_A710_1847092			:=	1
ERRATA_A710_1887102			:=	1
ERRATA_A710_1901946			:=	1
ERRATA_A710_1916945			:=	1
ERRATA_A710_1917258			:=	1
ERRATA_A710_1927200			:=	1
ERRATA_A710_1987031			:=	1
ERRATA_A710_2008768			:=	1
ERRATA_A710_2017096			:=	1
ERRATA_A710_2055002			:=	1
ERRATA_A710_2081180			:=	1
ERRATA_A710_2083908			:=	1
ERRATA_A710_2136059			:=	1
ERRATA_A710_2147715			:=	1
ERRATA_A710_2216384			:=	1
ERRATA_A710_2267065			:=	1
ERRATA_A710_2282622			:=	1
ERRATA_A710_2291219			:=	1
ERRATA_A710_2371105			:=	1
ERRATA_A710_2742423			:=	1
ERRATA_A710_2768515			:=	1
ERRATA_A710_2778471			:=	1
ERRATA_A710_3324338			:=	1
ERRATA_A710_3701772			:=	1
ERRATA_A710_3888122			:=	1
ERRATA_A710_4302969			:=	1
ERRATA_X3_2138930			:=	1
ERRATA_X3_2147714			:=	1
ERRATA_X3_2184829			:=	1
ERRATA_X3_2214778			:=	1
ERRATA_X3_2266875			:=	1
ERRATA_X3_2302506			:=	1
ERRATA_X3_2313909			:=	1
ERRATA_X3_2372204			:=	1
ERRATA_X3_2615812			:=	1
ERRATA_X3_2641945			:=	1
ERRATA_X3_2742421			:=	1
ERRATA_X3_2743088			:=	1
ERRATA_X3_2779509			:=	1
ERRATA_X3_3213672			:=	1
ERRATA_X3_3692984			:=	1
ERRATA_X3_3701769			:=	1
ERRATA_X3_3827463			:=	1
ERRATA_X3_3888125			:=	1
ERRATA_X3_4302966			:=	1
ERRATA_DSU_2313941			:=	1
WORKAROUND_CVE_2025_10263		:=	1

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

# The Cortex-A510, Cortex-A710, Cortex-A715 and Cortex-X3 cores are AArch64
# only at EL1+.
override CTX_INCLUDE_AARCH32_REGS	:=	0

# Enable stack protector.
ENABLE_STACK_PROTECTOR			:=	strong

# Grant the EL2 controls Linux uses at EL2 (VHE, KVM) that Armv9.0 cores may
# implement from EL3, each checked at runtime against the ID registers.
ENABLE_FEAT_HCX				:=	2
ENABLE_FEAT_FGT				:=	2
ENABLE_FEAT_ECV				:=	2
ENABLE_FEAT_AMU				:=	2
ENABLE_FEAT_AMUv1p1			:=	2
ENABLE_FEAT_MTE2			:=	2
ENABLE_FEAT_CSV2_2			:=	2
ENABLE_FEAT_TWED			:=	2
ENABLE_TRBE_FOR_NS			:=	2
ENABLE_TRF_FOR_NS			:=	2
ENABLE_SYS_REG_TRACE_FOR_NS		:=	2

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\
				-I${PLAT_PATH}/hoya/inc					\
				-I${PLAT_PATH}/hoya/${CHIPSET}/inc			\
				-I${PLAT_PATH}/hoya/${CHIPSET}/${PLAT}/inc		\
				-I${PLAT_PATH}/common/inc				\
				-I${PLAT_PATH}/common/inc/$(ARCH)			\
				-I${PLAT_PATH}/hoya/qtiseclib/inc			\
				-Iinclude/drivers/qti/qtimer/hoya

include lib/xlat_tables_v2/xlat_tables.mk
include drivers/qti/chipinfo/chipinfo.mk
include drivers/qti/smem/smem.mk
PLAT_BL_COMMON_SOURCES	+=	common/desc_image_load.c				\
				drivers/qti/crypto/rng.c				\
				lib/cpus/aarch64/cortex_a510.S				\
				lib/cpus/aarch64/cortex_a710.S				\
				lib/cpus/aarch64/cortex_a715.S				\
				lib/cpus/aarch64/cortex_x3.S				\
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
				$(PLAT_PATH)/common/src/pm_ps_hold.c			\
				$(PLAT_PATH)/common/src/qti_bl31_setup.c		\
				$(PLAT_PATH)/common/src/qti_gic_v3.c			\
				$(PLAT_PATH)/common/src/qti_interrupt_svc.c		\
				$(PLAT_PATH)/common/src/qti_interrupt_svc_el3.c	\
				$(PLAT_PATH)/common/src/qti_syscall.c			\
				$(PLAT_PATH)/common/src/qti_topology.c			\
				$(PLAT_PATH)/common/src/qti_pm.c			\
				$(PLAT_PATH)/common/src/spmi_arb.c			\
				$(PLAT_PATH)/hoya/common/$(ARCH)/hoya_helpers.S		\
				$(PLAT_PATH)/hoya/common/hoya_bl31_setup.c		\
				$(PLAT_PATH)/hoya/common/hoya_dsu_gicv3.c		\
				$(PLAT_PATH)/hoya/common/hoya_dsu_pm.c		\
				$(PLAT_PATH)/hoya/qtiseclib/src/qtiseclib_cb_interface.c \
				$(PLAT_PATH)/hoya/qtiseclib/src/qtiseclib_interface_stub.c \
				drivers/qti/accesscontrol/access_control_stub.c		\
				drivers/qti/qtimer/qtimer.c

# Nothing completes the power-down request of a core turned off: no PCU
# power-collapse sequence is ported.
include plat/qti/hoya/common/cpu_hold.mk

PLAT_INCLUDES		+=	-Iinclude/drivers/qti/sec_core/${CHIPSET}		\
				-Iinclude/drivers/qti/watchdog/hoya_dsu

BL31_SOURCES		+=	drivers/qti/sec_core/sec_core.c				\
				drivers/qti/sec_core/${CHIPSET}/sec_core_cfg.c		\
				drivers/qti/watchdog/watchdog.c

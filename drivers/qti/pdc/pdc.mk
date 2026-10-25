#
# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PDC (Power Domain Controller) driver
#

$(eval $(call add_define,QTI_PDC_ENABLED))

# Set to 0 where the AOP programs the apps PDC sleep and wake TCSes.
QTI_PDC_TCS ?= 1
$(eval $(call add_define,QTI_PDC_TCS))

# Set to 1 where the interrupt owners are banked bitmasks with a single
# non-zero owner DRV.
QTI_PDC_IRQ_OWNER_BANKED ?= 0
$(eval $(call add_define,QTI_PDC_IRQ_OWNER_BANKED))

PDC_DRV_PATH := drivers/qti/pdc

# Every supported apps PDC has the hoya register map. A chipset directory
# with its own interrupt table holds all of its tables; the other chipsets
# use the hoya tables. The sequences differ per chipset.
PDC_COMMON := $(PDC_DRV_PATH)/hoya
ifneq ($(filter lemans monaco,$(CHIPSET)),)
PDC_SEQ_CFG := $(PDC_COMMON)/pdc_seq_cfg.c
else
PDC_SEQ_CFG := $(PDC_DRV_PATH)/$(CHIPSET)/pdc_seq_cfg.c
endif
PDC_TABLES := $(if $(wildcard $(PDC_DRV_PATH)/$(CHIPSET)/interrupt_table.c),$(PDC_DRV_PATH)/$(CHIPSET),$(PDC_COMMON))

include $(PDC_COMMON)/pdc_config.mk

PDC_ENABLE_TCS ?= 1
PDC_SEQ_TARGET_SOURCE ?= $(PDC_DRV_PATH)/pdc_seq_cmd.c

# A PDC whose TCSes the AOP programs needs no TCS code in BL31.
ifeq ($(QTI_PDC_TCS),0)
PDC_ENABLE_TCS := 0
endif

PLAT_INCLUDES += \
	-I$(PDC_COMMON)

BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc.c					\
	$(PDC_DRV_PATH)/pdc_seq.c				\
	$(PDC_SEQ_TARGET_SOURCE)				\
	$(PDC_SEQ_CFG)						\
	$(PDC_TABLES)/interrupt_table.c				\
	$(PDC_TABLES)/gpio_table.c

ifeq ($(PDC_ENABLE_TCS),1)
BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc_tcs.c				\
	$(PDC_TABLES)/tcs_resource.c
endif
